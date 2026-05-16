#!/bin/bash
QUERY_NUM="$1"
COLUMNAR="$2"
OUTPUT_CSV="$3"
LOG_FILE="$4"
./build/bin/run_queries "$COLUMNAR" $((QUERY_NUM + 1)) > "$OUTPUT_CSV" 2>> "$LOG_FILE"
