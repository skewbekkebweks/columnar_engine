#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include <spdlog/spdlog.h>

ABSL_FLAG(std::string, input, "", "Input file");
ABSL_FLAG(std::string, output, "", "Output file");
ABSL_FLAG(std::string, schema, "", "Schema file");

int main(int argc, char** argv) {
    absl::ParseCommandLine(argc, argv);

    std::string input = absl::GetFlag(FLAGS_input);
    std::string output = absl::GetFlag(FLAGS_output);
    std::string schema = absl::GetFlag(FLAGS_schema);

    if (input.empty()) {
        std::cerr << "--input <input-csv-file> is required" << std::endl;
        return 1;
    }

    if (output.empty()) {
        std::cerr << "--output <output-db-file> is required" << std::endl;
        return 1;
    }

    if (schema.empty()) {
        std::cerr << "--schema <schema-csv-file> is required" << std::endl;
        return 1;
    }
}