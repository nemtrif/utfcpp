#include "ftest.h"
#include "utf8.h"
#include <string>

/* Data from:
UTF-8 decoder capability and stress test
----------------------------------------

Markus Kuhn <http://www.cl.cam.ac.uk/~mgk25/> - 2003-02-19
*/

using namespace utf8;

TEST(NegativeTests, Section1_ValidTextRemainsUnchanged)
{
    std::string  in = "You should see the Greek word 'kosme':       \"κόσμε\"";
    std::string  out;
    replace_invalid(in.begin(), in.end(), std::back_inserter(out), '?');
    EXPECT_EQ(out, in);
    EXPECT_TRUE(is_valid(out.begin(), out.end()));
}

TEST(NegativeTests, Section2_BoundaryConditionUTF8)
{
    std::string in;
    std::string out;

    // 2.1 First possible sequence of a certain length
    in = "\x00"; // U+00000000 (NUL)
    out.clear();
    replace_invalid(in.begin(), in.end(), std::back_inserter(out), '?');
    EXPECT_EQ(out, in);
    EXPECT_TRUE(is_valid(out.begin(), out.end()));

    in = "\xC2\x80"; // U+00000080 (control)
    out.clear();
    replace_invalid(in.begin(), in.end(), std::back_inserter(out), '?');
    EXPECT_EQ(out, in);
    EXPECT_TRUE(is_valid(out.begin(), out.end()));

    in = "\xE0\xA0\x80"; // U+00000800 (first 3-byte char)
    out.clear();
    replace_invalid(in.begin(), in.end(), std::back_inserter(out), '?');
    EXPECT_EQ(out, in);
    EXPECT_TRUE(is_valid(out.begin(), out.end()));

    in = "\xF0\x90\x80\x80"; // U+00010000 (first 4-byte char)
    out.clear();
    replace_invalid(in.begin(), in.end(), std::back_inserter(out), '?');
    EXPECT_EQ(out, in);
    EXPECT_TRUE(is_valid(out.begin(), out.end()));

    in = "\xF8\x80\x80\x80\x80"; // U+00200000 (first 5-byte sequence, invalid)
    out.clear();
    replace_invalid(in.begin(), in.end(), std::back_inserter(out), '?');
    EXPECT_EQ(out, "?????");
    EXPECT_TRUE(is_valid(out.begin(), out.end()));

    in = "\xFC\x80\x80\x80\x80\x80"; // U+04000000 (first 6-byte sequence, invalid)
    out.clear();
    replace_invalid(in.begin(), in.end(), std::back_inserter(out), '?');
    EXPECT_EQ(out, "??????");
    EXPECT_TRUE(is_valid(out.begin(), out.end()));

    // 2.2 Last possible sequence of a certain length
    in = "\x7F"; // U+0000007F (DEL)
    out.clear();
    replace_invalid(in.begin(), in.end(), std::back_inserter(out), '?');
    EXPECT_EQ(out, in);
    EXPECT_TRUE(is_valid(out.begin(), out.end()));

    in = "\xDF\xBF"; // U+000007FF (last 2-byte char)
    out.clear();
    replace_invalid(in.begin(), in.end(), std::back_inserter(out), '?');
    EXPECT_EQ(out, in);
    EXPECT_TRUE(is_valid(out.begin(), out.end()));

    in = "\xEF\xBF\xBF"; // U+0000FFFF (last 3-byte before surrogate range)
    out.clear();
    replace_invalid(in.begin(), in.end(), std::back_inserter(out), '?');
    EXPECT_EQ(out, in);
    EXPECT_TRUE(is_valid(out.begin(), out.end()));

    in = "\xF7\xBF\xBF\xBF"; // U+001FFFFF (invalid code point outside Unicode range)
    out.clear();
    replace_invalid(in.begin(), in.end(), std::back_inserter(out), '?');
    EXPECT_EQ(out, "?");
    EXPECT_TRUE(is_valid(out.begin(), out.end()));

    in = "\xF8\xBF\xBF\xBF\xBF"; // U+03FFFFFF (last 5-byte sequence, invalid)
    out.clear();
    replace_invalid(in.begin(), in.end(), std::back_inserter(out), '?');
    EXPECT_EQ(out, "?????");
    EXPECT_TRUE(is_valid(out.begin(), out.end()));

    in = "\xFC\xBF\xBF\xBF\xBF\xBF"; // U+7FFFFFFF (last 6-byte sequence, invalid)
    out.clear();
    replace_invalid(in.begin(), in.end(), std::back_inserter(out), '?');
    EXPECT_EQ(out, "??????");
    EXPECT_TRUE(is_valid(out.begin(), out.end()));
}


