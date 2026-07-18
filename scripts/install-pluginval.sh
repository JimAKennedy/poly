#!/usr/bin/env bash
# Install pluginval for local pre-push verification.
#
# Prefers Homebrew on macOS; falls back to the same GitHub release asset CI uses
# on both macOS and Linux. Idempotent — safe to re-run.
#
# Once installed, `scripts/pre-push-check.sh` will pick up pluginval and run it
# against the built .vst3 bundle, closing the local/CI parity gap that produced
# the M046 S01 pluginval regressions.

set -euo pipefail

BIN_DIR="${HOME}/.local/bin"
PLUGINVAL_DIR="${HOME}/.local/share/pluginval"
mkdir -p "$BIN_DIR" "$PLUGINVAL_DIR"

if command -v pluginval >/dev/null 2>&1; then
    echo "pluginval already on PATH: $(command -v pluginval)"
    exit 0
fi

OS_NAME="$(uname -s)"

if [ "$OS_NAME" = "Darwin" ]; then
    if command -v brew >/dev/null 2>&1; then
        if brew list --cask pluginval >/dev/null 2>&1; then
            echo "pluginval already installed via brew (cask)."
        elif brew list pluginval >/dev/null 2>&1; then
            echo "pluginval already installed via brew (formula)."
        else
            echo "Installing pluginval via brew..."
            brew install pluginval || brew install --cask pluginval
        fi
        if command -v pluginval >/dev/null 2>&1; then
            echo "pluginval installed via brew: $(command -v pluginval)"
            exit 0
        fi
    fi
    ASSET_URL="https://github.com/Tracktion/pluginval/releases/latest/download/pluginval_macOS.zip"
    APP_PATH="${PLUGINVAL_DIR}/pluginval.app/Contents/MacOS/pluginval"
elif [ "$OS_NAME" = "Linux" ]; then
    ASSET_URL="https://github.com/Tracktion/pluginval/releases/latest/download/pluginval_Linux.zip"
    APP_PATH="${PLUGINVAL_DIR}/pluginval"
else
    echo "ERROR: unsupported OS '$OS_NAME'. Install pluginval manually from https://github.com/Tracktion/pluginval/releases"
    exit 1
fi

echo "Downloading pluginval from $ASSET_URL..."
TMP_ZIP="$(mktemp -t pluginval.XXXXXX).zip"
trap 'rm -f "$TMP_ZIP"' EXIT
curl -fsSL -o "$TMP_ZIP" "$ASSET_URL"

echo "Extracting into $PLUGINVAL_DIR..."
rm -rf "${PLUGINVAL_DIR}/pluginval.app" "${PLUGINVAL_DIR}/pluginval"
unzip -q -o "$TMP_ZIP" -d "$PLUGINVAL_DIR"

if [ ! -x "$APP_PATH" ] && [ -f "$APP_PATH" ]; then
    chmod +x "$APP_PATH"
fi
if [ ! -f "$APP_PATH" ]; then
    echo "ERROR: expected binary at $APP_PATH after extract; contents:"
    ls -la "$PLUGINVAL_DIR"
    exit 1
fi

ln -sf "$APP_PATH" "$BIN_DIR/pluginval"
echo "Symlinked $BIN_DIR/pluginval -> $APP_PATH"

case ":$PATH:" in
    *:"$BIN_DIR":*) ;;
    *)
        echo
        echo "NOTE: $BIN_DIR is not on your PATH. Add this to your shell rc:"
        echo "  export PATH=\"$BIN_DIR:\$PATH\""
        ;;
esac

"$BIN_DIR/pluginval" --version || true
echo "pluginval install complete."
