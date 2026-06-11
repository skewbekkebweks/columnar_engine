#!/bin/bash
QUERY_NUM="$1"
COLUMNAR="$2"
OUTPUT_CSV="$3"
LOG_FILE="$4"
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
"$ROOT/build/bin/run_queries" "$COLUMNAR" "$QUERY_NUM" > "$OUTPUT_CSV" 2>> "$LOG_FILE"
