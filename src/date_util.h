#pragma once

#include <cstdint>
#include <cstdio>
#include <string>

inline int64_t DaysFromCivil(int y, unsigned m, unsigned d) {
    y -= m <= 2;
    const int era = (y >= 0 ? y : y - 399) / 400;
    const unsigned yoe = static_cast<unsigned>(y - era * 400);
    const unsigned doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
    const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return static_cast<int64_t>(era) * 146097 + static_cast<int>(doe) - 719468;
}

inline void CivilFromDays(int64_t z, int& y, unsigned& m, unsigned& d) {
    z += 719468;
    const int64_t era = (z >= 0 ? z : z - 146096) / 146097;
    const unsigned doe = static_cast<unsigned>(z - era * 146097);
    const unsigned yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    y = static_cast<int>(yoe) + static_cast<int>(era * 400);
    const unsigned doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    const unsigned mp = (5 * doy + 2) / 153;
    d = doy - (153 * mp + 2) / 5 + 1;
    m = mp + (mp < 10 ? 3 : -9);
    y += (m <= 2);
}

namespace date_detail {
inline unsigned Digits2(const char* p) {
    return static_cast<unsigned>((p[0] - '0') * 10 + (p[1] - '0'));
}
}  // namespace date_detail

inline int64_t ParseDate(const std::string& s) {
    if (s.size() < 10) {
        return 0;
    }
    const char* p = s.data();
    int y = (p[0] - '0') * 1000 + (p[1] - '0') * 100 + (p[2] - '0') * 10 + (p[3] - '0');
    unsigned mo = date_detail::Digits2(p + 5);
    unsigned d = date_detail::Digits2(p + 8);
    return DaysFromCivil(y, mo, d);
}

inline std::string FormatDate(int64_t days) {
    int y;
    unsigned m, d;
    CivilFromDays(days, y, m, d);
    char buf[16];
    std::snprintf(buf, sizeof(buf), "%04d-%02u-%02u", y, m, d);
    return buf;
}

inline int64_t ParseTimestamp(const std::string& s) {
    int64_t days = ParseDate(s);
    if (s.size() < 19) {
        return days * 86400;
    }
    const char* p = s.data();
    unsigned hh = date_detail::Digits2(p + 11);
    unsigned mm = date_detail::Digits2(p + 14);
    unsigned ss = date_detail::Digits2(p + 17);
    return days * 86400 + hh * 3600 + mm * 60 + ss;
}

inline std::string FormatTimestamp(int64_t secs) {
    int64_t days = secs / 86400;
    int64_t rem = secs % 86400;
    if (rem < 0) {
        rem += 86400;
        days -= 1;
    }
    int y;
    unsigned m, d;
    CivilFromDays(days, y, m, d);
    unsigned hh = static_cast<unsigned>(rem / 3600);
    unsigned mm = static_cast<unsigned>((rem % 3600) / 60);
    unsigned ss = static_cast<unsigned>(rem % 60);
    char buf[24];
    std::snprintf(buf, sizeof(buf), "%04d-%02u-%02u %02u:%02u:%02u", y, m, d, hh, mm, ss);
    return buf;
}
