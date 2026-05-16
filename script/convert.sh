#!/bin/bash
set -e
INPUT_CSV="$1"
COLUMNAR="$2"
./build/bin/csv_to_columnar --input "$INPUT_CSV" --output "$COLUMNAR" --schema ./script/hits_schema.csv --delimiter $'\t'
