#include "text_tests.hpp"
#include "testtool.hpp"
#include <texttools.hpp>

void test_icu_regex_group() {
    DECLARE_TEST(test_parse_command)

    std::string s;
    std::unique_ptr<RegexPattern> re;
    std::vector<std::string> groups;
    bool res;

    REPORT_CASE
    s = "+CUSD: 1,\"0031\", 72";
    re = tools::g_unicode_ts.regex_pattern("\\+CUSD:\\s?(\\d+)\\s?,\\s?\\\"([^\\\"]*)\\\"\\s?,\\s?(\\d+)", 0);
    assert(re);
    groups.clear();
    res = tools::g_unicode_ts.regex_groups(*re, s, groups);
    assert(res);
    assert(groups.size()==4);
    assert(groups[0].compare(s)==0);
    assert(groups[1].compare("1")==0);
    assert(groups[2].compare("0031")==0);
    assert(groups[3].compare("72")==0);

    REPORT_CASE
    s = "+CMGL: 59,\"REC READ\",\"002B\",\"\",\"20/08/06,16:29:57+12\"";
    re = tools::g_unicode_ts.regex_pattern("\\+CMGL:\\s*(\\d+)\\s*,\\s*\\\"([^\\\"\\d]+)\\\"\\s*,\\s*\\\"([a-fA-F\\d]+)\\\"\\s*,\\s*\\\"([^\\\"]*)\\\"\\s*,\\s*\\\"(\\S+)\\\"", 0);
    assert(re);
    groups.clear();
    res = tools::g_unicode_ts.regex_groups(*re, s, groups);
    assert(res);
    assert(groups.size()==6);
    assert(groups[0].compare(s)==0);
    assert(groups[1].compare("59")==0);
    assert(groups[2].compare("REC READ")==0);
    assert(groups[3].compare("002B")==0);
    assert(groups[4].compare("")==0);
    assert(groups[5].compare("20/08/06,16:29:57+12")==0);
}
void test_check_prefix() {
    DECLARE_TEST(test_check_prefix)
    bool result;

    REPORT_CASE
    result = tools::check_prefix((const char*)"",(const char*)"");
    assert(result==true);

    REPORT_CASE
    result = tools::check_prefix("","A");
    assert(result==false);

    REPORT_CASE
    result = tools::check_prefix("","AA");
    assert(result==false);

    REPORT_CASE
    result = tools::check_prefix("A","");
    assert(result==true);

    REPORT_CASE
    result = tools::check_prefix("A","A");
    assert(result==true);

    REPORT_CASE
    result = tools::check_prefix("AA","A");
    assert(result==true);

    REPORT_CASE
    result = tools::check_prefix("C","CC");
    assert(result==false);

    REPORT_CASE
    result = tools::check_prefix("CC","CC");
    assert(result==true);

    REPORT_CASE
    result = tools::check_prefix("CCC","CC");
    assert(result==true);
}
void test_hex_val() {
    DECLARE_TEST(test_hex_val)
    REPORT_CASE

    for (size_t i=0; i<256; ++i) {
        char c = (char) (i & 0xFF);
        uint8_t res, exp;

        res = tools::SpecialCharacterTables::hex_val[i];

        if (c>='0' && c<='9') {
            exp = c - '0';
        } else if (c>='A' && c<='F') {
            exp = 0x0A + (c - 'A');
        } else if (c>='a' && c<='f') {
            exp = 0x0A + (c - 'a');
        } else {
            exp = 255;
        }

        assert(exp==res);
    }
}
void test_buffer_to_hex() {
    DECLARE_TEST(test_hex_val)

    REPORT_CASE
    std::vector<uint8_t> buffer = {0x00};
    std::string res = tools::buffer_to_hex(buffer.data(), buffer.size(), true, nullptr);
    assert(res.compare("00")==0);

    REPORT_CASE
    buffer = {};
    res = tools::buffer_to_hex(nullptr,  buffer.size(), true, nullptr);
    assert(res.compare("")==0);

    REPORT_CASE
    buffer = {0x01, 0x02};
    res = tools::buffer_to_hex(buffer.data(), buffer.size(), true, nullptr);
    assert(res.compare("0102")==0);

    REPORT_CASE
    buffer = {0x01, 0x02};
    res = tools::buffer_to_hex(buffer.data(), buffer.size(), true, "");
    assert(res.compare("0102")==0);

    REPORT_CASE
    buffer = {0x01, 0x02};
    res = tools::buffer_to_hex(buffer.data(), buffer.size(), true, "->");
    assert(res.compare("01->02")==0);

    REPORT_CASE
    buffer = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};
    res = tools::buffer_to_hex(buffer.data(), buffer.size(), false, " ");
    assert(res.compare("01 23 45 67 89 AB CD EF")==0);

    REPORT_CASE
    buffer = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};
    res = tools::buffer_to_hex(buffer.data(), buffer.size(), true, " ");
    assert(res.compare("01 23 45 67 89 ab cd ef")==0);
}
void test_buffer_from_hex() {
    DECLARE_TEST(test_buffer_from_hex)

    REPORT_CASE
    std::string hex = "";
    std::vector<uint8_t> exp;
    std::vector<uint8_t> res = tools::buffer_from_hex(hex);
    assert(std::equal(res.begin(), res.end(), exp.begin()));

    REPORT_CASE
    hex = "000102";
    exp = {0x00, 0x01, 0x02};
    res = tools::buffer_from_hex(hex);
    assert(std::equal(res.begin(), res.end(), exp.begin()));

    REPORT_CASE
    hex = "00";
    exp = {0x00};
    res = tools::buffer_from_hex(hex);
    assert(std::equal(res.begin(), res.end(), exp.begin()));

    REPORT_CASE
    hex = "0123456789ABCDEF";
    exp = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};
    res = tools::buffer_from_hex(hex);
    assert(std::equal(res.begin(), res.end(), exp.begin()));

    REPORT_CASE
    hex = "0123456789abcdef";
    exp = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};
    res = tools::buffer_from_hex(hex);
    assert(std::equal(res.begin(), res.end(), exp.begin()));

    REPORT_CASE
    hex = "0123456789abCDeF";
    exp = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};
    res = tools::buffer_from_hex(hex);
    assert(std::equal(res.begin(), res.end(), exp.begin()));

    REPORT_CASE
    try {
        hex = "01234567P9abCDeF";
        exp = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};
        res = tools::buffer_from_hex(hex);
        assert(false);
    } catch (std::out_of_range& e) {
    }

    REPORT_CASE
    try {
        hex = "01234567P9abCDe";
        exp = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};
        res = tools::buffer_from_hex(hex);
        assert(std::equal(exp.begin(), exp.end(), res.begin()));
    } catch (std::length_error& e) {
    }
}
void test_split_and_trim() {
    DECLARE_TEST(split_and_trim)
    std::vector<std::string> lines;
    std::string s;

    REPORT_CASE
    s = "";
    lines = tools::split_and_trim(s, [](char b){return b=='\n';}, [](char b){return b==' ';});
    assert(lines.size()==0);

    REPORT_CASE
    s = "1";
    lines = tools::split_and_trim(s, [](char b){return b=='\n';}, [](char b){return b==' ';});
    assert(lines.size()==1);
    assert(lines[0].compare("1")==0);

    REPORT_CASE
    s = "11";
    lines = tools::split_and_trim(s, [](char b){return b=='\n';}, [](char b){return b==' ';});
    assert(lines.size()==1);
    assert(lines[0].compare("11")==0);

    REPORT_CASE
    s = "11\n";
    lines = tools::split_and_trim(s, [](char b){return b=='\n';}, [](char b){return b==' ';});
    assert(lines.size()==1);
    assert(lines[0].compare("11")==0);

    REPORT_CASE
    s = "\n11\n";
    lines = tools::split_and_trim(s, [](char b){return b=='\n';}, [](char b){return b==' ';});
    assert(lines.size()==1);
    assert(lines[0].compare("11")==0);

    REPORT_CASE
    s = "\n11\n\n";
    lines = tools::split_and_trim(s, [](char b){return b=='\n';}, [](char b){return b==' ';});
    assert(lines.size()==1);
    assert(lines[0].compare("11")==0);

    REPORT_CASE
    s = "\n\n11\n\n";
    lines = tools::split_and_trim(s, [](char b){return b=='\n';}, [](char b){return b==' ';});
    assert(lines.size()==1);
    assert(lines[0].compare("11")==0);

    REPORT_CASE
    s = "\n\n1 1\n\n";
    lines = tools::split_and_trim(s, [](char b){return b=='\n';}, [](char b){return b==' ';});
    assert(lines.size()==1);
    assert(lines[0].compare("1 1")==0);

    REPORT_CASE
    s = "\n\n11 \n\n";
    lines = tools::split_and_trim(s, [](char b){return b=='\n';}, [](char b){return b==' ';});
    assert(lines.size()==1);
    assert(lines[0].compare("11")==0);

    REPORT_CASE
    s = "\n\n11 \n\n1";
    lines = tools::split_and_trim(s, [](char b){return b=='\n';}, [](char b){return b==' ';});
    assert(lines.size()==2);
    assert(lines[0].compare("11")==0);
    assert(lines[1].compare("1")==0);

    REPORT_CASE
    s = "\n\n11 \n\n1 ";
    lines = tools::split_and_trim(s, [](char b){return b=='\n';}, [](char b){return b==' ';});
    assert(lines.size()==2);
    assert(lines[0].compare("11")==0);
    assert(lines[1].compare("1")==0);

    REPORT_CASE
    s = "\n\n11 \n\n 1 ";
    lines = tools::split_and_trim(s, [](char b){return b=='\n';}, [](char b){return b==' ';});
    assert(lines.size()==2);
    assert(lines[0].compare("11")==0);
    assert(lines[1].compare("1")==0);

    REPORT_CASE
    s = "\n\n 11 \n\n 1 ";
    lines = tools::split_and_trim(s, [](char b){return b=='\n';}, [](char b){return b==' ';});
    assert(lines.size()==2);
    assert(lines[0].compare("11")==0);
    assert(lines[1].compare("1")==0);

    REPORT_CASE
    s = "\n\n 11 \n\n    ";
    lines = tools::split_and_trim(s, [](char b){return b=='\n';}, [](char b){return b==' ';});
    assert(lines.size()==1);
    assert(lines[0].compare("11")==0);

    REPORT_CASE
    s = " \n\n  \n \n    \n";
    lines = tools::split_and_trim(s, [](char b){return b=='\n';}, [](char b){return b==' ';});
    assert(lines.size()==0);

    REPORT_CASE
    s = " \n\n  \n \n    \n\n";
    lines = tools::split_and_trim(s, [](char b){return b=='\n';}, [](char b){return b==' ';});
    assert(lines.size()==0);

    REPORT_CASE
    s = "\n \n\n  \n \n    \n";
    lines = tools::split_and_trim(s, [](char b){return b=='\n';}, [](char b){return b==' ';});
    assert(lines.size()==0);
}
void test_trim_string() {
    DECLARE_TEST(test_trim_string)
    size_t res = 0;
    std::string s;

    REPORT_CASE
    s = "";
    res = tools::trim_string(s, [](char b){return false;});
    assert(res==0);
    assert(s.empty());

    REPORT_CASE
    s = "";
    res = tools::trim_string(s, [](char b){return b==' ';});
    assert(res==0);
    assert(s.empty());

    REPORT_CASE
    s = " ";
    res = tools::trim_string(s, [](char b){return b==' ';});
    assert(res==0);
    assert(s.empty());

    REPORT_CASE
    s = " ";
    res = tools::trim_string(s, [](char b){return b==' ';});
    assert(res==0);
    assert(s.empty());

    REPORT_CASE
    s = "  ";
    res = tools::trim_string(s, [](char b){return b==' ';});
    assert(res==0);
    assert(s.empty());

    REPORT_CASE
    s = "      ";
    res = tools::trim_string(s, [](char b){return b==' ';});
    assert(res==0);
    assert(s.empty());

    REPORT_CASE
    s = "1";
    res = tools::trim_string(s, [](char b){return b==' ';});
    assert(res==1);
    assert(s.compare("1")==0);

    REPORT_CASE
    s = "1 ";
    res = tools::trim_string(s, [](char b){return b==' ';});
    assert(res==1);
    assert(s.compare("1")==0);

    REPORT_CASE
    s = " 1";
    res = tools::trim_string(s, [](char b){return b==' ';});
    assert(res==1);
    assert(s.compare("1")==0);

    REPORT_CASE
    s = " 1 ";
    res = tools::trim_string(s, [](char b){return b==' ';});
    assert(res==1);
    assert(s.compare("1")==0);

    REPORT_CASE
    s = "12";
    res = tools::trim_string(s, [](char b){return b==' ';});
    assert(res==2);
    assert(s.compare("12")==0);

    REPORT_CASE
    s = " 12";
    res = tools::trim_string(s, [](char b){return b==' ';});
    assert(res==2);
    assert(s.compare("12")==0);

    REPORT_CASE
    s = "  12";
    res = tools::trim_string(s, [](char b){return b==' ';});
    assert(res==2);
    assert(s.compare("12")==0);

    REPORT_CASE
    s = "12 ";
    res = tools::trim_string(s, [](char b){return b==' ';});
    assert(res==2);
    assert(s.compare("12")==0);

    REPORT_CASE
    s = "12  ";
    res = tools::trim_string(s, [](char b){return b==' ';});
    assert(res==2);
    assert(s.compare("12")==0);

    REPORT_CASE
    s = " 12  ";
    res = tools::trim_string(s, [](char b){return b==' ';});
    assert(res==2);
    assert(s.compare("12")==0);

    REPORT_CASE
    s = "  12  ";
    res = tools::trim_string(s, [](char b){return b==' ';});
    assert(res==2);
    assert(s.compare("12")==0);

    REPORT_CASE
    s = "  12  ";
    res = tools::trim_string(s, [](char b){return std::isspace(b);});
    assert(res==2);
    assert(s.compare("12")==0);
}