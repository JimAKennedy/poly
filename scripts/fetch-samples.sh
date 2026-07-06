#!/usr/bin/env bash
# fetch-samples.sh — fetch CC0 / CC-BY drum sample one-shots into
# site/public/samples/<role>/ from pinned upstream repos.
#
# Contract:
#   - Idempotent: rerunning must exit 0 and copy zero additional files.
#   - Deterministic: every repo is pinned to a specific commit SHA.
#   - No external runtime beyond git and standard POSIX tools.
#
# Exit codes:
#   0 success
#   1 usage error
#   2 git / network failure
#   3 pinned SHA mismatch (checkout produced unexpected HEAD)
#   4 expected upstream file missing after checkout (SHA-bump smoke signal)

set -euo pipefail

# -----------------------------------------------------------------------------
# Pinned upstream SHAs (captured 2026-07-06 via `git ls-remote <url> HEAD`).
# -----------------------------------------------------------------------------
readonly VCSL_URL="https://github.com/sgossner/VCSL"
readonly VCSL_SHA="c1ea7bcc3c7309650ab0da9d15c9cd1fbc4a4c7e"

readonly FISCHER808_URL="https://github.com/tidalcycles/sounds-tr808-fischer"
readonly FISCHER808_SHA="85fbecf1bec32553395625ea659e2a56dfd7c0e1"

readonly FREEPATS_URL="https://github.com/freepats/world-percussion"
readonly FREEPATS_SHA="e54eb2912a0d6d4444ab205d52f778e27da0fc96"

readonly BOOCHI44_URL="https://github.com/Boochi44/free-drum-samples"
readonly BOOCHI44_SHA="77ba31428a079dd8f17c8e144c1e649ea0a198b3"

readonly MULDJORD_URL="https://github.com/freepats/MuldjordKit"
readonly MULDJORD_SHA="719fe72bc6693b94f1229674e202881145ab44ed"

readonly DRSKIT_URL="https://github.com/sfzinstruments/DrumGizmo.DRSKit"
readonly DRSKIT_SHA="b01448990a31f722542f1812e56159a97a6a2a82"

readonly DIMCABASA_URL="https://github.com/sfzinstruments/kinwie.dim-cabasa"
readonly DIMCABASA_SHA="016457e51a3e6558ab49676789260b54ff52ff1b"

# -----------------------------------------------------------------------------
# Paths
# -----------------------------------------------------------------------------
readonly REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
readonly SAMPLES_DIR="${REPO_ROOT}/site/public/samples"
readonly CACHE_DIR="${REPO_ROOT}/.cache/samples"

# -----------------------------------------------------------------------------
# Log helpers
# -----------------------------------------------------------------------------
log() { printf '%s\n' "$*" >&2; }
die() {
    local code="$1"; shift
    printf 'error: %s\n' "$*" >&2
    exit "$code"
}

# -----------------------------------------------------------------------------
# fetch_git_sparse <name> <url> <sha> <workdir> <path1> [path2 ...]
#   Sparse-checkout at pinned SHA. Used for multi-GB repos where a full clone
#   is untenable (VCSL). No-op if <workdir>/.git already exists and HEAD
#   matches <sha>.
# -----------------------------------------------------------------------------
fetch_git_sparse() {
    local name="$1" url="$2" sha="$3" workdir="$4"
    shift 4
    local paths=("$@")

    if [ -d "${workdir}/.git" ]; then
        local actual
        actual="$(git -C "${workdir}" rev-parse HEAD 2>/dev/null || true)"
        if [ "${actual}" = "${sha}" ]; then
            return 0
        fi
        rm -rf "${workdir}"
    fi

    mkdir -p "${workdir}"
    git -C "${workdir}" init -q || die 2 "${name}: git init failed"
    git -C "${workdir}" remote add origin "${url}" \
        || die 2 "${name}: git remote add failed"
    git -C "${workdir}" config core.sparseCheckout true
    git -C "${workdir}" sparse-checkout init --no-cone >/dev/null 2>&1 \
        || die 2 "${name}: sparse-checkout init failed"
    git -C "${workdir}" sparse-checkout set "${paths[@]}" >/dev/null 2>&1 \
        || die 2 "${name}: sparse-checkout set failed"
    # --filter=blob:none skips downloading blobs during fetch; sparse-checkout
    # + checkout then pulls only the blobs whose paths match the sparse spec.
    # Critical for VCSL, which is multi-GB when fully cloned.
    git -C "${workdir}" fetch --filter=blob:none --depth 1 origin "${sha}" \
        >/dev/null 2>&1 \
        || die 2 "${name}: fetch of ${sha} failed"
    git -C "${workdir}" checkout -q FETCH_HEAD \
        || die 2 "${name}: checkout FETCH_HEAD failed"

    local checked
    checked="$(git -C "${workdir}" rev-parse HEAD)"
    if [ "${checked}" != "${sha}" ]; then
        die 3 "${name}: expected SHA ${sha}, got ${checked}"
    fi
}

