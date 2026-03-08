#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
LIB_JSON="$ROOT_DIR/library.json"
HEADER_FILE="$ROOT_DIR/include/librelinkup.h"

if [[ ! -f "$LIB_JSON" || ! -f "$HEADER_FILE" ]]; then
  echo "Error: expected files not found (library.json and include/librelinkup.h)"
  exit 1
fi

LIB_VERSION="$(sed -nE 's/^[[:space:]]*"version":[[:space:]]*"([^"]+)".*/\1/p' "$LIB_JSON" | head -n1)"
HEADER_VERSION="$(sed -nE 's/^#define[[:space:]]+VERSION_LIBRELINKUP_LIB[[:space:]]+"([^"]+)".*/\1/p' "$HEADER_FILE" | head -n1)"
DOC_VERSION="$(sed -nE 's/^[[:space:]]*\*[[:space:]]+@version[[:space:]]+([0-9]+\.[0-9]+\.[0-9]+).*/\1/p' "$HEADER_FILE" | head -n1)"

if [[ -z "$LIB_VERSION" || -z "$HEADER_VERSION" || -z "$DOC_VERSION" ]]; then
  echo "Error: failed to parse one or more version fields"
  echo "library.json: '$LIB_VERSION'"
  echo "macro        : '$HEADER_VERSION'"
  echo "docblock     : '$DOC_VERSION'"
  exit 1
fi

if [[ "$LIB_VERSION" != "$HEADER_VERSION" || "$LIB_VERSION" != "$DOC_VERSION" ]]; then
  echo "Version mismatch detected:"
  echo "- library.json version       : $LIB_VERSION"
  echo "- header macro version       : $HEADER_VERSION"
  echo "- header docblock @version   : $DOC_VERSION"
  exit 1
fi

echo "Version check OK: $LIB_VERSION"
