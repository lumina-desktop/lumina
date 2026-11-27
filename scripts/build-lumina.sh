#!/usr/bin/env bash
# Build and install the Lumina desktop (Qt6) from a git checkout.


set -euo pipefail

usage() {
    cat <<EOF
Usage: $(basename "$0") [options]

  -s, --src DIR       Lumina checkout (default: repo containing this script)
  -b, --branch NAME   Checkout this branch before building (default: current)
  -B, --build DIR     Build directory (default: <src>/build-lumina-qt6)
  -p, --prefix DIR    Install prefix (default: /usr/local)
  -t, --type TYPE     CMAKE_BUILD_TYPE (default: Debug)
      --qt5           Build the Qt5 tree instead (WITH_QT6=OFF)
  -c, --clean         Wipe the build directory first (automatic if the
                      CMake cache came from a different path)
  -j, --jobs N        Parallel jobs (default: CPU count)
      --pull          git pull (fast-forward only) before building
      --no-install    Build only, don't install
  -h, --help          Show this help

Full output is also written to <build>/build.log.
EOF
}

die() { echo "Error: $*" >&2; exit 1; }

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SRC="$(cd "$SCRIPT_DIR/.." && pwd)"
BRANCH=""
BUILD=""
PREFIX="/usr/local"
TYPE="Debug"
WITH_QT6="ON"
CLEAN=0
PULL=0
INSTALL=1
JOBS=""

# Options that take a value: make sure one was actually given.
need_arg() { [ $# -ge 2 ] && [ -n "$2" ] || { echo "Option $1 requires a value" >&2; usage >&2; exit 2; }; }

while [ $# -gt 0 ]; do
    case "$1" in
        -s|--src)     need_arg "$@"; SRC="$2"; shift ;;
        -b|--branch)  need_arg "$@"; BRANCH="$2"; shift ;;
        -B|--build)   need_arg "$@"; BUILD="$2"; shift ;;
        -p|--prefix)  need_arg "$@"; PREFIX="$2"; shift ;;
        -t|--type)    need_arg "$@"; TYPE="$2"; shift ;;
        --qt5)        WITH_QT6="OFF" ;;
        -c|--clean)   CLEAN=1 ;;
        -j|--jobs)    need_arg "$@"; JOBS="$2"; shift ;;
        --pull)       PULL=1 ;;
        --no-install) INSTALL=0 ;;
        -h|--help)    usage; exit 0 ;;
        *) echo "Unknown option: $1" >&2; usage >&2; exit 2 ;;
    esac
    shift
done

command -v cmake >/dev/null 2>&1 || die "cmake not found — install cmake, a C++ compiler and the Qt development packages first."
[ -f "$SRC/CMakeLists.txt" ] || die "$SRC does not look like a Lumina checkout (no CMakeLists.txt)."
SRC="$(cd "$SRC" && pwd)"

# Use a dedicated build dir so we never collide with hand-made build dirs.
if [ -z "$BUILD" ]; then
    if [ "$WITH_QT6" = "ON" ]; then BUILD="$SRC/build-lumina-qt6"; else BUILD="$SRC/build-lumina-qt5"; fi
fi

if [ -z "$JOBS" ]; then
    JOBS="$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 2)"
fi

# Privilege escalation for install (Linux: sudo, FreeBSD often: doas)
if [ "$(id -u)" -eq 0 ]; then
    SUDO=""
elif command -v sudo >/dev/null 2>&1; then
    SUDO="sudo"
elif command -v doas >/dev/null 2>&1; then
    SUDO="doas"
else
    SUDO=""
fi

step() { printf '\n\033[1;34m==> %s\033[0m\n' "$*"; }
head_desc() { git -C "$SRC" log -1 --oneline 2>/dev/null || echo "(not a git checkout)"; }

mkdir -p "$BUILD"
BUILD="$(cd "$BUILD" && pwd)"
LOG="$BUILD/build.log"
: > "$LOG"
exec > >(tee -a "$LOG") 2>&1

START=$(date +%s)
step "Lumina build on $(hostname) ($(uname -sr))"
echo "src=$SRC build=$BUILD prefix=$PREFIX type=$TYPE qt6=$WITH_QT6 jobs=$JOBS"

cd "$SRC"
if [ -n "$BRANCH" ]; then
    step "Checking out $BRANCH"
    [ "$PULL" -eq 1 ] && git fetch --all --prune
    git checkout "$BRANCH"
fi
if [ "$PULL" -eq 1 ]; then
    step "Pulling latest"
    git pull --ff-only
fi
echo "HEAD: $(head_desc)"

# A CMakeCache from another path/source (copied or committed build dir) is
# unusable; wipe it instead of letting cmake fail.
if [ -f "$BUILD/CMakeCache.txt" ] && [ "$CLEAN" -eq 0 ]; then
    cached_src="$(sed -n 's/^CMAKE_HOME_DIRECTORY:INTERNAL=//p' "$BUILD/CMakeCache.txt")"
    cached_bin="$(sed -n 's/^CMAKE_CACHEFILE_DIR:INTERNAL=//p' "$BUILD/CMakeCache.txt")"
    if [ "$cached_src" != "$SRC" ] || [ "$cached_bin" != "$BUILD" ]; then
        echo "Stale CMake cache (src=$cached_src, build=$cached_bin) — forcing clean"
        CLEAN=1
    fi
fi

if [ "$CLEAN" -eq 1 ]; then
    # Only ever wipe something that is recognisably a CMake build dir (or is
    # empty apart from our log), so a mistyped --build can't delete real files.
    if [ -f "$BUILD/CMakeCache.txt" ] || [ -d "$BUILD/CMakeFiles" ]; then
        step "Cleaning $BUILD"
        # Keep the log we're writing to
        find "$BUILD" -mindepth 1 ! -name build.log -exec rm -rf {} + 2>/dev/null || true
    elif [ -n "$(find "$BUILD" -mindepth 1 ! -name build.log -print -quit)" ]; then
        die "refusing to clean $BUILD: it has files but is not a CMake build directory."
    fi
fi

step "Configuring"
cmake -S "$SRC" -B "$BUILD" \
    -DWITH_QT6="$WITH_QT6" \
    -DCMAKE_BUILD_TYPE="$TYPE" \
    -DCMAKE_INSTALL_PREFIX="$PREFIX"

step "Building ($JOBS jobs)"
cmake --build "$BUILD" --parallel "$JOBS"

if [ "$INSTALL" -eq 1 ]; then
    step "Installing to $PREFIX"
    $SUDO cmake --install "$BUILD"
fi

step "Done in $(( $(date +%s) - START ))s — $(head_desc)"
if [ "$INSTALL" -eq 1 ]; then
    echo "Installed to $PREFIX. Log out and pick Lumina in your display manager, or run start-lumina-desktop from an X session."
else
    echo "Built in $BUILD (not installed)."
fi
