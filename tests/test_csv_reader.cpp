#include <gtest/gtest.h>

#include "csv_reader.h"
#include "../tests/utility.h"


TEST(CsvReader, BasicTest) {
    std::string content =
        "a,b,c,d\n"
        "1,2,3,4\n"
        "а,б,в,г\n"
        ;
    std::string path = CreateTempFile(content);
    CsvReader reader{path};

    std::vector<std::string> row;

    for (size_t i = 0; i < 3; ++i) {
        row = reader.ReadRow().value();
        EXPECT_EQ(row.size(), 4);
    }

    EXPECT_FALSE(reader.ReadRow().has_value());
}

TEST(CsvReader, WrappedByQuotes) {
    std::string content =
        "\"a\",b,\"c\",d\n"
        ;
    std::string path = CreateTempFile(content);
    CsvReader reader{path};

    std::vector<std::string> row = reader.ReadRow().value();
    EXPECT_EQ(row.size(), 4);
    EXPECT_EQ(row[0], "a");
    EXPECT_EQ(row[1], "b");
    EXPECT_EQ(row[2], "c");
    EXPECT_EQ(row[3], "d");
}

TEST(CsvReader, LinebreakInside) {
    std::string content =
        "\"a\nbc\",def\n"
        ;
    std::string path = CreateTempFile(content);
    CsvReader reader{path};

    std::vector<std::string> row = reader.ReadRow().value();
    EXPECT_EQ(row.size(), 2);
    EXPECT_EQ(row[0], "a\nbc");
    EXPECT_EQ(row[1], "def");
}

TEST(CsvReader, QuotesInside) {
    std::string content =
        "\"\"\"abc\"\"\",\"def\",ghi"
        ;
    std::string path = CreateTempFile(content);
    CsvReader reader{path};

    std::vector<std::string> row = reader.ReadRow().value();
    EXPECT_EQ(row.size(), 3);
    EXPECT_EQ(row[0], "\"abc\"");
    EXPECT_EQ(row[1], "def");
    EXPECT_EQ(row[2], "ghi");
}

TEST(CsvReader, DifferentRowSize) {
    std::string content =
        "a,b,c,d\n"
        "a,b,c\n"
        "a,b\n"
        "a"
        ;
    std::string path = CreateTempFile(content);
    CsvReader reader{path};

    for (int i = 4; i > 0; --i) {
        std::vector<std::string> row = reader.ReadRow().value();
        EXPECT_EQ(row.size(), i);
        for (int j = 0; j < i; ++j) {
            EXPECT_EQ(row[j][0], 'a' + j);
        }
    }
}

TEST(CsvReader, ChangeDelimiter) {
    std::string content =
        "a;b;c;d\n"
        "a,b;c,d"
        ;
    std::string path = CreateTempFile(content);
    CsvConfig config{';', '"'};
    CsvReader reader{path, config};

    std::vector<std::string> row = reader.ReadRow().value();
    EXPECT_EQ(row.size(), 4);
    row = reader.ReadRow().value();
    EXPECT_EQ(row.size(), 2);
}

TEST(CsvReader, ChangeQuoteSymbol) {
    std::string content =
        "\"a,b\",c,d\n"
        "'a,b',c,d"
        ;
    std::string path = CreateTempFile(content);
    CsvConfig config{',', '\''};
    CsvReader reader{path, config};

    std::vector<std::string> row = reader.ReadRow().value();
    EXPECT_EQ(row.size(), 4);
    row = reader.ReadRow().value();
    EXPECT_EQ(row.size(), 3);
}