#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"

if ! command -v pyinstaller >/dev/null 2>&1; then
  echo "pyinstaller not found. Install with: pip install -r python/requirements-dev.txt"
  exit 1
fi

rm -rf build dist

pyinstaller \
  --noconfirm \
  --clean \
  --name ExpenseTracker \
  --windowed \
  --onedir \
  --add-data "shared/ui:shared/ui" \
  --add-data "shared/resources:shared/resources" \
  --add-data "shared/data:shared/data" \
  python/main.py

echo "Build complete: dist/ExpenseTracker"