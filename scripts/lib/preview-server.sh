#!/usr/bin/env bash
# preview-server.sh — sourceable helper that starts and stops the `astro
# preview` server the e2e gate tests against, and checks its port.
#
# Usage (from a project script):
#   source "$(dirname "$0")/lib/preview-server.sh"
#   preview_require_free_port "${PORT}" || exit 1
#   preview_start "${SITE_DIR}" "${PORT}" "${LOG}"   # sets PREVIEW_PID
#   ...
#   preview_stop
#   [ -z "$(preview_port_holder "${PORT}")" ] || echo "still listening"
#
# Why, two causes, both measured under /bin/bash 3.2: bash forks the last
# command of a `( … ) &` subshell, so the PID a script holds is a bash whose
# death orphans `npm exec` and the server; and Astro 7.3.8 detaches `astro
# preview` into a locked background server whenever it detects an AI agent
# (`isRunByAgent()`, which reads CLAUDECODE, AI_AGENT and similar), so even
# the right PID has exited at once. `exec` fixes the first and `--ignore-lock`
# the second — which also skips Astro's own one-server check, so the port is
# checked here instead. guide-parity M006/S01, GP15; measurements in
# docs/plans/guide-parity/M006-decisions.md.

# Prints "<pid> <command>" for the process listening on TCP <port>, or nothing
# when the port is free.
preview_port_holder() {
    local port="$1" pid
    if command -v lsof >/dev/null 2>&1; then
        pid="$(lsof -nP -iTCP:"${port}" -sTCP:LISTEN -t 2>/dev/null | head -n 1)"
        [ -n "${pid}" ] || return 0
        echo "${pid} $(ps -o command= -p "${pid}" 2>/dev/null)"
    elif curl -s -o /dev/null "http://localhost:${port}/"; then
        echo "unknown (lsof not found)"
    fi
}

# Returns 0 when <port> is free; otherwise names its holder and returns 1.
preview_require_free_port() {
    local port="$1" holder
    holder="$(preview_port_holder "${port}")"
    [ -z "${holder}" ] && return 0
    echo "=== FAIL: port ${port} is already in use by ${holder} ===" >&2
    echo "    stop it, or set POLY_PREVIEW_PORT to another port" >&2
    return 1
}

# Starts `astro preview` for <site_dir> on <port>, logging to <log>, and sets
# PREVIEW_PID to the subshell that runs it.
preview_start() {
    local site_dir="$1" port="$2" log="$3"
    # exec: bash forks the last command of a subshell, so without it the PID
    # is a bash whose death orphans npm exec and the server. With it the PID
    # is npm exec, which forwards SIGTERM to the server.
    # --ignore-lock: without it, an agent-run script gets a detached server
    # that outlives it (see the header).
    (cd "${site_dir}" && exec npx astro preview --port "${port}" --ignore-lock >"${log}" 2>&1) &
    PREVIEW_PID=$!
}

# Stops the server preview_start started, if it is still running.
preview_stop() {
    [ -n "${PREVIEW_PID:-}" ] || return 0
    if kill -0 "${PREVIEW_PID}" 2>/dev/null; then
        kill "${PREVIEW_PID}" 2>/dev/null || true
        # Give it a beat to die cleanly, then reap.
        for _ in 1 2 3 4 5; do
            kill -0 "${PREVIEW_PID}" 2>/dev/null || break
            sleep 0.2
        done
        kill -9 "${PREVIEW_PID}" 2>/dev/null || true
        wait "${PREVIEW_PID}" 2>/dev/null || true
    fi
}