# -----------------------------------------------------------------------------
# fetch_git_full <name> <url> <sha> <workdir>
#   Full shallow clone at pinned SHA. Used for small repos where sparse-checkout
#   is unnecessary overhead. Idempotent.
# -----------------------------------------------------------------------------
fetch_git_full() {
    local name="$1" url="$2" sha="$3" workdir="$4"

    if [ -d "${workdir}/.git" ]; then
        local actual
        actual="$(git -C "${workdir}" rev-parse HEAD 2>/dev/null || true)"
        if [ "${actual}" = "${sha}" ]; then
            return 0
        fi
        rm -rf "${workdir}"
    fi

    mkdir -p "${workdir}"
    git -C "${workdir}" init -q || die 2 "${name}: git init failed"
    git -C "${workdir}" remote add origin "${url}" \
        || die 2 "${name}: git remote add failed"
    git -C "${workdir}" fetch --depth 1 origin "${sha}" >/dev/null 2>&1 \
        || die 2 "${name}: fetch of ${sha} failed"
    git -C "${workdir}" checkout -q FETCH_HEAD \
        || die 2 "${name}: checkout FETCH_HEAD failed"

    local checked
    checked="$(git -C "${workdir}" rev-parse HEAD)"
    if [ "${checked}" != "${sha}" ]; then
        die 3 "${name}: expected SHA ${sha}, got ${checked}"
    fi
}

# -----------------------------------------------------------------------------
# copy_if_missing <src> <dest>
#   Enforces the no-op-on-rerun contract. Skips when <dest> already exists.
#   Emits `[copy] role=<dir> file=<basename>` and increments COPIED counter.
#   Exits 4 if <src> does not exist (SHA-bump smoke signal).
# -----------------------------------------------------------------------------
COPIED=0
copy_if_missing() {
    local src="$1" dest="$2"
    if [ ! -f "${src}" ]; then
        die 4 "expected upstream file not found: ${src}"
    fi
    if [ -f "${dest}" ]; then
        return 0
    fi
    mkdir -p "$(dirname "${dest}")"
    cp "${src}" "${dest}"
    local role file
    role="$(basename "$(dirname "${dest}")")"
    file="$(basename "${dest}")"
    log "[copy] role=${role} file=${file}"
    COPIED=$((COPIED + 1))
}