TEST(NegativeTests, Section3_1_UnexpectedContinuationBytes)
{
    std::string in = "\x80";
    std::string out;
    replace_invalid(in.begin(), in.end(), std::back_inserter(out), '?');
    EXPECT_EQ(out, "?");
    EXPECT_TRUE(is_valid(out.begin(), out.end()));

    in = "\xBF";
    out.clear();
    replace_invalid(in.begin(), in.end(), std::back_inserter(out), '?');
    EXPECT_EQ(out, "?");
    EXPECT_TRUE(is_valid(out.begin(), out.end()));

    in = "\x80\x80";
    out.clear();
    replace_invalid(in.begin(), in.end(), std::back_inserter(out), '?');
    EXPECT_EQ(out, "??");
    EXPECT_TRUE(is_valid(out.begin(), out.end()));

    in = "\x80\x80\x80";
    out.clear();
    replace_invalid(in.begin(), in.end(), std::back_inserter(out), '?');
    EXPECT_EQ(out, "???");
    EXPECT_TRUE(is_valid(out.begin(), out.end()));

    in = "\x80\x80\x80\x80";
    out.clear();
    replace_invalid(in.begin(), in.end(), std::back_inserter(out), '?');
    EXPECT_EQ(out, "????");
    EXPECT_TRUE(is_valid(out.begin(), out.end()));

    in.clear();
    for (unsigned byte = 0x80; byte <= 0xBF; ++byte) {
        in.push_back(static_cast<char>(byte));
    }
    out.clear();
    replace_invalid(in.begin(), in.end(), std::back_inserter(out), '?');
    EXPECT_EQ(out, std::string(64, '?'));
    EXPECT_TRUE(is_valid(out.begin(), out.end()));
}

TEST(NegativeTests, Section3_2_LonelyStartBytes)
{
    std::string in;
    std::string out;

    // 3.2.1 All 32 first bytes of 2-byte sequences (0xc0-0xdf), each followed by a space character
    for (unsigned byte = 0xC0; byte <= 0xDF; ++byte) {
        in.clear();
        in.push_back(static_cast<char>(byte));
        in.push_back(' ');
        out.clear();
        replace_invalid(in.begin(), in.end(), std::back_inserter(out), '?');
        EXPECT_EQ(out, "? ");
        EXPECT_TRUE(is_valid(out.begin(), out.end()));
    }

    // 3.2.2 All 16 first bytes of 3-byte sequences (0xe0-0xef), each followed by a space character
    for (unsigned byte = 0xE0; byte <= 0xEF; ++byte) {
        in.clear();
        in.push_back(static_cast<char>(byte));
        in.push_back(' ');
        out.clear();
        replace_invalid(in.begin(), in.end(), std::back_inserter(out), '?');
        EXPECT_EQ(out, "? ");
        EXPECT_TRUE(is_valid(out.begin(), out.end()));
    }

    // 3.2.3 All 8 first bytes of 4-byte sequences (0xf0-0xf7), each followed by a space character
    for (unsigned byte = 0xF0; byte <= 0xF7; ++byte) {
        in.clear();
        in.push_back(static_cast<char>(byte));
        in.push_back(' ');
        out.clear();
        replace_invalid(in.begin(), in.end(), std::back_inserter(out), '?');
        EXPECT_EQ(out, "? ");
        EXPECT_TRUE(is_valid(out.begin(), out.end()));
    }

    // 3.2.4 All 4 first bytes of 5-byte sequences (0xf8-0xfb), each followed by a space character
    for (unsigned byte = 0xF8; byte <= 0xFB; ++byte) {
        in.clear();
        in.push_back(static_cast<char>(byte));
        in.push_back(' ');
        out.clear();
        replace_invalid(in.begin(), in.end(), std::back_inserter(out), '?');
        EXPECT_EQ(out, "? ");
        EXPECT_TRUE(is_valid(out.begin(), out.end()));
    }

    // 3.2.5 All 2 first bytes of 6-byte sequences (0xfc-0xfd), each followed by a space character
    for (unsigned byte = 0xFC; byte <= 0xFD; ++byte) {
        in.clear();
        in.push_back(static_cast<char>(byte));
        in.push_back(' ');
        out.clear();
        replace_invalid(in.begin(), in.end(), std::back_inserter(out), '?');
        EXPECT_EQ(out, "? ");
        EXPECT_TRUE(is_valid(out.begin(), out.end()));
    }
}

