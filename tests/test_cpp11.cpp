#include "gtest/gtest.h"
#include <utf8.h>
#include <string>
using namespace utf8;
using namespace std;

#if __cplusplus >= 201103L // C++ 11 or later

static void swap_u16_raw_buffer(vector<uint8_t>& buffer);

TEST(CPP11APITests, test_append)
{
    string u;
    append(0x0448, u);
    EXPECT_EQ (u[0], char(0xd1));
    EXPECT_EQ (u[1], char(0x88));
    EXPECT_EQ (u.length(), 2);

    u.clear();
    append(0x65e5, u);
    EXPECT_EQ (u[0], char(0xe6));
    EXPECT_EQ (u[1], char(0x97));
    EXPECT_EQ (u[2], char(0xa5));
    EXPECT_EQ (u.length(), 3);

    u.clear();
    append(0x3044, u);
    EXPECT_EQ (u[0], char(0xe3));
    EXPECT_EQ (u[1], char(0x81));
    EXPECT_EQ (u[2], char(0x84));
    EXPECT_EQ (u.length(), 3);

    u.clear();
    append(0x10346, u);
    EXPECT_EQ (u[0], char(0xf0));
    EXPECT_EQ (u[1], char(0x90));
    EXPECT_EQ (u[2], char(0x8d));
    EXPECT_EQ (u[3], char(0x86));
    EXPECT_EQ (u.length(), 4);
}

TEST(CPP11APITests, test_utf16to8)
{
    u16string utf16string = {0x41, 0x0448, 0x65e5, 0xd834, 0xdd1e};
    string u = utf16to8(utf16string);
    EXPECT_EQ (u.size(), 10);
}

TEST(CPP11APITests, test_utf8to16)
{
    string utf8_with_surrogates = "\xe6\x97\xa5\xd1\x88\xf0\x9d\x84\x9e";
    u16string utf16result = utf8to16(utf8_with_surrogates);
    EXPECT_EQ (utf16result.size(), 4);
    EXPECT_EQ (utf16result[2], 0xd834);
    EXPECT_EQ (utf16result[3], 0xdd1e);
}

TEST(CPP11APITests, test_utf32to8)
{
    u32string utf32string = {0x448, 0x65E5, 0x10346};
    string utf8result = utf32to8(utf32string);
    EXPECT_EQ (utf8result.size(), 9);
}

TEST(CPP11APITests, test_utf8to32)
{
    const char* twochars = "\xe6\x97\xa5\xd1\x88";
    u32string utf32result = utf8to32(twochars);
    EXPECT_EQ (utf32result.size(), 2);
}

TEST(CPP11APITests, test_find_invalid)
{
    string utf_invalid = "\xe6\x97\xa5\xd1\x88\xfa";
    auto invalid = find_invalid(utf_invalid);
    EXPECT_EQ (invalid, 5);
}

TEST(CPP11APITests, test_is_valid)
{
    string utf_invalid = "\xe6\x97\xa5\xd1\x88\xfa";
    bool bvalid = is_valid(utf_invalid);
    EXPECT_FALSE (bvalid);
    string utf8_with_surrogates = "\xe6\x97\xa5\xd1\x88\xf0\x9d\x84\x9e";
    bvalid = is_valid(utf8_with_surrogates);
    EXPECT_TRUE (bvalid);
}

TEST(CPP11APITests, test_replace_invalid)
{
    string invalid_sequence = "a\x80\xe0\xa0\xc0\xaf\xed\xa0\x80z";
    string replace_invalid_result = replace_invalid(invalid_sequence, '?');
    bool bvalid = is_valid(replace_invalid_result);
    EXPECT_TRUE (bvalid);
    const string fixed_invalid_sequence = "a????z";
    EXPECT_EQ(fixed_invalid_sequence, replace_invalid_result);
}

TEST(CPP11APITests, test_starts_with_bom)
{
    string byte_order_mark = {char(0xef), char(0xbb), char(0xbf)};
    bool bbom = starts_with_bom(byte_order_mark);
    EXPECT_TRUE (bbom);
    string threechars = "\xf0\x90\x8d\x86\xe6\x97\xa5\xd1\x88";
    bool no_bbom = starts_with_bom(threechars);
    EXPECT_FALSE (no_bbom);
}

