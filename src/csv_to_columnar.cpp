#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include <spdlog/spdlog.h>

#include "schema.h"
#include "column.h"
#include "csv_reader.h"
#include "columnar_writer.h"

static constexpr size_t kRowGroupSize = 100;

ABSL_FLAG(std::string, input, "", "Input file");
ABSL_FLAG(std::string, output, "", "Output file");
ABSL_FLAG(std::string, schema, "", "Schema file");

int main(int argc, char** argv) {
    spdlog::set_level(spdlog::level::level_enum::warn);

    absl::ParseCommandLine(argc, argv);

    std::string input_file = absl::GetFlag(FLAGS_input);
    std::string output_file = absl::GetFlag(FLAGS_output);
    std::string schema_file = absl::GetFlag(FLAGS_schema);

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

    Schema schema = Schema::FromCsv(schema_file);

    CsvReader input_reader{input_file};
    ColumnarWriter output_writer{output_file + ".skewdb", schema};

    while (true) {
        std::vector<std::unique_ptr<Column>> cur_row_group;
        cur_row_group.reserve(schema.Size());

        for (auto column : schema.GetColumns()) {
            switch (column.type) {
            case Type::Int64: {
                cur_row_group.push_back(std::make_unique<ColumnInt64>());
                break;
            }
            case Type::String: {
                cur_row_group.push_back(std::make_unique<ColumnString>());
                break;
            }
            default:
                spdlog::error("Unreachable code reached in CsvToColumnar");
                throw std::runtime_error{"Unreachable code reached in CsvToColumnar"};
            }
        }

        size_t cur_row_group_size = 0;

        while (auto row_opt = input_reader.ReadRow()) {
            std::vector<std::string> row = std::move(row_opt.value());
            if (row.size() != schema.Size()) {
                throw std::runtime_error{"Csv file has " + std::to_string(row.size()) + " columns, but schema has " + std::to_string(schema.Size())};
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

    output_writer.Finalize();
}   