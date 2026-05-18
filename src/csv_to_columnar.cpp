#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include <spdlog/cfg/env.h>
#include <spdlog/spdlog.h>
#include <chrono>

#include "error.h"
#include "schema.h"
#include "column.h"
#include "csv_reader.h"
#include "columnar_writer.h"

static constexpr size_t kRowGroupSize = 50000;

ABSL_FLAG(std::string, input, "", "Input file");
ABSL_FLAG(std::string, output, "", "Output file");
ABSL_FLAG(std::string, schema, "", "Schema file");
ABSL_FLAG(std::string, delimiter, ",", "Field delimiter character");

int main(int argc, char** argv) {
    spdlog::cfg::load_env_levels();

    absl::ParseCommandLine(argc, argv);

    std::string input_file = absl::GetFlag(FLAGS_input);
    std::string output_file = absl::GetFlag(FLAGS_output);
    std::string schema_file = absl::GetFlag(FLAGS_schema);
    std::string delimiter_str = absl::GetFlag(FLAGS_delimiter);
    char delimiter = delimiter_str.empty() ? ',' : delimiter_str[0];

    if (input_file.empty()) {
        std::cerr << "--input <input-csv-file> is required" << std::endl;
        return 1;
    }
    if (output_file.empty()) {
        std::cerr << "--output <output-db-file> is required" << std::endl;
        return 1;
    }
    if (schema_file.empty()) {
        std::cerr << "--schema <schema-csv-file> is required" << std::endl;
        return 1;
    }

    auto start_time = std::chrono::steady_clock::now();

    Schema schema = Schema::FromCsv(schema_file);

    CsvReader input_reader{input_file, CsvConfig{delimiter}};
    ColumnarWriter output_writer{output_file, schema};

    while (true) {
        std::vector<std::unique_ptr<Column>> cur_row_group;
        cur_row_group.reserve(schema.Size());

        for (auto column : schema.GetColumns()) {
            cur_row_group.push_back(MakeColumn(column.type));
        }

        size_t cur_row_group_size = 0;

        while (auto row_opt = input_reader.ReadRow()) {
            std::vector<std::string> row = std::move(row_opt.value());
            if (row.size() != schema.Size()) {
                THROW_RUNTIME_ERROR("Csv file has " + std::to_string(row.size()) +
                                    " columns, but schema has " + std::to_string(schema.Size()));
            }

            for (size_t i = 0; i < row.size(); ++i) {
                cur_row_group[i]->PushBack(row[i]);
            }

            cur_row_group_size++;
            if (cur_row_group_size == kRowGroupSize) {
                break;
            }
        }

        if (cur_row_group_size == 0) {
            break;
        }

        output_writer.AddRowGroup(cur_row_group);
    }

    std::move(output_writer).Finalize();

    auto end_time = std::chrono::steady_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

    std::cout << "Processing time: " << static_cast<double>(duration) / 1000 << "s" << '\n';
}