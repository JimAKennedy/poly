// Windows native drag-source window for MIDI export. Opens a small top-level
// window containing a draggable clip proxy; the user drags it into the DAW,
// which pulls a temporary .mid file through OLE drag-and-drop (a CF_HDROP data
// object served via DoDragDrop). Called from the UI thread by WebUIView's
// beginMidiDrag action. Additive to openMidiSaveDialog (see
// platform_save_dialog_win.cpp); does NOT use VSTGUI.

#include <algorithm>
#include <cstring>
#include <fstream>
#include <string>
#include <utility>
#include <vector>

#include <ole2.h>
#include <oleidl.h>
#include <shlobj.h>
#include <windows.h>

#include "platform_drag_source.h"

namespace poly {

namespace {

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

// Write the export bytes to a temp .mid file and return its full path, or an
// empty string on failure. The DAW reads this file when the drag is dropped, so
// it must outlive the drag session — a temp-dir file is the simplest durable
// backing (mirrors writeTempMidi in platform_drag_source_mac.mm).
std::wstring writeTempMidi(const std::string& suggestedName, const std::vector<uint8_t>& bytes) {
    wchar_t tempDir[MAX_PATH] = {0};
    DWORD dirLen = GetTempPathW(MAX_PATH, tempDir);
    if (dirLen == 0 || dirLen > MAX_PATH)
        return {};

    std::wstring name = utf8ToWide(suggestedName);
    if (name.empty())
        name = L"poly.mid";

    std::wstring path = std::wstring(tempDir, dirLen) + name;
    std::ofstream out(path.c_str(), std::ios::binary);
    if (!out)
        return {};
    out.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    return out ? path : std::wstring{};
}

// Build a CF_HDROP HGLOBAL naming a single file. The DROPFILES header is
// followed by a double-null-terminated wide path list, which is exactly what a
// DAW (or Explorer) expects from a file drag.
HGLOBAL makeHDrop(const std::wstring& path) {
    const size_t pathBytes = (path.size() + 1) * sizeof(wchar_t);
    // DROPFILES + path + one extra wide NUL to terminate the file list.
    const size_t total = sizeof(DROPFILES) + pathBytes + sizeof(wchar_t);

    HGLOBAL handle = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, total);
    if (!handle)
        return nullptr;

    auto* drop = static_cast<DROPFILES*>(GlobalLock(handle));
    if (!drop) {
        GlobalFree(handle);
        return nullptr;
    }
    drop->pFiles = sizeof(DROPFILES);
    drop->fWide = TRUE;
    auto* dst = reinterpret_cast<wchar_t*>(reinterpret_cast<BYTE*>(drop) + sizeof(DROPFILES));
    std::memcpy(dst, path.c_str(), pathBytes);
    // The trailing list-terminator NUL is already zero from GMEM_ZEROINIT.
    GlobalUnlock(handle);
    return handle;
}

// Minimal IDataObject that serves a single CF_HDROP format backed by the temp
// .mid file. Only the GET direction is implemented — that is all DoDragDrop and
// a file-drop target consume.
class PolyDataObject : public IDataObject {
public:
    explicit PolyDataObject(std::wstring path) : m_path(std::move(path)) {}

    // IUnknown -------------------------------------------------------------
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv) override {
        if (!ppv)
            return E_POINTER;
        if (riid == IID_IUnknown || riid == IID_IDataObject) {
            *ppv = static_cast<IDataObject*>(this);
            AddRef();
            return S_OK;
        }
        *ppv = nullptr;
        return E_NOINTERFACE;
    }
    ULONG STDMETHODCALLTYPE AddRef() override { return static_cast<ULONG>(InterlockedIncrement(&m_ref)); }
    ULONG STDMETHODCALLTYPE Release() override {
        LONG ref = InterlockedDecrement(&m_ref);
        if (ref == 0)
            delete this;
        return static_cast<ULONG>(ref);
    }

