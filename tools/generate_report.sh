#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR=$(cd "$(dirname "$0")/.." && pwd)
JSON_FILE="$ROOT_DIR/metrics.json"
TEMPLATE="$ROOT_DIR/tools/metrics_template.json"

if [ ! -f "$JSON_FILE" ]; then
  cp "$TEMPLATE" "$JSON_FILE"
  echo "Created metrics.json from template. Fill it with your measurements."
fi

echo "=== Metrics Report ==="
if command -v jq >/dev/null 2>&1; then
  jq '.' "$JSON_FILE"
elif command -v python3 >/dev/null 2>&1; then
  python3 -m json.tool "$JSON_FILE"
elif command -v python >/dev/null 2>&1; then
  python -m json.tool "$JSON_FILE"
else
  echo "(no jq/python) showing raw metrics.json"
  cat "$JSON_FILE"
fi

echo "\nTips:"
echo "- Run: make run-tests and make run-bench to gather measurements"
echo "- Record p50/p99 and environment info (CPU, kernel)"
echo "- Edit metrics.json with your measured values and re-run: make report"
