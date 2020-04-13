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


#ifndef UTF8_FOR_CPP_CORE_H_2675DCD0_9480_4c0c_B92A_CC14C027B731
#define UTF8_FOR_CPP_CORE_H_2675DCD0_9480_4c0c_B92A_CC14C027B731

#include <iterator>
#include <stdexcept>

// Determine the C++ standard version.
// If the user defines UTF_CPP_CPLUSPLUS, use that.
// Otherwise, trust the unreliable predefined macro __cplusplus

#if !defined UTF_CPP_CPLUSPLUS
    #define UTF_CPP_CPLUSPLUS __cplusplus
#endif

#if UTF_CPP_CPLUSPLUS >= 201103L // C++ 11 or later
    #define OVERRIDE override
    #define NOEXCEPT noexcept
#else // C++ 98/03
    #define OVERRIDE
    #define NOEXCEPT throw()
#endif // C++ 11 or later


namespace utf8
{
    // The typedefs for 8-bit, 16-bit and 32-bit unsigned integers
    // You may need to change them to match your system.
    // These typedefs have the same names as ones from cstdint, or boost/cstdint
    typedef unsigned char   uint8_t;
    typedef unsigned short  uint16_t;
    typedef unsigned int    uint32_t;

    enum endianess
    {
        little_endian,
        big_endian,
    };

// Helper code - not intended to be directly called by the library users. May be changed at any time
namespace internal
{
    // Unicode constants
    // Leading (high) surrogates: 0xd800 - 0xdbff
    // Trailing (low) surrogates: 0xdc00 - 0xdfff
    const uint16_t LEAD_SURROGATE_MIN  = 0xd800u;
    const uint16_t LEAD_SURROGATE_MAX  = 0xdbffu;
    const uint16_t TRAIL_SURROGATE_MIN = 0xdc00u;
    const uint16_t TRAIL_SURROGATE_MAX = 0xdfffu;
    const uint16_t LEAD_OFFSET         = 0xd7c0u;       // LEAD_SURROGATE_MIN - (0x10000 >> 10)
    const uint32_t SURROGATE_OFFSET    = 0xfca02400u;   // 0x10000u - (LEAD_SURROGATE_MIN << 10) - TRAIL_SURROGATE_MIN

    // Maximum valid value for a Unicode code point
    const uint32_t CODE_POINT_MAX      = 0x0010ffffu;

    // Perform an runtime check to determine if byte swap is required
    // with the given endianess hint
    inline bool is_byte_swap_required(endianess hint)
    {
        switch (hint)
        {
        case little_endian:
            return (*(const uint16_t*)"\1\0" >> 8) == 1;
        case big_endian:
            return (*(const uint16_t*)"\0\1" >> 8) == 1;
        default:
            throw std::runtime_error("Unexpected endianess hint");
        }
    }

    template<typename octet_type>
    inline uint8_t mask8(octet_type oc)
    {
        return static_cast<uint8_t>(0xff & oc);
    }

    template<typename u16_type>
    inline uint16_t mask16(u16_type oc)
    {
        return static_cast<uint16_t>(0xffff & oc);
    }

    template<typename octet_type>
    inline bool is_trail(octet_type oc)
    {
        return ((utf8::internal::mask8(oc) >> 6) == 0x2);
    }

    template <typename u16>
    inline bool is_lead_surrogate(u16 cp)
    {
        return (cp >= LEAD_SURROGATE_MIN && cp <= LEAD_SURROGATE_MAX);
    }

    template <typename u16>
    inline bool is_trail_surrogate(u16 cp)
    {
        return (cp >= TRAIL_SURROGATE_MIN && cp <= TRAIL_SURROGATE_MAX);
    }

    template <typename u16>
    inline bool is_surrogate(u16 cp)
    {
        return (cp >= LEAD_SURROGATE_MIN && cp <= TRAIL_SURROGATE_MAX);
    }

    template <typename u32>
    inline bool is_code_point_valid(u32 cp)
    {
        return (cp <= CODE_POINT_MAX && !utf8::internal::is_surrogate(cp));
    }

    template <typename octet_iterator>
    inline typename std::iterator_traits<octet_iterator>::difference_type
    sequence_length(octet_iterator lead_it)
    {
        uint8_t lead = utf8::internal::mask8(*lead_it);
        if (lead < 0x80)
            return 1;
        else if ((lead >> 5) == 0x6)
            return 2;
        else if ((lead >> 4) == 0xe)
            return 3;
        else if ((lead >> 3) == 0x1e)
            return 4;
        else
            return 0;
    }

    template <typename octet_difference_type>
    inline bool is_overlong_sequence(uint32_t cp, octet_difference_type length)
    {
        if (cp < 0x80) {
            if (length != 1) 
                return true;
        }
        else if (cp < 0x800) {
            if (length != 2) 
                return true;
        }
        else if (cp < 0x10000) {
            if (length != 3) 
                return true;
        }

        return false;
    }