static void test_utf16to8_endianess_hint(const vector<uint8_t>& u16_src_raw, endianess hint, const string& u8_ref)
{
    u16string u16_src((const char16_t*)u16_src_raw.data(), u16_src_raw.size() / 2);
    string u8_conv = utf16to8(u16_src, hint);
    EXPECT_EQ(u8_conv, u8_ref);

    u8_conv.clear();
    unchecked::utf16to8(hint, u16_src.begin(), u16_src.end(), std::back_inserter(u8_conv));
    EXPECT_EQ(u8_conv, u8_ref);

    u8_conv.clear();
    unchecked::utf16to8(!internal::is_byte_swap_required(hint), u16_src.begin(), u16_src.end(), std::back_inserter(u8_conv));
    EXPECT_NE(u8_conv, u8_ref);
}

TEST(CPP11APITests, test_utf16to8_endianess_hint)
{
    // https://r12a.github.io/app-conversion/?q=%F0%A0%94%80%F0%A0%81%85%F0%A0%81%BAad%F0%A0%81%BC
    string u8_ref = "\xF0\xA0\x94\x80\xF0\xA0\x81\x85\xF0\xA0\x81\xBA\x61\x64\xF0\xA0\x81\xBC";
    vector<uint8_t> u16_src_raw = { 0xD8, 0x41, 0xDD, 0x00, 0xD8, 0x40, 0xDC, 0x45, 0xD8, 0x40,
                                           0xDC, 0x7A, 0x00, 0x61, 0x00, 0x64, 0xD8, 0x40, 0xDC, 0x7C };

    test_utf16to8_endianess_hint(u16_src_raw, endianess::big_endian, u8_ref);
    swap_u16_raw_buffer(u16_src_raw);
    test_utf16to8_endianess_hint(u16_src_raw, endianess::little_endian, u8_ref);
}

static void test_utf8to16_endianess_hint(const string& u8_src, const vector<uint8_t>& u16_ref_raw, endianess hint)
{
    u16string u16_ref((const char16_t*)u16_ref_raw.data(), u16_ref_raw.size() / 2);
    u16string u16_conv = utf8::utf8to16(u8_src, hint);
    EXPECT_EQ(u16_conv, u16_ref);

    u16_conv.clear();
    unchecked::utf8to16(hint, u8_src.begin(), u8_src.end(), std::back_inserter(u16_conv));
    EXPECT_EQ(u16_conv, u16_ref);

    u16_conv.clear();
    unchecked::utf8to16(!internal::is_byte_swap_required(hint), u8_src.begin(), u8_src.end(), std::back_inserter(u16_conv));
    EXPECT_NE(u16_conv, u16_ref);
}

TEST(CPP11APITests, test_utf8to16_endianess_hint)
{
    // https://r12a.github.io/app-conversion/?q=hello
    string u8_src = "hello";
    vector<uint8_t> u16_ref_raw = { 0x00, 0x68, 0x00, 0x65, 0x00, 0x6C, 0x00, 0x6C, 0x00, 0x6F };
    test_utf8to16_endianess_hint(u8_src, u16_ref_raw, endianess::big_endian);
    swap_u16_raw_buffer(u16_ref_raw);
    test_utf8to16_endianess_hint(u8_src, u16_ref_raw, endianess::little_endian);
}

// Swap bytes in utf-16 characters
void swap_u16_raw_buffer(vector<uint8_t>& buffer)
{
    size_t loopUpperLimit = buffer.size() / 2;
    for (size_t i = 0; i < loopUpperLimit; i++)
    {
        size_t offesetIdx = i * 2;
        uint8_t temp = buffer[offesetIdx];
        buffer[offesetIdx + 0] = buffer[offesetIdx + 1];
        buffer[offesetIdx + 1] = temp;
    }
}

#endif  // C++ 11 or later
