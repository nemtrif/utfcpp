#include "gtest/gtest.h"
#include "utf8/unchecked.h"

using namespace utf8::unchecked;


TEST(UnCheckedIteratrTests, test_increment_char)
{
    auto s = make_cp_sequence(u8"\U00010346\u65e5\u0448\u0041");
    EXPECT_EQ (std::distance(s.begin(), s.end()), 4);
    auto it = s.begin();
    auto it2 = it;
    EXPECT_EQ (it2, it);
    EXPECT_EQ (*it, 0x10346);
    EXPECT_EQ (*it, 0x10346);
    EXPECT_EQ (*(++it), 0x65e5);
    EXPECT_EQ (*it, 0x65e5);
    EXPECT_EQ ((*it++), 0x65e5);
    EXPECT_EQ (*it, 0x0448);
    EXPECT_EQ (*it++, 0x0448);
    EXPECT_EQ (*it, 0x0041);
    EXPECT_EQ (*it, 0x0041);
    EXPECT_NE (it, it2);
    EXPECT_EQ (++it, s.end());

    {
    const char * tab = u8"\U00010346\u65e5\u0448\u0041";
    auto s = make_cp_sequence(tab);
    EXPECT_EQ (std::distance(s.begin(), s.end()), 4);
    auto it = s.begin();
    auto it2 = it;
    EXPECT_EQ (it2, it);
    EXPECT_EQ (*it, 0x10346);
    EXPECT_EQ (*it, 0x10346);
    EXPECT_EQ (*(++it), 0x65e5);
    EXPECT_EQ (*it, 0x65e5);
    EXPECT_EQ ((*it++), 0x65e5);
    EXPECT_EQ (*it, 0x0448);
    EXPECT_EQ (*it++, 0x0448);
    EXPECT_EQ (*it, 0x0041);
    EXPECT_EQ (*it, 0x0041);
    EXPECT_NE (it, it2);
    EXPECT_EQ (++it, s.end());
    }
}

TEST(UnCheckedIteratrTests, test_empty_string) {
    auto tab = "";
    const std::string str{tab};
    constexpr ptrdiff_t dist{};
    {
    auto s = make_cp_sequence(tab);
    EXPECT_EQ (std::distance(s.begin(), s.end()), dist);
    EXPECT_EQ (s.begin(), s.end());
    }
    {
    auto s = make_cp_sequence(str);
    EXPECT_EQ (std::distance(s.begin(), s.end()), dist);
    EXPECT_EQ (s.begin(), s.end());
    }
    {
    auto s = make_cp_sequence(str.c_str());
    EXPECT_EQ (std::distance(s.begin(), s.end()), dist);
    EXPECT_EQ (s.begin(), s.end());
    }
    {
    std::istringstream is = std::istringstream{str};
    is >> std::noskipws;
    auto s = make_cp_sequence(is);
    EXPECT_EQ (s.begin(), s.end());
    }
}

TEST(UnCheckedIteratrTests, test_one_ascii) {
    auto tab = u8"\u0041";
    constexpr int N{0x0041};
    const std::string str{tab};
    constexpr ptrdiff_t dist{1};
    {
    auto s = make_cp_sequence(tab);
    EXPECT_EQ (std::distance(s.begin(), s.end()), dist);
    EXPECT_EQ (*s.begin(), N);
    }
    {
    auto s = make_cp_sequence(str);
    EXPECT_EQ (std::distance(s.begin(), s.end()), dist);
    EXPECT_EQ (*s.begin(), N);
    }
    {
    auto s = make_cp_sequence(str.c_str());
    EXPECT_EQ (std::distance(s.begin(), s.end()), dist);
    EXPECT_EQ (*s.begin(), N);
    }
    {
    std::istringstream is = std::istringstream{str};
    is >> std::noskipws;
    auto s = make_cp_sequence(is);
    auto it = s.begin();
    EXPECT_EQ (*it, N);
    EXPECT_NE (it, s.end());
    EXPECT_NE (it++, s.end());
    EXPECT_EQ (it, s.end());
    }
}

TEST(UnCheckedIteratrTests, test_one_utf8) {
    auto tab = u8"\u00E9";
    constexpr int N{0x00E9};
    const std::string str{tab};
    constexpr ptrdiff_t dist{1};
    {
    auto s = make_cp_sequence(tab);
    EXPECT_EQ (std::distance(s.begin(), s.end()), dist);
    EXPECT_EQ (*s.begin(), N);
    }
    {
    auto s = make_cp_sequence(str);
    EXPECT_EQ (std::distance(s.begin(), s.end()), dist);
    EXPECT_EQ (*s.begin(), N);
    }
    {
    auto s = make_cp_sequence(str.c_str());
    EXPECT_EQ (std::distance(s.begin(), s.end()), dist);
    EXPECT_EQ (*s.begin(), N);
    }
    {
    std::istringstream is = std::istringstream{str};
    is >> std::noskipws;
    auto s = make_cp_sequence(is);
    auto it = s.begin();
    EXPECT_EQ (*it, N);
    EXPECT_NE (it, s.end());
    EXPECT_NE (it++, s.end());
    EXPECT_EQ (it, s.end());
    }
}


