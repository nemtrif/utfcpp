#ifndef UTF8_FOR_CPP_TEST_CHECKED_ITERATOR_H_2675DCD0_9480_4c0c_B92A_CC14C027B731
#define UTF8_FOR_CPP_TEST_CHECKED_ITERATOR_H_2675DCD0_9480_4c0c_B92A_CC14C027B731

#include "utf8.h"

using namespace utf8;


TEST(CheckedIteratrTests, test_increment)
{
    const char* threechars = "\xf0\x90\x8d\x86\xe6\x97\xa5\xd1\x88";
    utf8::iterator<const char*> it(threechars, threechars, threechars + 9);
    utf8::iterator<const char*> it2 = it;
    EXPECT_EQ (it2, it);
    EXPECT_EQ (*it, 0x10346);
    EXPECT_EQ (*(++it), 0x65e5);
    EXPECT_EQ ((*it++), 0x65e5);
    EXPECT_EQ (*it, 0x0448);
    EXPECT_NE (it, it2);
    utf8::iterator<const char*> endit (threechars + 9, threechars, threechars + 9);
    EXPECT_EQ (++it, endit);
}

TEST(CheckedIteratrTests, test_decrement)
{
    const char* threechars = "\xf0\x90\x8d\x86\xe6\x97\xa5\xd1\x88";
    utf8::iterator<const char*> it(threechars+9, threechars, threechars + 9);
    EXPECT_EQ (*(--it), 0x0448);
    EXPECT_EQ ((*it--), 0x0448);
    EXPECT_EQ (*it, 0x65e5);
    EXPECT_EQ (--it, utf8::iterator<const char*>(threechars, threechars, threechars + 9));
    EXPECT_EQ (*it, 0x10346);
}

TEST(CheckedIteratrTests, test_increment16)
{
    const utfchar16_t codepoints[] = {0x65e5, 0xd800, 0xdf46, 0x0448};
    utf8::iterator16<const utfchar16_t*> it(codepoints, codepoints, codepoints + 4);
    utf8::iterator16<const utfchar16_t*> it2 = it;
    EXPECT_EQ (it2, it);
    EXPECT_EQ (*it, 0x65e5);
    EXPECT_EQ (*(++it), 0x10346);
    EXPECT_EQ ((*it++), 0x10346);
    EXPECT_EQ (*it, 0x0448);
    EXPECT_NE (it, it2);
    utf8::iterator16<const utfchar16_t*> endit(codepoints + 4, codepoints, codepoints + 4);
    EXPECT_EQ (++it, endit);
    EXPECT_THROW(*it, not_enough_room);
}

TEST(CheckedIteratrTests, test_decrement16)
{
    const utfchar16_t codepoints[] = {0x65e5, 0xd800, 0xdf46, 0x0448};
    utf8::iterator16<const utfchar16_t*> it(codepoints + 4, codepoints, codepoints + 4);
    EXPECT_EQ (*(--it), 0x0448);
    EXPECT_EQ ((*it--), 0x0448);
    EXPECT_EQ (*it, 0x10346);
    EXPECT_EQ (
        --it,
        utf8::iterator16<const utfchar16_t*>(codepoints, codepoints, codepoints + 4));
    EXPECT_EQ (*it, 0x65e5);
    EXPECT_THROW(--it, not_enough_room);
}

#endif
