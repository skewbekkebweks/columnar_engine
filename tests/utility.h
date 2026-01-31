#include <string>
#include <fstream>

inline std::string CreateTempFile(const std::string& content, const std::string& filename = "a.txt") {
    std::string path = "/tmp/" + filename;
    std::ofstream out(path);
    out << content;
    out.close();
    return path;
}