#!/usr/bin/env bash
set -eu

if (set -o pipefail) >/dev/null 2>&1; then
  set -o pipefail
fi

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${BUILD_DIR:-$PROJECT_ROOT/build/macos}"
BUILD_TYPE="${BUILD_TYPE:-Debug}"
GENERATOR="${GENERATOR:-Ninja}"
CONFIGURE_ONLY="${CONFIGURE_ONLY:-0}"

if [[ ! -f "$PROJECT_ROOT/CMakeLists.txt" ]]; then
  echo "Missing CMakeLists.txt in: $PROJECT_ROOT"
  echo "Create your JUCE app CMake project first, then rerun this script."
  exit 1
fi

if ! command -v cmake >/dev/null 2>&1; then
  echo "cmake is required but not installed."
  exit 1
fi

if [[ "$GENERATOR" == "Ninja" ]] && ! command -v ninja >/dev/null 2>&1; then
  echo "Ninja generator selected, but ninja is not installed."
  echo "Install ninja or run with: GENERATOR=\"Unix Makefiles\" ./build-mach.sh"
  exit 1
fi

echo "Project root : $PROJECT_ROOT"
echo "Build dir    : $BUILD_DIR"
echo "Build type   : $BUILD_TYPE"
echo "Generator    : $GENERATOR"

mkdir -p "$BUILD_DIR"

cmake -S "$PROJECT_ROOT" \
  -B "$BUILD_DIR" \
  -G "$GENERATOR" \
  -DCMAKE_BUILD_TYPE="$BUILD_TYPE"

if [[ "$CONFIGURE_ONLY" == "1" ]]; then
  echo "Configure completed (CONFIGURE_ONLY=1)."
  exit 0
fi

cmake --build "$BUILD_DIR" --config "$BUILD_TYPE"
echo "Build completed."