# -----------------------------------------------------------------------------
# Placeholder role-source functions. T02 (VCSL) and T03 (Fischer808, FreePats,
# Boochi44) replace these with real implementations.
# -----------------------------------------------------------------------------
fetch_vcsl() {
    local -a targets=(
        "${SAMPLES_DIR}/agogo/vcsl-hi.wav"
        "${SAMPLES_DIR}/agogo/vcsl-lo.wav"
        "${SAMPLES_DIR}/cowbell/vcsl-cowbell.wav"
        "${SAMPLES_DIR}/shaker/vcsl-cabasa.wav"
        "${SAMPLES_DIR}/clave/vcsl-claves.wav"
        "${SAMPLES_DIR}/woodblock/vcsl-hi.wav"
        "${SAMPLES_DIR}/woodblock/vcsl-lo.wav"
        "${SAMPLES_DIR}/conga/vcsl-conga-open.wav"
        "${SAMPLES_DIR}/snare/vcsl-snare.wav"
    )
    local missing=0
    for t in "${targets[@]}"; do
        [ -f "${t}" ] || missing=$((missing + 1))
    done
    if [ "${missing}" -eq 0 ]; then
        log "[fetch] repo=vcsl sha=${VCSL_SHA:0:7} mode=sparse files=0"
        return 0
    fi

    local workdir="${CACHE_DIR}/vcsl"
    local -a sparse=(
        "/Idiophones/Struck Idiophones/Agogo Bells/*.wav"
        "/Idiophones/Struck Idiophones/Cowbells/*.wav"
        "/Idiophones/Struck Idiophones/Cabasa/*.wav"
        "/Idiophones/Struck Idiophones/Claves/*.wav"
        "/Idiophones/Struck Idiophones/Woodblock/*.wav"
        "/Membranophones/Struck Membranophones/Conga/*.wav"
        "/Membranophones/Struck Membranophones/Snare Drum, Modern 1/*.wav"
    )
    fetch_git_sparse vcsl "${VCSL_URL}" "${VCSL_SHA}" "${workdir}" "${sparse[@]}"

    local ib="${workdir}/Idiophones/Struck Idiophones"
    local mb="${workdir}/Membranophones/Struck Membranophones"
    local before="${COPIED}"

    copy_if_missing "${ib}/Agogo Bells/Agogo_High_v1_rr1_Mid.wav" \
                    "${SAMPLES_DIR}/agogo/vcsl-hi.wav"
    copy_if_missing "${ib}/Agogo Bells/Agogo_Low_v1_rr1_Mid.wav" \
                    "${SAMPLES_DIR}/agogo/vcsl-lo.wav"
    copy_if_missing "${ib}/Cowbells/Cowbell1_Hit_v2_rr1_Mid.wav" \
                    "${SAMPLES_DIR}/cowbell/vcsl-cowbell.wav"
    copy_if_missing "${ib}/Cabasa/Cabasa1_Hit_rr1_Mid.wav" \
                    "${SAMPLES_DIR}/shaker/vcsl-cabasa.wav"
    copy_if_missing "${ib}/Claves/Claves1_Hit_v1_rr1_Mid.wav" \
                    "${SAMPLES_DIR}/clave/vcsl-claves.wav"
    copy_if_missing "${ib}/Woodblock/wood_click_mp.wav" \
                    "${SAMPLES_DIR}/woodblock/vcsl-hi.wav"
    copy_if_missing "${ib}/Woodblock/wood_click3_vl1.wav" \
                    "${SAMPLES_DIR}/woodblock/vcsl-lo.wav"
    copy_if_missing "${mb}/Conga/Conga_HitN_v1_rr1_Sum.wav" \
                    "${SAMPLES_DIR}/conga/vcsl-conga-open.wav"
    copy_if_missing "${mb}/Snare Drum, Modern 1/Snare2_HitNS_v2_rr1_Mid.wav" \
                    "${SAMPLES_DIR}/snare/vcsl-snare.wav"

    local added=$((COPIED - before))
    log "[fetch] repo=vcsl sha=${VCSL_SHA:0:7} mode=sparse files=${added}"
    rm -rf "${workdir}"
}

