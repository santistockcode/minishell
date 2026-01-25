#!/usr/bin/env bash
# checks staged files with norminette
# - SKIP_NORM=1 bypasses the whole hook
# - add the token "NORM_SKIP" anywhere in a file to skip that file

set -euo pipefail

# allow full bypass
if [ "${SKIP_NORM:-0}" = "1" ]; then
  echo "[check_norm] SKIP_NORM=1 -> skipping norminette checks"
  exit 0
fi

# require norminette in PATH
if ! command -v norminette >/dev/null 2>&1; then
  echo "ERROR: norminette not found in PATH. Install it or activate your venv." >&2
  exit 2
fi

fail=0
outputs=""

for file in "$@"; do
  # only check target directories (pre-commit already filters, double check)
  if [[ ! "$file" =~ ^(src|include)/.*\.(c|h)$ ]]; then
    continue
  fi

  # per-file skip marker: any file containing NORM_SKIP will be ignored
  if grep -q -m1 -F "NORM_SKIP" "$file" 2>/dev/null; then
    echo "[check_norm] skipping $file (contains NORM_SKIP)"
    continue
  fi

  # run norminette and capture output
  out="$(norminette "$file" 2>&1)" || rc=$?; rc=${rc:-0}
  if [ "$rc" -ne 0 ]; then
    fail=1
    outputs="$outputs
===== norminette failure: $file =====
$out
"
  fi
done

if [ "$fail" -ne 0 ]; then
  printf "%s\n" "$outputs"
  echo "norminette checks failed. Add NORM_SKIP to draft files or fix style." >&2
  echo "To bypass temporarily: SKIP_NORM=1 git commit -m '...'  OR  git commit --no-verify" >&2
  exit 1
fi

exit 0