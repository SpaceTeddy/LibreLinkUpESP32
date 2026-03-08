#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 1 ]]; then
  echo "Usage: $0 <version>"
  echo "Example: $0 1.2.0"
  exit 1
fi

VERSION="$1"

if [[ ! "$VERSION" =~ ^[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
  echo "Error: version must match MAJOR.MINOR.PATCH (e.g. 1.2.0)"
  exit 1
fi

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
LIB_JSON="$ROOT_DIR/library.json"
HEADER_FILE="$ROOT_DIR/include/librelinkup.h"

if [[ ! -f "$LIB_JSON" || ! -f "$HEADER_FILE" ]]; then
  echo "Error: expected files not found (library.json and include/librelinkup.h)"
  exit 1
fi

perl -0777 -i -pe "s/\"version\"\\s*:\\s*\"[^\"]+\"/\"version\": \"$VERSION\"/m" "$LIB_JSON"
perl -0777 -i -pe "s/#define\\s+VERSION_LIBRELINKUP_LIB\\s+\"[^\"]+\"/#define VERSION_LIBRELINKUP_LIB \"$VERSION\"/m" "$HEADER_FILE"
perl -0777 -i -pe "s/\\*\\s+\\@version\\s+[0-9]+\\.[0-9]+\\.[0-9]+/* \\@version $VERSION/m" "$HEADER_FILE"

echo "Updated version to $VERSION in:"
echo "- $LIB_JSON"
echo "- $HEADER_FILE"
echo
echo "Next steps:"
echo "1) ./scripts/check-version-consistency.sh"
echo "2) git diff"
echo "3) git commit -am \"chore: release $VERSION\""
echo "4) git tag v$VERSION"