fetch_fischer808() {
    local -a targets=(
        "${SAMPLES_DIR}/kick/fischer808-bd.wav"
        "${SAMPLES_DIR}/snare/fischer808-sd.wav"
        "${SAMPLES_DIR}/hat/fischer808-ch.wav"
        "${SAMPLES_DIR}/hat/fischer808-oh.wav"
        "${SAMPLES_DIR}/clave/fischer808-clave.wav"
        "${SAMPLES_DIR}/cowbell/fischer808-cowbell.wav"
        "${SAMPLES_DIR}/shaker/fischer808-maraca.wav"
    )
    local missing=0
    for t in "${targets[@]}"; do
        [ -f "${t}" ] || missing=$((missing + 1))
    done
    if [ "${missing}" -eq 0 ]; then
        log "[fetch] repo=fischer808 sha=${FISCHER808_SHA:0:7} mode=full files=0"
        return 0
    fi

    local workdir="${CACHE_DIR}/fischer808"
    fetch_git_full fischer808 "${FISCHER808_URL}" "${FISCHER808_SHA}" "${workdir}"
    local before="${COPIED}"

    # BD0000 = default kick (0 tone / 0 decay / 0 tune). SD0000 = default snare.
    # CH.WAV = the sole closed hat. OH00.WAV = open hat with decay=0.
    # CL/CB/MA = single-sample clave / cowbell / maraca.
    copy_if_missing "${workdir}/bd8/BD0000.WAV" \
                    "${SAMPLES_DIR}/kick/fischer808-bd.wav"
    copy_if_missing "${workdir}/sd8/SD0000.WAV" \
                    "${SAMPLES_DIR}/snare/fischer808-sd.wav"
    copy_if_missing "${workdir}/ch8/CH.WAV" \
                    "${SAMPLES_DIR}/hat/fischer808-ch.wav"
    copy_if_missing "${workdir}/oh8/OH00.WAV" \
                    "${SAMPLES_DIR}/hat/fischer808-oh.wav"
    copy_if_missing "${workdir}/cl8/CL.WAV" \
                    "${SAMPLES_DIR}/clave/fischer808-clave.wav"
    copy_if_missing "${workdir}/cb8/CB.WAV" \
                    "${SAMPLES_DIR}/cowbell/fischer808-cowbell.wav"
    copy_if_missing "${workdir}/ma8/MA.WAV" \
                    "${SAMPLES_DIR}/shaker/fischer808-maraca.wav"

    local added=$((COPIED - before))
    log "[fetch] repo=fischer808 sha=${FISCHER808_SHA:0:7} mode=full files=${added}"
    rm -rf "${workdir}"
}

fetch_freepats() {
    # Format policy: WAV and FLAC are both accepted (browser Web Audio
    # decodeAudioData() decodes both natively; the plugin itself never touches
    # these files). FreePats ships FLAC and WAV in every folder. S02 landed
    # shaker/freepats-egg.wav; S04 backfills 7 FLAC picks (Bongos, CajonFlamenco,
    # Castanets, Claves, Conga, Darbuka, HandClap) plus the Tambourine WAV.
    local -a targets=(
        "${SAMPLES_DIR}/shaker/freepats-egg.wav"
        "${SAMPLES_DIR}/bongo/freepats-bongo-hi.flac"
        "${SAMPLES_DIR}/cajon/freepats-cajon.flac"
        "${SAMPLES_DIR}/castanets/freepats-castanets.flac"
        "${SAMPLES_DIR}/clave/freepats-claves.flac"
        "${SAMPLES_DIR}/conga/freepats-conga-open.flac"
        "${SAMPLES_DIR}/darbuka/freepats-darbuka.flac"
        "${SAMPLES_DIR}/handclap/freepats-clap.flac"
        "${SAMPLES_DIR}/tambourine/freepats-tambourine.wav"
    )
    local missing=0
    for t in "${targets[@]}"; do
        [ -f "${t}" ] || missing=$((missing + 1))
    done
    if [ "${missing}" -eq 0 ]; then
        log "[fetch] repo=freepats sha=${FREEPATS_SHA:0:7} mode=full files=0"
        return 0
    fi

    local workdir="${CACHE_DIR}/freepats"
    fetch_git_full freepats "${FREEPATS_URL}" "${FREEPATS_SHA}" "${workdir}"
    local before="${COPIED}"

    copy_if_missing "${workdir}/samples/EggShaker/slow_01.wav" \
                    "${SAMPLES_DIR}/shaker/freepats-egg.wav"
    copy_if_missing "${workdir}/samples/Bongos/1_01.flac" \
                    "${SAMPLES_DIR}/bongo/freepats-bongo-hi.flac"
    copy_if_missing "${workdir}/samples/CajonFlamenco/101.flac" \
                    "${SAMPLES_DIR}/cajon/freepats-cajon.flac"
    copy_if_missing "${workdir}/samples/Castanets/01.flac" \
                    "${SAMPLES_DIR}/castanets/freepats-castanets.flac"
    copy_if_missing "${workdir}/samples/Claves/01.flac" \
                    "${SAMPLES_DIR}/clave/freepats-claves.flac"
    copy_if_missing "${workdir}/samples/Conga/v2_01_01.flac" \
                    "${SAMPLES_DIR}/conga/freepats-conga-open.flac"
    copy_if_missing "${workdir}/samples/Darbuka/doom_01_01.flac" \
                    "${SAMPLES_DIR}/darbuka/freepats-darbuka.flac"
    copy_if_missing "${workdir}/samples/HandClap/01_02.flac" \
                    "${SAMPLES_DIR}/handclap/freepats-clap.flac"
    copy_if_missing "${workdir}/samples/Tambourine/fast_03.wav" \
                    "${SAMPLES_DIR}/tambourine/freepats-tambourine.wav"

    local added=$((COPIED - before))
    log "[fetch] repo=freepats sha=${FREEPATS_SHA:0:7} mode=full files=${added}"
    rm -rf "${workdir}"
}