    // IDataObject ----------------------------------------------------------
    HRESULT STDMETHODCALLTYPE GetData(FORMATETC* format, STGMEDIUM* medium) override {
        if (!format || !medium)
            return E_INVALIDARG;
        if (!wantsHDrop(*format))
            return DV_E_FORMATETC;

        HGLOBAL hdrop = makeHDrop(m_path);
        if (!hdrop)
            return E_OUTOFMEMORY;

        std::memset(medium, 0, sizeof(*medium));
        medium->tymed = TYMED_HGLOBAL;
        medium->hGlobal = hdrop;
        medium->pUnkForRelease = nullptr; // caller owns via ReleaseStgMedium.
        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE GetDataHere(FORMATETC*, STGMEDIUM*) override { return E_NOTIMPL; }
    HRESULT STDMETHODCALLTYPE QueryGetData(FORMATETC* format) override {
        if (!format)
            return E_INVALIDARG;
        return wantsHDrop(*format) ? S_OK : DV_E_FORMATETC;
    }
    HRESULT STDMETHODCALLTYPE GetCanonicalFormatEtc(FORMATETC*, FORMATETC* out) override {
        if (out)
            out->ptd = nullptr;
        return E_NOTIMPL;
    }
    HRESULT STDMETHODCALLTYPE SetData(FORMATETC*, STGMEDIUM*, BOOL) override { return E_NOTIMPL; }
    HRESULT STDMETHODCALLTYPE EnumFormatEtc(DWORD direction, IEnumFORMATETC** out) override {
        if (direction != DATADIR_GET)
            return E_NOTIMPL;
        FORMATETC format = hdropFormat();
        return SHCreateStdEnumFmtEtc(1, &format, out);
    }
    HRESULT STDMETHODCALLTYPE DAdvise(FORMATETC*, DWORD, IAdviseSink*, DWORD*) override {
        return OLE_E_ADVISENOTSUPPORTED;
    }
    HRESULT STDMETHODCALLTYPE DUnadvise(DWORD) override { return OLE_E_ADVISENOTSUPPORTED; }
    HRESULT STDMETHODCALLTYPE EnumDAdvise(IEnumSTATDATA**) override { return OLE_E_ADVISENOTSUPPORTED; }

private:
    static FORMATETC hdropFormat() {
        FORMATETC format = {};
        format.cfFormat = CF_HDROP;
        format.ptd = nullptr;
        format.dwAspect = DVASPECT_CONTENT;
        format.lindex = -1;
        format.tymed = TYMED_HGLOBAL;
        return format;
    }
    static bool wantsHDrop(const FORMATETC& format) {
        return format.cfFormat == CF_HDROP && (format.tymed & TYMED_HGLOBAL) && (format.dwAspect & DVASPECT_CONTENT);
    }

    LONG m_ref = 1;
    std::wstring m_path;
};

// Standard IDropSource: keep dragging while the left button is held, drop when
// it is released, cancel on Escape.
class PolyDropSource : public IDropSource {
public:
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv) override {
        if (!ppv)
            return E_POINTER;
        if (riid == IID_IUnknown || riid == IID_IDropSource) {
            *ppv = static_cast<IDropSource*>(this);
            AddRef();
            return S_OK;
        }
        *ppv = nullptr;
        return E_NOINTERFACE;
    }
    ULONG STDMETHODCALLTYPE AddRef() override { return static_cast<ULONG>(InterlockedIncrement(&m_ref)); }
    ULONG STDMETHODCALLTYPE Release() override {
        LONG ref = InterlockedDecrement(&m_ref);
        if (ref == 0)
            delete this;
        return static_cast<ULONG>(ref);
    }
    HRESULT STDMETHODCALLTYPE QueryContinueDrag(BOOL escapePressed, DWORD keyState) override {
        if (escapePressed)
            return DRAGDROP_S_CANCEL;
        if (!(keyState & MK_LBUTTON))
            return DRAGDROP_S_DROP;
        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE GiveFeedback(DWORD) override { return DRAGDROP_S_USEDEFAULTCURSORS; }

private:
    LONG m_ref = 1;
};

// Per-window state: the label shown in the proxy and the temp file it serves.
struct DragWindowState {
    std::wstring label;
    std::wstring path;
};

const wchar_t* kDragWindowClass = L"PolyMidiDragSource";

HINSTANCE thisModule() {
    HMODULE module = nullptr;
    // Register/create against our own DLL module rather than the host EXE.
    GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                       reinterpret_cast<LPCWSTR>(&thisModule), &module);
    return module;
}

void beginDrag(HWND window) {
    auto* state = reinterpret_cast<DragWindowState*>(GetWindowLongPtrW(window, GWLP_USERDATA));
    if (!state || state->path.empty())
        return;

    // ownership-transfer — COM objects are refcount-managed (initial ref 1);
    // ownership passes to DoDragDrop and is released below, so a smart pointer
    // would fight the AddRef/Release protocol.
    auto* data = new PolyDataObject(state->path); // ownership-transfer
    auto* source = new PolyDropSource();          // ownership-transfer

    DWORD effect = DROPEFFECT_NONE;
    DoDragDrop(data, source, DROPEFFECT_COPY, &effect);

    data->Release();
    source->Release();
}

