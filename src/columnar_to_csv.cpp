#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include <spdlog/spdlog.h>

#include "columnar_reader.h"
#include "csv_writer.h"

ABSL_FLAG(std::string, input, "", "Input file");
ABSL_FLAG(std::string, output, "", "Output file");

int main(int argc, char** argv) {
    spdlog::set_level(spdlog::level::level_enum::warn);

    absl::ParseCommandLine(argc, argv);

    std::string input_file = absl::GetFlag(FLAGS_input);
    std::string output_file = absl::GetFlag(FLAGS_output);

    if (input_file.empty()) {
        std::cerr << "--input <input-db-file> is required" << std::endl;
        return 1;
    }
    if (output_file.empty()) {
        std::cerr << "--output <output-csv-file> is required" << std::endl;
        return 1;
    }

    ColumnarReader input_reader{input_file};
    CsvWriter output_writer{output_file};

    while (auto row_group_opt = input_reader.ReadRowGroup()) {
        auto row_group = std::move(row_group_opt.value());
        size_t coulumns_count = row_group.size();
        for (size_t i = 0; i < row_group[0]->Size(); ++i) {
            spdlog::debug("row number " + std::to_string(i));
            std::vector<std::string> row;
            row.reserve(coulumns_count);

            for (size_t j = 0; j < coulumns_count; ++j) {
                row.push_back(row_group[j]->operator[](i));
            }

            output_writer.WriteRow(row);
        }
    }
}