fetch_muldjord() {
    local -a targets=(
        "${SAMPLES_DIR}/kick/muldjord-kick.flac"
        "${SAMPLES_DIR}/snare/muldjord-snare.flac"
        "${SAMPLES_DIR}/hat/muldjord-hat.flac"
        "${SAMPLES_DIR}/tom/muldjord-tom-hi.flac"
        "${SAMPLES_DIR}/tom/muldjord-tom-mid.flac"
        "${SAMPLES_DIR}/tom/muldjord-tom-lo.flac"
    )
    local missing=0
    for t in "${targets[@]}"; do
        [ -f "${t}" ] || missing=$((missing + 1))
    done
    if [ "${missing}" -eq 0 ]; then
        log "[fetch] repo=muldjord sha=${MULDJORD_SHA:0:7} mode=full files=0"
        return 0
    fi

    local workdir="${CACHE_DIR}/muldjord"
    fetch_git_full muldjord "${MULDJORD_URL}" "${MULDJORD_SHA}" "${workdir}"
    local before="${COPIED}"

    # MuldjordKit velocity layers are numbered <n>-<name>.flac, with higher
    # numbers = louder velocities. Pick the top layer per drum for a full-hit
    # single-shot suitable for a demo. Toms: Tom1=highest pitch, Tom4=lowest
    # (floor); Tom2 fills the middle. Skip Tom3 to widen the spread.
    copy_if_missing "${workdir}/samples/KdrumL/25-KdrumL.flac" \
                    "${SAMPLES_DIR}/kick/muldjord-kick.flac"
    copy_if_missing "${workdir}/samples/Snare1/56-Snare.flac" \
                    "${SAMPLES_DIR}/snare/muldjord-snare.flac"
    copy_if_missing "${workdir}/samples/HihatClosed/29-HihatClosed.flac" \
                    "${SAMPLES_DIR}/hat/muldjord-hat.flac"
    copy_if_missing "${workdir}/samples/Tom1/11-Tom1.flac" \
                    "${SAMPLES_DIR}/tom/muldjord-tom-hi.flac"
    copy_if_missing "${workdir}/samples/Tom2/13-Tom2.flac" \
                    "${SAMPLES_DIR}/tom/muldjord-tom-mid.flac"
    copy_if_missing "${workdir}/samples/Tom4/20-Tom4.flac" \
                    "${SAMPLES_DIR}/tom/muldjord-tom-lo.flac"

    local added=$((COPIED - before))
    log "[fetch] repo=muldjord sha=${MULDJORD_SHA:0:7} mode=full files=${added}"
    rm -rf "${workdir}"
}

