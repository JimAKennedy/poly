// Windows native Save As… dialog for MIDI export. Uses IFileSaveDialog
// (Vista+); falls back to nothing if the COM call fails. Called from the
// UI thread by WebUIView's exportSaveAs action.

#include <fstream>

#include <shlwapi.h>
#include <shobjidl.h>
#include <windows.h>

#include "platform_save_dialog.h"

namespace poly {

namespace {

std::string wideToUtf8(const wchar_t* wide) {
    if (!wide)
        return {};
    int len = WideCharToMultiByte(CP_UTF8, 0, wide, -1, nullptr, 0, nullptr, nullptr);
    if (len <= 1)
        return {};
    std::string out(static_cast<size_t>(len - 1), '\0');
    WideCharToMultiByte(CP_UTF8, 0, wide, -1, out.data(), len, nullptr, nullptr);
    return out;
}

std::wstring utf8ToWide(const std::string& utf8) {
    if (utf8.empty())
        return {};
    int len = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, nullptr, 0);
    if (len <= 1)
        return {};
    std::wstring out(static_cast<size_t>(len - 1), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, out.data(), len);
    return out;
}

void writeBytesToPath(const std::string& path, const std::vector<uint8_t>& bytes) {
    std::ofstream out(path, std::ios::binary);
    if (!out)
        return;
    out.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
}

} // namespace

void openMidiSaveDialog(void* parentView, const std::string& suggestedName, const std::vector<uint8_t>& bytes,
                        std::function<void(const std::string&)> onResult) {
    std::string savedPath;

    // COM is per-thread. The DAW's UI thread usually initialises STA, but we
    // guard our own scope with CoInitializeEx and match with CoUninitialize.
    HRESULT hrInit = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    bool weInitialised = SUCCEEDED(hrInit);

    IFileSaveDialog* dialog = nullptr;
    HRESULT hr = CoCreateInstance(CLSID_FileSaveDialog, nullptr, CLSCTX_INPROC_SERVER, IID_IFileSaveDialog,
                                  reinterpret_cast<void**>(&dialog));
    if (SUCCEEDED(hr) && dialog) {
        COMDLG_FILTERSPEC filter[] = {{L"MIDI File", L"*.mid;*.midi"}};
        dialog->SetFileTypes(1, filter);
        dialog->SetFileTypeIndex(1);
        dialog->SetDefaultExtension(L"mid");
        dialog->SetTitle(L"Export MIDI");

        auto wname = utf8ToWide(suggestedName);
        if (!wname.empty())
            dialog->SetFileName(wname.c_str());

        HWND parent = static_cast<HWND>(parentView);
        hr = dialog->Show(parent);
        if (SUCCEEDED(hr)) {
            IShellItem* item = nullptr;
            if (SUCCEEDED(dialog->GetResult(&item)) && item) {
                PWSTR path = nullptr;
                if (SUCCEEDED(item->GetDisplayName(SIGDN_FILESYSPATH, &path)) && path) {
                    savedPath = wideToUtf8(path);
                    CoTaskMemFree(path);
                }
                item->Release();
            }
        }
        dialog->Release();
    }

    if (weInitialised)
        CoUninitialize();

    if (!savedPath.empty())
        writeBytesToPath(savedPath, bytes);

    if (onResult)
        onResult(savedPath);
}

} // namespace poly
