#include "gtest/gtest.h"
#include "utf8/unchecked.h"

using namespace utf8::unchecked;


TEST(UnCheckedIteratrTests, test_increment_char)
{
   const char tab[] = u8"\U00010346\u65e5\u0448\u0041";
	 auto p = make_iterator_pair(tab);
    EXPECT_EQ (std::distance(p.first, p.second), 4);
	 p = make_iterator_pair(tab);
    auto it = p.first;
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
    EXPECT_EQ (++it, p.second);

    {
    const char tab[] = "";
	 p = make_iterator_pair(tab);
    EXPECT_EQ (std::distance(p.first, p.second), 0);
    EXPECT_EQ (p.first, p.second);
    }

}

TEST(UnCheckedIteratrTests, test_increment_const_string)
{
    const std::string tab{u8"\U00010346\u65e5\u0448\u0041"};
	 auto p = make_iterator_pair(tab);
    EXPECT_EQ (std::distance(p.first, p.second), 4);
	 p = make_iterator_pair(tab);
    auto it = p.first;
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
    EXPECT_EQ (++it, p.second);
}

TEST(UnCheckedIteratrTests, test_increment_istream)
{
    const std::string tab{u8"\U00010346\u65e5\u0448\u0041"};
	 std::istringstream is{tab};
	 is >> std::noskipws;
	 auto pair = make_iterator_pair(is);
    EXPECT_EQ (std::distance(pair.first, pair.second), 4);
	 is = std::istringstream{tab};//reload
	 is >> std::noskipws;
	 pair = make_iterator_pair(is);
    auto it = pair.first;
    EXPECT_EQ (*it, 0x10346);
    EXPECT_EQ (*it, 0x10346);
    EXPECT_EQ (*(++it), 0x65e5);
    EXPECT_EQ (*it, 0x65e5);
    EXPECT_EQ ((*it++), 0x65e5);
    EXPECT_EQ (*it, 0x0448);
    EXPECT_EQ (*it++, 0x0448);
    EXPECT_EQ (*it, 0x0041);
    EXPECT_EQ (*it, 0x0041);
    EXPECT_EQ (++it, pair.second);
}

TEST(UnCheckedIteratrTests, test_increment_string)
{
    std::string tab;
	 auto p = make_iterator_pair(tab);
    EXPECT_EQ (std::distance(p.first, p.second), 0);

	 tab += u8"\u0448";
	 p = make_iterator_pair(tab);
    EXPECT_EQ (std::distance(p.first, p.second), 1);

    EXPECT_EQ (*p.first, 0x0448);

	 tab += u8"\u00e9\u65e5";
	 p = make_iterator_pair(tab);
    EXPECT_EQ (std::distance(p.first, p.second), 3);

    EXPECT_EQ (*p.first, 0x0448);
	 auto it2 = p.first;
    std::advance(it2, 2);
    EXPECT_EQ (*it2, 0x65e5);
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