fetch_drskit() {
    local -a targets=(
        "${SAMPLES_DIR}/kick/drskit-kick.flac"
        "${SAMPLES_DIR}/snare/drskit-snare.flac"
    )
    local missing=0
    for t in "${targets[@]}"; do
        [ -f "${t}" ] || missing=$((missing + 1))
    done
    if [ "${missing}" -eq 0 ]; then
        log "[fetch] repo=drskit sha=${DRSKIT_SHA:0:7} mode=full files=0"
        return 0
    fi

    local workdir="${CACHE_DIR}/drskit"
    fetch_git_full drskit "${DRSKIT_URL}" "${DRSKIT_SHA}" "${workdir}"
    local before="${COPIED}"

    # DRSKit is a multi-mic recording — each hit produces AmbL/AmbR/OHL/OHR
    # plus per-drum close mics and bleed. Pick the close mic for the played
    # drum at the top velocity layer. Kdrum has 22 layers, Snare has 49.
    local samples="${workdir}/DrumGizmo/DRSKit/Samples"
    copy_if_missing "${samples}/Kdrum_with_contact/22-Kdrum_with_contact-Kdrum_front.flac" \
                    "${SAMPLES_DIR}/kick/drskit-kick.flac"
    copy_if_missing "${samples}/Snare/49-Snare-Snare_top.flac" \
                    "${SAMPLES_DIR}/snare/drskit-snare.flac"

    local added=$((COPIED - before))
    log "[fetch] repo=drskit sha=${DRSKIT_SHA:0:7} mode=full files=${added}"
    rm -rf "${workdir}"
}

fetch_dimcabasa() {
    local target="${SAMPLES_DIR}/shaker/dimcabasa-cabasa.flac"
    if [ -f "${target}" ]; then
        log "[fetch] repo=dimcabasa sha=${DIMCABASA_SHA:0:7} mode=full files=0"
        return 0
    fi

    local workdir="${CACHE_DIR}/dimcabasa"
    fetch_git_full dimcabasa "${DIMCABASA_URL}" "${DIMCABASA_SHA}" "${workdir}"
    local before="${COPIED}"

    # Kinwie's Dim Cabasa: `Dim Cabasa Samples/bw_<velocity>_<round-robin>.flac`.
    # rr1_01 = velocity layer 1, round-robin 1 — smallest deterministic pick.
    copy_if_missing "${workdir}/Dim Cabasa Samples/bw_rr1_01.flac" \
                    "${SAMPLES_DIR}/shaker/dimcabasa-cabasa.flac"

    local added=$((COPIED - before))
    log "[fetch] repo=dimcabasa sha=${DIMCABASA_SHA:0:7} mode=full files=${added}"
    rm -rf "${workdir}"
}

fetch_boochi44() {
    local -a targets=(
        "${SAMPLES_DIR}/kick/boochi44-kick.wav"
        "${SAMPLES_DIR}/hat/boochi44-hat.wav"
    )
    local missing=0
    for t in "${targets[@]}"; do
        [ -f "${t}" ] || missing=$((missing + 1))
    done
    if [ "${missing}" -eq 0 ]; then
        log "[fetch] repo=boochi44 sha=${BOOCHI44_SHA:0:7} mode=full files=0"
        return 0
    fi

    local workdir="${CACHE_DIR}/boochi44"
    fetch_git_full boochi44 "${BOOCHI44_URL}" "${BOOCHI44_SHA}" "${workdir}"
    local before="${COPIED}"

    copy_if_missing "${workdir}/drum-samples/01-hard-trap/kicks/hard-kick-01.wav" \
                    "${SAMPLES_DIR}/kick/boochi44-kick.wav"
    copy_if_missing "${workdir}/drum-samples/01-hard-trap/hi-hats/hi-hat-closed-01.wav" \
                    "${SAMPLES_DIR}/hat/boochi44-hat.wav"

    local added=$((COPIED - before))
    log "[fetch] repo=boochi44 sha=${BOOCHI44_SHA:0:7} mode=full files=${added}"
    rm -rf "${workdir}"
}

