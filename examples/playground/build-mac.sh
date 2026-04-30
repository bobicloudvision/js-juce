#!/usr/bin/env bash
set -eu

if (set -o pipefail) >/dev/null 2>&1; then
  set -o pipefail
fi

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
XCODEPROJ="${XCODEPROJ:-$PROJECT_DIR/Builds/MacOSX/playground.xcodeproj}"
CONFIGURATION="${CONFIGURATION:-Debug}"
SCHEME="${SCHEME:-playground - App}"
DERIVED_DATA="${DERIVED_DATA:-$PROJECT_DIR/Builds/DerivedData}"

if [[ ! -d "$XCODEPROJ" ]]; then
  echo "Missing Xcode project: $XCODEPROJ"
  echo "Open playground.jucer in Projucer and save/export first."
  exit 1
fi

if ! command -v xcodebuild >/dev/null 2>&1; then
  echo "xcodebuild is required but not installed."
  exit 1
fi

echo "Project       : $XCODEPROJ"
echo "Scheme        : $SCHEME"
echo "Configuration : $CONFIGURATION"
echo "DerivedData   : $DERIVED_DATA"

xcodebuild \
  -project "$XCODEPROJ" \
  -scheme "$SCHEME" \
  -configuration "$CONFIGURATION" \
  -derivedDataPath "$DERIVED_DATA" \
  build

echo "Playground build completed."
