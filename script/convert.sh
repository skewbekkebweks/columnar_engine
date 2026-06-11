#!/bin/bash
set -e
INPUT_CSV="$1"
COLUMNAR="$2"
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
"$ROOT/build/bin/csv_to_columnar" --input "$INPUT_CSV" --output "$COLUMNAR" --schema "$ROOT/script/hits_schema.csv"
