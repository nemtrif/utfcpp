// Copyright 2006 Nemanja Trifunovic

/*
Permission is hereby granted, free of charge, to any person or organization
obtaining a copy of the software and accompanying documentation covered by
this license (the "Software") to use, reproduce, display, distribute,
execute, and transmit the Software, and to prepare derivative works of the
Software, and to permit third-parties to whom the Software is furnished to
do so, all subject to the following:

The copyright notices in the Software and this entire statement, including
the above license grant, this restriction and the following disclaimer,
must be included in all copies of the Software, in whole or in part, and
all derivative works of the Software, unless such copies or derivative
works are solely in the form of machine-executable object code generated by
a source language processor.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/


#ifndef UTF8_FOR_CPP_UNCHECKED_H_2675DCD0_9480_4c0c_B92A_CC14C027B731
#define UTF8_FOR_CPP_UNCHECKED_H_2675DCD0_9480_4c0c_B92A_CC14C027B731

#include "core.h"
#include "cpp11_facilities.h"
#include <utility>

namespace utf8
{
    namespace unchecked
    {
        template <typename octet_iterator>
        octet_iterator append(uint32_t cp, octet_iterator result)
        {
            if (cp < 0x80)                        // one octet
                *(result++) = static_cast<uint8_t>(cp);
            else if (cp < 0x800) {                // two octets
                *(result++) = static_cast<uint8_t>((cp >> 6)          | 0xc0);
                *(result++) = static_cast<uint8_t>((cp & 0x3f)        | 0x80);
            }
            else if (cp < 0x10000) {              // three octets
                *(result++) = static_cast<uint8_t>((cp >> 12)         | 0xe0);
                *(result++) = static_cast<uint8_t>(((cp >> 6) & 0x3f) | 0x80);
                *(result++) = static_cast<uint8_t>((cp & 0x3f)        | 0x80);
            }
            else {                                // four octets
                *(result++) = static_cast<uint8_t>((cp >> 18)         | 0xf0);
                *(result++) = static_cast<uint8_t>(((cp >> 12) & 0x3f)| 0x80);
                *(result++) = static_cast<uint8_t>(((cp >> 6) & 0x3f) | 0x80);
                *(result++) = static_cast<uint8_t>((cp & 0x3f)        | 0x80);
            }
            return result;
        }

        template <typename octet_iterator, typename output_iterator>
        output_iterator replace_invalid(octet_iterator start, octet_iterator end, output_iterator out, uint32_t replacement)
        {
            while (start != end) {
                octet_iterator sequence_start = start;
                internal::utf_error err_code = utf8::internal::validate_next(start, end);
                switch (err_code) {
                    case internal::UTF8_OK :
                        for (octet_iterator it = sequence_start; it != start; ++it)
                            *out++ = *it;
                        break;
                    case internal::NOT_ENOUGH_ROOM:
                        out = utf8::unchecked::append (replacement, out);
                        start = end;
                        break;
                    case internal::INVALID_LEAD:
                        out = utf8::unchecked::append (replacement, out);
                        ++start;
                        break;
                    case internal::INCOMPLETE_SEQUENCE:
                    case internal::OVERLONG_SEQUENCE:
                    case internal::INVALID_CODE_POINT:
                        out = utf8::unchecked::append (replacement, out);
                        ++start;
                        // just one replacement mark for the sequence
                        while (start != end && utf8::internal::is_trail(*start))
                            ++start;
                        break;
                }
            }
            return out;
        }

        template <typename octet_iterator, typename output_iterator>
        inline output_iterator replace_invalid(octet_iterator start, octet_iterator end, output_iterator out)
        {
            static const uint32_t replacement_marker = utf8::internal::mask16(0xfffd);
            return utf8::unchecked::replace_invalid(start, end, out, replacement_marker);
        }

        template <typename octet_iterator>
        uint32_t next_impl(octet_iterator& it)
        {
            uint32_t cp = utf8::internal::mask8(*it);
            typename std::iterator_traits<octet_iterator>::difference_type length = utf8::internal::sequence_length(it);
            switch (length) {
                case 1:
                    break;
                case 2:
                    it++;
                    cp = ((cp << 6) & 0x7ff) + ((*it) & 0x3f);
                    break;
                case 3:
                    ++it; 
                    cp = ((cp << 12) & 0xffff) + ((utf8::internal::mask8(*it) << 6) & 0xfff);
                    ++it;
                    cp += (*it) & 0x3f;
                    break;
                case 4:
                    ++it;
                    cp = ((cp << 18) & 0x1fffff) + ((utf8::internal::mask8(*it) << 12) & 0x3ffff);                
                    ++it;
                    cp += (utf8::internal::mask8(*it) << 6) & 0xfff;
                    ++it;
                    cp += (*it) & 0x3f; 
                    break;
            }
            return cp;
        }

        template <typename octet_iterator>
        inline uint32_t next(octet_iterator& it) {
            uint32_t cp{next_impl(it)};
            ++it;
            return cp;
        }

        template <typename octet_iterator>
        uint32_t peek_next(octet_iterator it)
        {
            return utf8::unchecked::next(it);
        }

        template <typename octet_iterator>
        uint32_t prior(octet_iterator& it)
        {
            while (utf8::internal::is_trail(*(--it))) ;
            octet_iterator temp = it;
            return utf8::unchecked::next(temp);
        }

        template <typename octet_iterator, typename distance_type>
        void advance (octet_iterator& it, distance_type n)
        {
            const distance_type zero(0);
            if (n < zero) {
                // backward
                for (distance_type i = n; i < zero; ++i)
                    utf8::unchecked::prior(it);
            } else {
                // forward
                for (distance_type i = zero; i < n; ++i)
                    utf8::unchecked::next(it);
            }
        }

        template <typename octet_iterator>
        typename std::iterator_traits<octet_iterator>::difference_type
        distance (octet_iterator first, octet_iterator last)
        {
            typename std::iterator_traits<octet_iterator>::difference_type dist;
            for (dist = 0; first != last; ++dist) 
                utf8::unchecked::next(first);
            return dist;
        }

        template <typename u16bit_iterator, typename octet_iterator>
        octet_iterator utf16to8 (u16bit_iterator start, u16bit_iterator end, octet_iterator result)
        {
            while (start != end) {
                uint32_t cp = utf8::internal::mask16(*start++);
            // Take care of surrogate pairs first
                if (utf8::internal::is_lead_surrogate(cp)) {
                    uint32_t trail_surrogate = utf8::internal::mask16(*start++);
                    cp = (cp << 10) + trail_surrogate + internal::SURROGATE_OFFSET;
                }
                result = utf8::unchecked::append(cp, result);
            }
            return result;
        }

        template <typename u16bit_iterator, typename octet_iterator>
        u16bit_iterator utf8to16 (octet_iterator start, octet_iterator end, u16bit_iterator result)
        {
            while (start < end) {
                uint32_t cp = utf8::unchecked::next(start);
                if (cp > 0xffff) { //make a surrogate pair
                    *result++ = static_cast<uint16_t>((cp >> 10)   + internal::LEAD_OFFSET);
                    *result++ = static_cast<uint16_t>((cp & 0x3ff) + internal::TRAIL_SURROGATE_MIN);
                }
                else
                    *result++ = static_cast<uint16_t>(cp);
            }
            return result;
        }

        template <typename octet_iterator, typename u32bit_iterator>
        octet_iterator utf32to8 (u32bit_iterator start, u32bit_iterator end, octet_iterator result)
        {
            while (start != end)
                result = utf8::unchecked::append(*(start++), result);

            return result;
        }

        template <typename octet_iterator, typename u32bit_iterator>
        u32bit_iterator utf8to32 (octet_iterator start, octet_iterator end, u32bit_iterator result)
        {
            while (start < end)
                (*result++) = utf8::unchecked::next(start);

            return result;
        }

        namespace internal {
        // The bidirectional_iterator class
        template <typename octet_iterator>
          class bidirectional_iterator { 
            octet_iterator it;
            public:
            using iterator_category = std::bidirectional_iterator_tag;
            using value_type = uint32_t;
            using difference_type = ptrdiff_t;
            using pointer = uint32_t*;
            using reference = uint32_t&;

            bidirectional_iterator () {}
            explicit bidirectional_iterator (const octet_iterator& octet_it): it(octet_it) {}
            // the default "big three" are OK
            octet_iterator base () const { return it; }
            uint32_t operator * () const
            {
                octet_iterator temp = it;
                return utf8::unchecked::next(temp);
            }
            bool operator == (const bidirectional_iterator& rhs) const 
            { 
                return (it == rhs.it);
            }
            bool operator != (const bidirectional_iterator& rhs) const
            {
                return !(operator == (rhs));
            }
            bidirectional_iterator& operator ++ () 
            {
                ::std::advance(it, utf8::internal::sequence_length(it));
                return *this;
            }
            bidirectional_iterator operator ++ (int)
            {
                bidirectional_iterator temp = *this;
                ++(*this);
                return temp;
            }  
            bidirectional_iterator& operator -- ()
            {
                utf8::unchecked::prior(it);
                return *this;
            }
            bidirectional_iterator operator -- (int)
            {
                bidirectional_iterator temp = *this;
                --(*this);
                return temp;
            }
          }; // class bidirectional_iterator

        class iterator_stream {
          public:
          using Iter = std::istream_iterator<char>;

          using iterator_category = Iter::iterator_category;
          using value_type = uint32_t;
          using difference_type = ptrdiff_t;
          using pointer = uint32_t*;
          using reference = uint32_t&;

          private:
          Iter it;
          Iter end_range{};
          uint32_t cp{};
          bool ok{};

          void read() {
             ok = it != end_range;
             if(ok) {
              cp = utf8::unchecked::next_impl(it);
             }
          }

          public:
          iterator_stream () {}
          explicit iterator_stream (const Iter& octet_it) : it(octet_it)
          {
              read();
          }
          Iter base () const { return it; }
          uint32_t operator * () const
          {
              return cp;
          }
    
          bool operator == (const iterator_stream& rhs) const
          {
              return ok == rhs.ok && (!ok || it == rhs.it);
          }
          bool operator != (const iterator_stream& rhs) const
          {
              return !(operator == (rhs));
          }
          iterator_stream& operator ++ ()
          {
              ++it;
              read();
              return *this;
          }
          iterator_stream operator ++ (int)
          {
              iterator_stream temp = *this;
              ++(*this);
              return temp;
          }
        }; // class iterator_stream

        template <typename octet_iterator>
        struct get_iterator_class {
        private:
          static void get(std::input_iterator_tag);//not supported
          static bidirectional_iterator<octet_iterator> get(std::bidirectional_iterator_tag);
        public:
          using type = utf8::internal::Conditional<
          std::is_same<std::istream_iterator<char>, octet_iterator>::value,
          iterator_stream,
          decltype(get(utf8::internal::Iterator_category<octet_iterator>{}))>;
        };
        }//internal

        template <typename octet_iterator>
        using iterator = typename internal::get_iterator_class<octet_iterator>::type;

        /**
         * a class for containers like std::string
         * to ease the creation of a code point iterator begin and end
         */ 
        template <typename Cont>
        class utf8_sequence {
        public:
          using Iter = typename unchecked::iterator<typename Cont::const_iterator>;
          using iterator = Iter;
          using const_iterator = Iter;
          using value_type = typename std::iterator_traits<Iter>::value_type;
        private:
          const Cont *p{};
        public:
          explicit utf8_sequence(const Cont& c) : p{&c} {}
          Iter begin() const {
            return Iter{std::begin(*p)};
          }
          Iter end() const {
            return Iter{std::end(*p)};
          }
        };

        /**
         * a class for null-terminated strings
         * to ease the creation of a code point iterator begin and end
         */ 
        class utf8_sequence_char {
        public:
          using Iter = unchecked::iterator<const char*>;
          using const_iterator = Iter;
          using value_type = typename std::iterator_traits<Iter>::value_type;
        private:
          const char *range_start{};
          const char *range_end{};
        public:
          /**
           * \param n : size of string, excluding the terminating null byte
           */
          utf8_sequence_char(const char *p, size_t n) : range_start{p}, range_end{p+n} {}
          explicit utf8_sequence_char(const char *p) : utf8_sequence_char{p, std::char_traits<char>::length(p)} {}
          template<size_t N>
          explicit utf8_sequence_char(const char(&tab)[N]) : utf8_sequence_char{&tab[0], N-1} { }
          Iter begin() const {
            return Iter{range_start};
          }
          Iter end() const {
            return Iter{range_end};
          }
        };

        /**
         * a class for istream
         * to ease the creation of a code point iterator begin and end
         */ 
        class utf8_sequence_stream {
        private:
          using Iter_base = std::istream_iterator<char>;
        public:
          using Iter = unchecked::iterator<Iter_base>;
          using iterator = Iter;
          using const_iterator = Iter;
          using value_type = typename std::iterator_traits<Iter>::value_type;
        private:
          Iter_base range_start;
          Iter_base range_end{};
        public:
          explicit utf8_sequence_stream(std::istream& s) : range_start{s}{}
          Iter begin() const {
            return Iter{range_start};
          }
          Iter end() const {
            return Iter{range_end};
          }
        };
        /// make_cp_sequence to create a container of cp point
        template <typename Cont, typename T = utf8::internal::Decay<decltype(*std::begin(std::declval<Cont>()))>>
        utf8_sequence<Cont> make_cp_sequence(const Cont& c) {
          return utf8_sequence<Cont>{c};
        }
        inline utf8_sequence_char make_cp_sequence(const char *s) {
          return utf8_sequence_char{s};
        }
        inline utf8_sequence_char make_cp_sequence(const char *s, size_t n) {
          return utf8_sequence_char{s, n};
        }
        inline utf8_sequence_stream make_cp_sequence(std::istream& is) {
          return utf8_sequence_stream{is};
        }
        template <size_t N>
        utf8_sequence_char make_cp_sequence(const char (&tab)[N]) {
          return utf8_sequence_char{tab};
        }

    } // namespace utf8::unchecked
} // namespace utf8 


#endif // header guard