TEST(NegativeTests, Section3_3_IncompleteSequences)
{
    std::string in;
    std::string out;

    // 3.3.1  2-byte sequence with last byte missing (U+0000)
    in = "\xC2";
    out.clear();
    replace_invalid(in.begin(), in.end(), std::back_inserter(out), '?');
    EXPECT_EQ(out, "?");
    EXPECT_TRUE(is_valid(out.begin(), out.end()));

    // 3.3.2  3-byte sequence with last byte missing (U+0000)
    in = "\xE0\xA0";
    out.clear();
    replace_invalid(in.begin(), in.end(), std::back_inserter(out), '?');
    EXPECT_EQ(out, "?");
    EXPECT_TRUE(is_valid(out.begin(), out.end()));

    // 3.3.3  4-byte sequence with last byte missing (U+0000)
    in = "\xF0\x90\x80";
    out.clear();
    replace_invalid(in.begin(), in.end(), std::back_inserter(out), '?');
    EXPECT_EQ(out, "?");
    EXPECT_TRUE(is_valid(out.begin(), out.end()));

    // 3.3.4  5-byte sequence with last byte missing (U+0000)
    in = "\xF8\x88\x80\x80";
    out.clear();
    replace_invalid(in.begin(), in.end(), std::back_inserter(out), '?');
    EXPECT_EQ(out, "????");
    EXPECT_TRUE(is_valid(out.begin(), out.end()));

    // 3.3.5  6-byte sequence with last byte missing (U+0000)
    in = "\xFC\x84\x80\x80\x80";
    out.clear();
    replace_invalid(in.begin(), in.end(), std::back_inserter(out), '?');
    EXPECT_EQ(out, "?????");
    EXPECT_TRUE(is_valid(out.begin(), out.end()));

    // 3.3.6  2-byte sequence with last byte missing (U-000007FF)
    in = "\xDF";
    out.clear();
    replace_invalid(in.begin(), in.end(), std::back_inserter(out), '?');
    EXPECT_EQ(out, "?");
    EXPECT_TRUE(is_valid(out.begin(), out.end()));

    // 3.3.7  3-byte sequence with last byte missing (U-0000FFFF)
    in = "\xEF\xBF";
    out.clear();
    replace_invalid(in.begin(), in.end(), std::back_inserter(out), '?');
    EXPECT_EQ(out, "?");
    EXPECT_TRUE(is_valid(out.begin(), out.end()));

    // 3.3.8  4-byte sequence with last byte missing (U-001FFFFF)
    in = "\xF8\x87\xBF\xBF";
    out.clear();
    replace_invalid(in.begin(), in.end(), std::back_inserter(out), '?');
    EXPECT_EQ(out, "????");
    EXPECT_TRUE(is_valid(out.begin(), out.end()));

    // 3.3.9  5-byte sequence with last byte missing (U-03FFFFFF)
    in = "\xFC\x83\xBF\xBF\xBF";
    out.clear();
    replace_invalid(in.begin(), in.end(), std::back_inserter(out), '?');
    EXPECT_EQ(out, "?????");
    EXPECT_TRUE(is_valid(out.begin(), out.end()));

    // 3.3.10 6-byte sequence with last byte missing (U-7FFFFFFF)
    in = "\xFD\x83\xBF\xBF\xBF\xBF";
    out.clear();
    replace_invalid(in.begin(), in.end(), std::back_inserter(out), '?');
    EXPECT_EQ(out, "??????");
    EXPECT_TRUE(is_valid(out.begin(), out.end()));
}

