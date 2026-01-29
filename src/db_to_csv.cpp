#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include <spdlog/spdlog.h>

ABSL_FLAG(std::string, input, "", "Input file");
ABSL_FLAG(std::string, output, "", "Output file");

int main(int argc, char** argv) {
    absl::ParseCommandLine(argc, argv);

    std::string input = absl::GetFlag(FLAGS_input);
    std::string output = absl::GetFlag(FLAGS_output);

    if (input.empty()) {
        std::cerr << "--input <input-db-file> is required" << std::endl;
        return 1;
    }

    if (output.empty()) {
        std::cerr << "--output <output-csv-file> is required" << std::endl;
        return 1;
    }
}