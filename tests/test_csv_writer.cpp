#include <gtest/gtest.h>

#include "csv_writer.h"

TEST(CsvWriter, BasicTest) {
    std::vector<std::string> content = {"abc", "def", "ghi"};
    
    std::string path = "/tmp/a.txt";

    {
        CsvWriter writer{path};
        writer.WriteRow(content);
    }

    std::ifstream input(path);
    std::string res;
    std::getline(input, res);
    EXPECT_EQ(res, "abc,def,ghi");
}

TEST(CsvWriter, ChangeDelimeter) {
    std::vector<std::string> content = {"abc", "def", "ghi"};
    
    std::string path = "/tmp/a.txt";

    {
        CsvConfig config{';', '"'};
        CsvWriter writer{path, config};
        writer.WriteRow(content);
    }

    std::ifstream input(path);
    std::string res;
    std::getline(input, res);
    EXPECT_EQ(res, "abc;def;ghi");
}

TEST(CsvWriter, ChangeQuoteSymbol) {
    std::vector<std::string> content = {"abc", "d,ef", "ghi"};
    
    std::string path = "/tmp/a.txt";

    {
        CsvConfig config{',', '\''};
        CsvWriter writer{path, config};
        writer.WriteRow(content);
    }

    std::ifstream input(path);
    std::string res;
    std::getline(input, res);
    EXPECT_EQ(res, "abc,'d,ef',ghi");
}

TEST(CsvWriter, LinebreakInside) {
    std::vector<std::string> content = {"abc", "d\nef", "ghi"};
    
    std::string path = "/tmp/a.txt";

    {
        CsvWriter writer{path};
        writer.WriteRow(content);
    }

    std::ifstream input(path);
    std::string res, line;
    while (std::getline(input, line)) {
        res += line + '\n';
    }
    res.pop_back();
    EXPECT_EQ(res, "abc,\"d\nef\",ghi");
}

TEST(CsvWriter, QuoteInside) {
    std::vector<std::string> content = {"abc", "d\"ef", "ghi"};
    
    std::string path = "/tmp/a.txt";

    {
        CsvWriter writer{path};
        writer.WriteRow(content);
    }

    std::ifstream input(path);
    std::string res;
    std::getline(input, res);
    EXPECT_EQ(res, "abc,\"d\"\"ef\",ghi");
}