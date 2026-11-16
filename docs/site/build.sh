#!/usr/bin/env bash
# Build the documentation site of libutils (docs/site -> docs/) with the better-html-doc skill
#   docs/site/build.sh [--check]     SKILLS=<folder holding better-html-doc> to change where the skills are
set -euo pipefail

SITE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$SITE/../.." && pwd)"
SKILLS="${SKILLS:-$HOME/.claude/skills}"
VERSION="v$(sed -n 's/^project([^)]*VERSION \([0-9.]*\).*/\1/p' "$ROOT/CMakeLists.txt")"

if [[ ! -f "$SKILLS/better-html-doc/scripts/build_site.py" ]]; then
    echo "build.sh: better-html-doc not found in $SKILLS (set SKILLS=<folder of the skills>)" >&2
    exit 1
fi

python3 "$SKILLS/better-html-doc/scripts/build_site.py" --site "$SITE" --out "$ROOT/docs" --version "$VERSION" --split
if [[ "${1:-}" == "--check" ]]; then
    python3 "$SKILLS/better-html-doc/scripts/check_site.py" "$ROOT/docs"
    node "$SKILLS/better-html-doc/scripts/test_search.js" "$ROOT/docs" "$SITE/search-cases.json"
fi