// 3.4 Concatenation of incomplete sequences
TEST(NegativeTests, Section3_4_ConcatenatedIncompletes)
{
    std::string in;
    in += "\xC2";
    in += "\xE0\xA0";
    in += "\xF0\x90\x80";
    in += "\xF8\x88\x80\x80";
    in += "\xFC\x84\x80\x80\x80";
    in += "\xDF";
    in += "\xEF\xBF";
    in += "\xF8\x87\xBF\xBF";
    in += "\xFC\x83\xBF\xBF\xBF";
    in += "\xFD\x83\xBF\xBF\xBF\xBF";

    std::string out;
    replace_invalid(in.begin(), in.end(), std::back_inserter(out), '?');
    EXPECT_EQ(out, std::string(29, '?'));
    EXPECT_TRUE(is_valid(out.begin(), out.end()));
}

// 3.5 Impossible bytes (fe, ff, fe fe ff ff)
TEST(NegativeTests, Section3_5_ImpossibleBytes)
{
    std::string in;
    std::string out;

    in = "\xFE";
    out.clear();
    replace_invalid(in.begin(), in.end(), std::back_inserter(out), '?');
    EXPECT_EQ(out, "?");
    EXPECT_TRUE(is_valid(out.begin(), out.end()));

    in = "\xFF";
    out.clear();
    replace_invalid(in.begin(), in.end(), std::back_inserter(out), '?');
    EXPECT_EQ(out, "?");
    EXPECT_TRUE(is_valid(out.begin(), out.end()));

    in = "\xFE\xFE\xFF\xFF";
    out.clear();
    replace_invalid(in.begin(), in.end(), std::back_inserter(out), '?');
    EXPECT_EQ(out, "????");
    EXPECT_TRUE(is_valid(out.begin(), out.end()));
}

// 4.1 Examples of an overlong ASCII character
TEST(NegativeTests, Section4_1_OverlongSlash)
{
    std::string in;
    std::string out;

    in = "\xC0\xAF";
    out.clear();
    replace_invalid(in.begin(), in.end(), std::back_inserter(out), '?');
    EXPECT_EQ(out, "?");
    EXPECT_TRUE(is_valid(out.begin(), out.end()));

    in = "\xE0\x80\xAF";
    out.clear();
    replace_invalid(in.begin(), in.end(), std::back_inserter(out), '?');
    EXPECT_EQ(out, "?");
    EXPECT_TRUE(is_valid(out.begin(), out.end()));

    in = "\xF0\x80\x80\xAF";
    out.clear();
    replace_invalid(in.begin(), in.end(), std::back_inserter(out), '?');
    EXPECT_EQ(out, "?");
    EXPECT_TRUE(is_valid(out.begin(), out.end()));

    in = "\xF8\x80\x80\x80\xAF";
    out.clear();
    replace_invalid(in.begin(), in.end(), std::back_inserter(out), '?');
    EXPECT_EQ(out, "?????");
    EXPECT_TRUE(is_valid(out.begin(), out.end()));

    in = "\xFC\x80\x80\x80\x80\xAF";
    out.clear();
    replace_invalid(in.begin(), in.end(), std::back_inserter(out), '?');
    EXPECT_EQ(out, "??????");
    EXPECT_TRUE(is_valid(out.begin(), out.end()));
}