# -----------------------------------------------------------------------------
# Ensure .cache/ is gitignored so transient clones never enter the tree.
# -----------------------------------------------------------------------------
ensure_gitignore() {
    local gi="${REPO_ROOT}/.gitignore"
    if [ ! -f "${gi}" ] || ! grep -q '^\.cache/' "${gi}"; then
        printf '\n# Sample-fetch scratch dir\n.cache/\n' >> "${gi}"
        log "[gitignore] added .cache/"
    fi
}

# -----------------------------------------------------------------------------
# Arg parsing
# -----------------------------------------------------------------------------
usage() {
    cat <<'EOF'
usage: fetch-samples.sh [--sources=cc0|ccby|all] [--only=<repo>] [--help]

Fetch drum sample one-shots into site/public/samples/<role>/ from pinned
upstream repos. Idempotent: rerunning copies zero additional files.

Options:
  --sources=SET   Which license bucket to fetch. Default: cc0.
                  Values: cc0 | ccby | all.
  --only=REPO     Restrict to a single upstream repo. Useful for CI matrix and
                  local iteration. Values: vcsl | fischer808 | freepats |
                  boochi44 | muldjord | drskit | dimcabasa | all. Default: all.
  --help          Show this message and exit 0.

Exit codes:
  0  success
  1  usage error
  2  git / network failure
  3  pinned SHA mismatch
  4  expected upstream file missing after checkout
EOF
}

SOURCES="cc0"
ONLY="all"

for arg in "$@"; do
    case "${arg}" in
        --help|-h)
            usage
            exit 0
            ;;
        --sources=*)
            SOURCES="${arg#--sources=}"
            ;;
        --only=*)
            ONLY="${arg#--only=}"
            ;;
        *)
            usage >&2
            die 1 "unknown argument: ${arg}"
            ;;
    esac
done

case "${SOURCES}" in
    cc0|ccby|all) ;;
    *)
        usage >&2
        die 1 "invalid --sources value: ${SOURCES}"
        ;;
esac

case "${ONLY}" in
    all|vcsl|fischer808|freepats|boochi44|muldjord|drskit|dimcabasa) ;;
    *)
        usage >&2
        die 1 "invalid --only value: ${ONLY}"
        ;;
esac

# -----------------------------------------------------------------------------
# Main dispatch
# -----------------------------------------------------------------------------
mkdir -p "${SAMPLES_DIR}" "${CACHE_DIR}"
ensure_gitignore

should_run() {
    [ "${ONLY}" = "all" ] || [ "${ONLY}" = "$1" ]
}

if [ "${SOURCES}" = "cc0" ] || [ "${SOURCES}" = "all" ]; then
    should_run vcsl        && fetch_vcsl
    should_run fischer808  && fetch_fischer808
    should_run freepats    && fetch_freepats
    should_run boochi44    && fetch_boochi44
fi

if [ "${SOURCES}" = "ccby" ] || [ "${SOURCES}" = "all" ]; then
    should_run muldjord  && fetch_muldjord
    should_run drskit    && fetch_drskit
    should_run dimcabasa && fetch_dimcabasa
fi

# Report the CURRENT total (not the per-run newly-copied count) so CI can grep
# `[done] cc0: 19 files across 9 role dirs` on any invocation, not just the
# first one. Per-run deltas are already visible in the `[fetch] ... files=N`
# lines above.
TOTAL_FILES=0
ROLE_DIRS=0
if [ -d "${SAMPLES_DIR}" ]; then
    TOTAL_FILES="$(find "${SAMPLES_DIR}" -mindepth 2 -type f \
        \( -name '*.wav' -o -name '*.flac' \) \
        | wc -l | tr -d ' ')"
    ROLE_DIRS="$(find "${SAMPLES_DIR}" -mindepth 2 -type f \
        \( -name '*.wav' -o -name '*.flac' \) \
        -exec dirname {} \; | sort -u | wc -l | tr -d ' ')"
fi

log "[done] ${SOURCES}: ${TOTAL_FILES} files across ${ROLE_DIRS} role dirs"