    enum utf_error
    {
        utf8_ok,
        not_enough_room,
        invalid_lead,
        incomplete_sequence,
        overlong_sequence,
        invalid_code_point
    };

    /// Helper for get_sequence_x
    template <typename octet_iterator>
    utf_error increase_safely(octet_iterator& it, octet_iterator end)
    {
        if (++it == end)
            return not_enough_room;

        if (!utf8::internal::is_trail(*it))
            return incomplete_sequence;

        return utf8_ok;
    }

    #define UTF8_CPP_INCREASE_AND_RETURN_ON_ERROR(IT, END) {utf_error ret = increase_safely(IT, END); if (ret != utf8_ok) return ret;}    

    /// get_sequence_x functions decode utf-8 sequences of the length x
    template <typename octet_iterator>
    utf_error get_sequence_1(octet_iterator& it, octet_iterator end, uint32_t& code_point)
    {
        if (it == end)
            return not_enough_room;

        code_point = utf8::internal::mask8(*it);

        return utf8_ok;
    }

    template <typename octet_iterator>
    utf_error get_sequence_2(octet_iterator& it, octet_iterator end, uint32_t& code_point)
    {
        if (it == end) 
            return not_enough_room;

        code_point = utf8::internal::mask8(*it);

        UTF8_CPP_INCREASE_AND_RETURN_ON_ERROR(it, end)

        code_point = ((code_point << 6) & 0x7ff) + ((*it) & 0x3f);

        return utf8_ok;
    }

    template <typename octet_iterator>
    utf_error get_sequence_3(octet_iterator& it, octet_iterator end, uint32_t& code_point)
    {
        if (it == end)
            return not_enough_room;
            
        code_point = utf8::internal::mask8(*it);

        UTF8_CPP_INCREASE_AND_RETURN_ON_ERROR(it, end)

        code_point = ((code_point << 12) & 0xffff) + ((utf8::internal::mask8(*it) << 6) & 0xfff);

        UTF8_CPP_INCREASE_AND_RETURN_ON_ERROR(it, end)

        code_point += (*it) & 0x3f;

        return utf8_ok;
    }

    template <typename octet_iterator>
    utf_error get_sequence_4(octet_iterator& it, octet_iterator end, uint32_t& code_point)
    {
        if (it == end)
           return not_enough_room;

        code_point = utf8::internal::mask8(*it);

        UTF8_CPP_INCREASE_AND_RETURN_ON_ERROR(it, end)

        code_point = ((code_point << 18) & 0x1fffff) + ((utf8::internal::mask8(*it) << 12) & 0x3ffff);

        UTF8_CPP_INCREASE_AND_RETURN_ON_ERROR(it, end)

        code_point += (utf8::internal::mask8(*it) << 6) & 0xfff;

        UTF8_CPP_INCREASE_AND_RETURN_ON_ERROR(it, end)

        code_point += (*it) & 0x3f;

        return utf8_ok;
    }

    #undef UTF8_CPP_INCREASE_AND_RETURN_ON_ERROR

    template <typename octet_iterator>
    utf_error validate_next(octet_iterator& it, octet_iterator end, uint32_t& code_point)
    {
        if (it == end)
            return not_enough_room;

        // Save the original value of it so we can go back in case of failure
        // Of course, it does not make much sense with i.e. stream iterators
        octet_iterator original_it = it;

        uint32_t cp = 0;
        // Determine the sequence length based on the lead octet
        typedef typename std::iterator_traits<octet_iterator>::difference_type octet_difference_type;
        const octet_difference_type length = utf8::internal::sequence_length(it);

        // Get trail octets and calculate the code point
        utf_error err = utf8_ok;
        switch (length) {
            case 0:
                return invalid_lead;
            case 1:
                err = utf8::internal::get_sequence_1(it, end, cp);
                break;
            case 2:
                err = utf8::internal::get_sequence_2(it, end, cp);
            break;
            case 3:
                err = utf8::internal::get_sequence_3(it, end, cp);
            break;
            case 4:
                err = utf8::internal::get_sequence_4(it, end, cp);
            break;
        }

        if (err == utf8_ok) {
            // Decoding succeeded. Now, security checks...
            if (utf8::internal::is_code_point_valid(cp)) {
                if (!utf8::internal::is_overlong_sequence(cp, length)){
                    // Passed! Return here.
                    code_point = cp;
                    ++it;
                    return utf8_ok;
                }
                else
                    err = overlong_sequence;
            }
            else 
                err = invalid_code_point;
        }

        // Failure branch - restore the original value of the iterator
        it = original_it;
        return err;
    }

    template <typename octet_iterator>
    inline utf_error validate_next(octet_iterator& it, octet_iterator end) {
        uint32_t ignored;
        return utf8::internal::validate_next(it, end, ignored);
    }

} // namespace internal