// 4.2 Maximum overlong sequences (overlong boundaries)
TEST(NegativeTests, Section4_2_OverlongMaxBoundaries)
{
    std::string in;
    std::string out;

    in = "\xC1\xBF";
    out.clear();
    replace_invalid(in.begin(), in.end(), std::back_inserter(out), '?');
    EXPECT_EQ(out, "?");
    EXPECT_TRUE(is_valid(out.begin(), out.end()));

    in = "\xE0\x9F\xBF";
    out.clear();
    replace_invalid(in.begin(), in.end(), std::back_inserter(out), '?');
    EXPECT_EQ(out, "?");
    EXPECT_TRUE(is_valid(out.begin(), out.end()));

    in = "\xF0\x8F\xBF\xBF";
    out.clear();
    replace_invalid(in.begin(), in.end(), std::back_inserter(out), '?');
    EXPECT_EQ(out, "?");
    EXPECT_TRUE(is_valid(out.begin(), out.end()));

    in = "\xF8\x87\xBF\xBF\xBF";
    out.clear();
    replace_invalid(in.begin(), in.end(), std::back_inserter(out), '?');
    EXPECT_EQ(out, "?????");
    EXPECT_TRUE(is_valid(out.begin(), out.end()));

    in = "\xFC\x83\xBF\xBF\xBF\xBF";
    out.clear();
    replace_invalid(in.begin(), in.end(), std::back_inserter(out), '?');
    EXPECT_EQ(out, "??????");
    EXPECT_TRUE(is_valid(out.begin(), out.end()));
}

// 4.3 Overlong representation of the NUL character
TEST(NegativeTests, Section4_3_OverlongNul)
{
    std::string in;
    std::string out;

    in = "\xC0\x80";
    out.clear();
    replace_invalid(in.begin(), in.end(), std::back_inserter(out), '?');
    EXPECT_EQ(out, "?");
    EXPECT_TRUE(is_valid(out.begin(), out.end()));

    in = "\xE0\x80\x80";
    out.clear();
    replace_invalid(in.begin(), in.end(), std::back_inserter(out), '?');
    EXPECT_EQ(out, "?");
    EXPECT_TRUE(is_valid(out.begin(), out.end()));

    in = "\xF0\x80\x80\x80";
    out.clear();
    replace_invalid(in.begin(), in.end(), std::back_inserter(out), '?');
    EXPECT_EQ(out, "?");
    EXPECT_TRUE(is_valid(out.begin(), out.end()));

    in = "\xF8\x80\x80\x80\x80";
    out.clear();
    replace_invalid(in.begin(), in.end(), std::back_inserter(out), '?');
    EXPECT_EQ(out, "?????");
    EXPECT_TRUE(is_valid(out.begin(), out.end()));

    in = "\xFC\x80\x80\x80\x80\x80";
    out.clear();
    replace_invalid(in.begin(), in.end(), std::back_inserter(out), '?');
    EXPECT_EQ(out, "??????");
    EXPECT_TRUE(is_valid(out.begin(), out.end()));
}

// 5.1 Single UTF-16 surrogates
TEST(NegativeTests, Section5_1_IllegalSingleSurrogates)
{
    std::string in;
    std::string out;

    const char* seqs[] = {"\xED\xA0\x80", "\xED\xAD\xBF", "\xED\xAE\x80", "\xED\xAF\xBF", "\xED\xB0\x80", "\xED\xBE\x80", "\xED\xBF\xBF"};
    for (auto seq : seqs) {
        in.assign(seq, seq + 3);
        out.clear();
        replace_invalid(in.begin(), in.end(), std::back_inserter(out), '?');
        EXPECT_EQ(out, "?");
        EXPECT_TRUE(is_valid(out.begin(), out.end()));
    }
}

// 5.2 Paired UTF-16 surrogates
TEST(NegativeTests, Section5_2_IllegalSurrogatePairs)
{
    std::string in;
    std::string out;

    const char* seqs[] = {
        "\xED\xA0\x80\xED\xB0\x80",
        "\xED\xA0\x80\xED\xBF\xBF",
        "\xED\xAD\xBF\xED\xB0\x80",
        "\xED\xAD\xBF\xED\xBF\xBF",
        "\xED\xAE\x80\xED\xB0\x80",
        "\xED\xAE\x80\xED\xBF\xBF",
        "\xED\xAF\xBF\xED\xB0\x80",
        "\xED\xAF\xBF\xED\xBF\xBF",
    };

    for (auto seq : seqs) {
        in.assign(seq, seq + 6);
        out.clear();
        replace_invalid(in.begin(), in.end(), std::back_inserter(out), '?');
        EXPECT_EQ(out, "??");
        EXPECT_TRUE(is_valid(out.begin(), out.end()));
    }
}