TEST(UnCheckedIteratrTests, test_increment_const_string)
{
    const std::string tab{u8"\U00010346\u65e5\u0448\u0041"};
    auto s = make_cp_sequence(tab);
    EXPECT_EQ (std::distance(s.begin(), s.end()), 4);
    auto it = s.begin();
    auto it2 = it;
    EXPECT_EQ (it2, it);
    EXPECT_EQ (*it, 0x10346);
    EXPECT_EQ (*it, 0x10346);
    EXPECT_EQ (*(++it), 0x65e5);
    EXPECT_EQ (*it, 0x65e5);
    EXPECT_EQ ((*it++), 0x65e5);
    EXPECT_EQ (*it, 0x0448);
    EXPECT_EQ (*it++, 0x0448);
    EXPECT_EQ (*it, 0x0041);
    EXPECT_EQ (*it, 0x0041);
    EXPECT_NE (it, it2);
    EXPECT_EQ (++it, s.end());
}

TEST(UnCheckedIteratrTests, test_increment_istream)
{
    const std::string tab{u8"\U00010346\u65e5\u0448\u0041"};
    std::istringstream is{tab};
    is >> std::noskipws;
    auto s = make_cp_sequence(is);
    EXPECT_EQ (std::distance(s.begin(), s.end()), 4);
    is = std::istringstream{tab};//reload
    is >> std::noskipws;
    s = make_cp_sequence(is);
    auto it = s.begin();
    EXPECT_EQ (*it, 0x10346);
    EXPECT_EQ (*it, 0x10346);
    EXPECT_EQ (*(++it), 0x65e5);
    EXPECT_EQ (*it, 0x65e5);
    EXPECT_EQ ((*it++), 0x65e5);
    EXPECT_EQ (*it, 0x0448);
    EXPECT_EQ (*it++, 0x0448);
    EXPECT_EQ (*it, 0x0041);
    EXPECT_EQ (*it, 0x0041);
    EXPECT_EQ (++it, s.end());
    //two end iterators are always equals
    using Iter = std::istream_iterator<char>;
    EXPECT_EQ(it, iterator<Iter>(Iter{}));
    EXPECT_EQ(iterator<Iter>(Iter{}), iterator<Iter>(Iter{}));
}

TEST(UnCheckedIteratrTests, test_increment_string)
{
    std::string tab;
    auto s = make_cp_sequence(tab);
    EXPECT_EQ (std::distance(s.begin(), s.end()), 0);

    tab += u8"\u0448";
    s = make_cp_sequence(tab);
    EXPECT_EQ (std::distance(s.begin(), s.end()), 1);

    EXPECT_EQ (*s.begin(), 0x0448);

    tab += u8"\u00e9\u65e5";
    s = make_cp_sequence(tab);
    EXPECT_EQ (std::distance(s.begin(), s.end()), 3);

    EXPECT_EQ (*s.begin(), 0x0448);
    auto it2 = s.begin();
    std::advance(it2, 2);
    EXPECT_EQ (*it2, 0x65e5);
}

TEST(UnCheckedIteratrTests, test_for_loop) {
    const std::string tab{u8"\u0020\U00010346\u65e5\u0448\u0041"};
    const std::vector<uint32_t> values{0x0020, 0x10346, 0x65e5, 0x448, 0x41};
    {
    auto s = make_cp_sequence(tab);
    auto it = values.begin();
    for(uint32_t cp : s) EXPECT_EQ(cp, *it++);
    }
    {
    auto s = make_cp_sequence(tab.c_str());
    auto it = values.begin();
    for(uint32_t cp : s) EXPECT_EQ(cp, *it++);
    }
    {
    std::istringstream is{tab};
    is >> std::noskipws;
    auto s = make_cp_sequence(is);
    auto it = values.begin();
    for(uint32_t cp : s) EXPECT_EQ(cp, *it++);
    }
}

TEST(UnCheckedIteratrTests, test_vector_char) {
    const std::string tab{u8"\u0020\U00010346\u65e5\u0448\u0041"};
    const std::vector<uint32_t> values{0x0020, 0x10346, 0x65e5, 0x448, 0x41};
    {
    std::vector<char> vchar;
    std::copy(tab.begin(), tab.end(), std::back_inserter(vchar));
    auto s = make_cp_sequence(vchar);
    auto it = values.begin();
    for(uint32_t cp : s) EXPECT_EQ(cp, *it++);
    }
}

TEST(UnCheckedIteratrTests, test_decrement)
{
    const char* threechars = "\xf0\x90\x8d\x86\xe6\x97\xa5\xd1\x88";
    utf8::unchecked::iterator<const char*> it(threechars+9);
    EXPECT_EQ (*(--it), 0x0448);
    EXPECT_EQ ((*it--), 0x0448);
    EXPECT_EQ (*it, 0x65e5);
    EXPECT_EQ (--it, utf8::unchecked::iterator<const char*>(threechars));
    EXPECT_EQ (*it, 0x10346);

}
