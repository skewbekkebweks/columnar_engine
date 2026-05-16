./bin/csv_to_columnar --input ../data/hits_sample.csv --output ../data/hits_sample_test.skewdb --schema ../data/hits_schema.csv

./bin/columnar_to_csv --input ../data/hits_sample_test.skewdb --output ../data/hits_sample_test.csv

./bin/run_queries ../data/hits_sample.skewdb all