    /// The library API - functions intended to be called by the users

    // Byte order mark
    const uint8_t bom[] = {0xef, 0xbb, 0xbf};

    template <typename octet_iterator>
    octet_iterator find_invalid(octet_iterator start, octet_iterator end)
    {
        octet_iterator result = start;
        while (result != end) {
            utf8::internal::utf_error err_code = utf8::internal::validate_next(result, end);
            if (err_code != internal::utf8_ok)
                return result;
        }
        return result;
    }

    template <typename octet_iterator>
    inline bool is_valid(octet_iterator start, octet_iterator end)
    {
        return (utf8::find_invalid(start, end) == end);
    }

    template <typename octet_iterator>
    inline bool starts_with_bom (octet_iterator it, octet_iterator end)
    {
        return (
            ((it != end) && (utf8::internal::mask8(*it++)) == bom[0]) &&
            ((it != end) && (utf8::internal::mask8(*it++)) == bom[1]) &&
            ((it != end) && (utf8::internal::mask8(*it))   == bom[2])
           );
    }

    struct swapped
    {
        static inline uint16_t handle(uint16_t c)
        {
            // Perform byte swap
            return ((c & 0xff00) >> 8) | ((c & 0x00ff) << 8);
        }
    };

    struct unswapped
    {
        static inline uint16_t handle(uint16_t c)
        {
            // Just return same number
            return c;
        }
    };

    // Handle reading/writing of utf16 character, swapping byte if needed/requested
    #define HANDLE_U16C(handler, x) handler::handle(static_cast<uint16_t>(x & 0xffff))

    template <typename swap_handler, typename u16bit_iterator, typename octet_iterator>
    octet_iterator utf16to8_checked(u16bit_iterator start, u16bit_iterator end, octet_iterator result)
    {
        while (start != end) {
            uint32_t cp = HANDLE_U16C(swap_handler, *start++);
            // Take care of surrogate pairs first
            if (utf8::internal::is_lead_surrogate(cp)) {
                if (start != end) {
                    uint32_t trail_surrogate = HANDLE_U16C(swap_handler, *start++);
                    if (utf8::internal::is_trail_surrogate(trail_surrogate))
                        cp = (cp << 10) + trail_surrogate + internal::SURROGATE_OFFSET;
                    else
                        throw invalid_utf16(static_cast<uint16_t>(trail_surrogate));
                }
                else
                    throw invalid_utf16(static_cast<uint16_t>(cp));

            }
            // Lone trail surrogate
            else if (utf8::internal::is_trail_surrogate(cp))
                throw invalid_utf16(static_cast<uint16_t>(cp));

            result = utf8::append(cp, result);
        }
        return result;
    }

    template <typename swap_handler, typename u16bit_iterator, typename octet_iterator>
    octet_iterator utf16to8_unchecked(u16bit_iterator start, u16bit_iterator end, octet_iterator result)
    {
        while (start != end) {
            uint32_t cp = HANDLE_U16C(swap_handler, *start++);
            // Take care of surrogate pairs first
            if (utf8::internal::is_lead_surrogate(cp)) {
                uint32_t trail_surrogate = HANDLE_U16C(swap_handler, *start++);
                cp = (cp << 10) + trail_surrogate + internal::SURROGATE_OFFSET;
            }
            result = utf8::unchecked::append(cp, result);
        }
        return result;
    }

    template <typename swap_handler, typename u16bit_iterator, typename octet_iterator>
    u16bit_iterator utf8to16_checked(bool swapbytes, octet_iterator start, octet_iterator end, u16bit_iterator result)
    {
        while (start < end) {
            uint32_t cp = utf8::next(start, end);
            if (cp > 0xffff) { //make a surrogate pair
                *result++ = HANDLE_U16C(swap_handler, (cp >> 10) + internal::LEAD_OFFSET);
                *result++ = HANDLE_U16C(swap_handler, (cp & 0x3ff) + internal::TRAIL_SURROGATE_MIN);
            }
            else
                *result++ = HANDLE_U16C(swap_handler, cp);
        }
        return result;
    }

    template <typename swap_handler, typename u16bit_iterator, typename octet_iterator>
    u16bit_iterator utf8to16_unchecked(octet_iterator start, octet_iterator end, u16bit_iterator result)
    {
        while (start < end) {
            uint32_t cp = utf8::unchecked::next(start);
            if (cp > 0xffff) { //make a surrogate pair
                *result++ = HANDLE_U16C(swap_handler, (cp >> 10) + internal::LEAD_OFFSET);
                *result++ = HANDLE_U16C(swap_handler, (cp & 0x3ff) + internal::TRAIL_SURROGATE_MIN);
            }
            else
                *result++ = HANDLE_U16C(swap_handler, cp);
        }
        return result;
    }
} // namespace utf8

#endif // header guard