void paintProxy(HWND window) {
    PAINTSTRUCT ps;
    HDC dc = BeginPaint(window, &ps);
    RECT rc;
    GetClientRect(window, &rc);

    FillRect(dc, &rc, reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1));

    RECT border = rc;
    InflateRect(&border, -4, -4);
    HBRUSH hollow = static_cast<HBRUSH>(GetStockObject(HOLLOW_BRUSH));
    HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(dc, hollow));
    RoundRect(dc, border.left, border.top, border.right, border.bottom, 12, 12);
    SelectObject(dc, oldBrush);

    auto* state = reinterpret_cast<DragWindowState*>(GetWindowLongPtrW(window, GWLP_USERDATA));
    if (state && !state->label.empty()) {
        SetBkMode(dc, TRANSPARENT);
        DrawTextW(dc, state->label.c_str(), -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }

    EndPaint(window, &ps);
}

LRESULT CALLBACK dragWndProc(HWND window, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_LBUTTONDOWN:
        beginDrag(window);
        return 0;
    case WM_PAINT:
        paintProxy(window);
        return 0;
    case WM_NCDESTROY: {
        auto* state = reinterpret_cast<DragWindowState*>(GetWindowLongPtrW(window, GWLP_USERDATA));
        delete state;
        SetWindowLongPtrW(window, GWLP_USERDATA, 0);
        return DefWindowProcW(window, msg, wParam, lParam);
    }
    default:
        return DefWindowProcW(window, msg, wParam, lParam);
    }
}

void ensureClassRegistered() {
    static bool registered = false;
    if (registered)
        return;
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = dragWndProc;
    wc.hInstance = thisModule();
    // IDC_HAND expands via MAKEINTRESOURCE, which is the ANSI (LPSTR) form when
    // the TU is built without UNICODE defined; cast the integer atom to LPCWSTR
    // so it matches the explicit LoadCursorW signature.
    wc.hCursor = LoadCursorW(nullptr, reinterpret_cast<LPCWSTR>(IDC_HAND));
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wc.lpszClassName = kDragWindowClass;
    RegisterClassExW(&wc); // Idempotent enough; a second call just fails harmlessly.
    registered = true;
}

} // namespace

void beginMidiDragExport(void* parentView, const std::string& suggestedName, const std::vector<uint8_t>& bytes) {
    std::wstring path = writeTempMidi(suggestedName, bytes);
    if (path.empty())
        return;

    // DoDragDrop requires OLE (not just COM) on the calling thread. The DAW's UI
    // thread is normally already OLE-initialised, in which case this just bumps
    // the ref count; we do not match with OleUninitialize because the window
    // outlives this call and needs OLE alive for its later drag.
    OleInitialize(nullptr);

    ensureClassRegistered();

    // ownership-transfer — ownership passes to the HWND via SetWindowLongPtrW
    // below and is reclaimed in the WM_NCDESTROY handler.
    auto* state = new DragWindowState(); // ownership-transfer
    state->label = utf8ToWide(suggestedName);
    if (state->label.empty())
        state->label = L"poly.mid";
    state->path = path;

    const int width = 180;
    const int height = 64;

    // Center over the parent window when we have one, otherwise center on the
    // primary work area.
    int x = CW_USEDEFAULT;
    int y = CW_USEDEFAULT;
    HWND parent = static_cast<HWND>(parentView);
    RECT anchor = {};
    if (parent && GetWindowRect(parent, &anchor)) {
        x = anchor.left + ((anchor.right - anchor.left) - width) / 2;
        y = anchor.top + ((anchor.bottom - anchor.top) - height) / 2;
    } else {
        RECT work = {};
        if (SystemParametersInfoW(SPI_GETWORKAREA, 0, &work, 0)) {
            x = work.left + ((work.right - work.left) - width) / 2;
            y = work.top + ((work.bottom - work.top) - height) / 2;
        }
    }

    HWND window =
        CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_TOPMOST, kDragWindowClass, L"Drag to DAW", WS_POPUPWINDOW | WS_CAPTION,
                        x, y, width, height, nullptr, nullptr, thisModule(), nullptr);
    if (!window) {
        delete state;
        return;
    }

    SetWindowLongPtrW(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(state));
    ShowWindow(window, SW_SHOWNOACTIVATE);
    UpdateWindow(window);
}

} // namespace poly
