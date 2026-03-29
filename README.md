
<!-- TOC --><a name="utf8-cpp-utf-8-with-c-in-a-portable-way"></a>
# UTF8-CPP: UTF-8 with C++ in a Portable Way


<!-- TOC --><a name="introduction"></a>
## Introduction

C++ developers still miss an easy and portable way of handling Unicode encoded strings. The original C++ standard (known as C++98 or C++03) is Unicode agnostic. Some progress has been made in the later editions of the standard, but it is still hard to work with Unicode using only the standard facilities.

I came up with a small, C++98 compatible generic library in order to handle UTF-8 encoded strings. For anybody used to work with STL algorithms and iterators, it should be easy and natural to use. The code is freely available for any purpose - check out the [license](./LICENSE). The library has been used a lot since the first release in 2006 both in commercial and open-source projects and proved to be stable and useful.

## Table of Contents

- [UTF8-CPP: UTF-8 with C++ in a Portable Way](#utf8-cpp-utf-8-with-c-in-a-portable-way)
  * [Introduction](#introduction)
  * [Installation](#installation)
  * [Examples of use](#examples-of-use)
    + [Introductory Sample](#introductory-sample)
    + [Checking if a file contains valid UTF-8 text](#checking-if-a-file-contains-valid-utf-8-text)
    + [Ensure that a string contains valid UTF-8 text](#ensure-that-a-string-contains-valid-utf-8-text)
  * [Points of interest](#points-of-interest)
  * [Reference](API_REFERENCE.md)

## Reference

Full API documentation is available in [`API_REFERENCE.md`](API_REFERENCE.md).

<!-- TOC --><a name="installation"></a>
## Installation

This is a header-only library and the supported way of deploying it is:
- Download a release from https://github.com/nemtrif/utfcpp/releases into a temporary directory
- Unzip the release
- Copy the content of utfcpp/source file into the directory where you keep include files for your project


The CMakeList.txt file was originally made for testing purposes only, but unfortunately over time I accepted contributions that added install target. *This is not a supported way of installing the utfcpp library* and I am considering removing the CMakeList.txt in a future release.

<!-- TOC --><a name="examples-of-use"></a>
## Examples of use

<!-- TOC --><a name="introductory-sample"></a>
### Introductory Sample

To illustrate the use of the library, let's start with a small but complete program that opens a file containing UTF-8 encoded text, reads it line by line, checks each line for invalid UTF-8 byte sequences, and converts it to UTF-16 encoding and back to UTF-8:

```cpp
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "utf8.h"
using namespace std;
int main(int argc, char** argv)
{
    if (argc != 2) {
        cout << "\nUsage: docsample filename\n";
        return 0;
    }
    const char* test_file_path = argv[1];
    // Open the test file (must be UTF-8 encoded)
    ifstream fs8(test_file_path);
    if (!fs8.is_open()) {
        cout << "Could not open " << test_file_path << endl;
        return 0;
    }

    unsigned line_count = 1;
    string line;
    // Play with all the lines in the file
    while (getline(fs8, line)) {
        // check for invalid utf-8 (for a simple yes/no check, there is also utf8::is_valid function)
#if __cplusplus >= 201103L // C++ 11 or later
        auto end_it = utf8::find_invalid(line.begin(), line.end());
#else
        string::iterator end_it = utf8::find_invalid(line.begin(), line.end());
#endif // C++ 11
        if (end_it != line.end()) {
            cout << "Invalid UTF-8 encoding detected at line " << line_count << "\n";
            cout << "This part is fine: " << string(line.begin(), end_it) << "\n";
        }
        // Get the line length (at least for the valid part)
        int length = utf8::distance(line.begin(), end_it);
        cout << "Length of line " << line_count << " is " << length <<  "\n";

        // Convert it to utf-16
#if __cplusplus >= 201103L // C++ 11 or later
        u16string utf16line = utf8::utf8to16(line);
#else
        vector<unsigned short> utf16line;
        utf8::utf8to16(line.begin(), end_it, back_inserter(utf16line));
#endif // C++ 11
        // And back to utf-8;
#if __cplusplus >= 201103L // C++ 11 or later
        string utf8line = utf8::utf16to8(utf16line);
#else
        string utf8line; 
        utf8::utf16to8(utf16line.begin(), utf16line.end(), back_inserter(utf8line));
#endif // C++ 11
        // Confirm that the conversion went OK:
        if (utf8line != string(line.begin(), end_it))
            cout << "Error in UTF-16 conversion at line: " << line_count << "\n";        

        line_count++;
    } 

    return 0;
}
```

In the previous code sample, for each line we performed a detection of invalid UTF-8 sequences with `find_invalid`; the number of characters (more precisely - the number of Unicode code points, including the end of line and even BOM if there is one) in each line was determined with a use of `utf8::distance`; finally, we have converted each line to UTF-16 encoding with `utf8to16` and back to UTF-8 with `utf16to8`.

Note a different pattern of usage for old compilers. For instance, this is how we convert
a UTF-8 encoded string to a UTF-16 encoded one with a pre - C++11 compiler:
```cpp
    vector<unsigned short> utf16line;
    utf8::utf8to16(line.begin(), end_it, back_inserter(utf16line));
```

With a more modern compiler, the same operation would look like:
```cpp
    u16string utf16line = utf8::utf8to16(line);
```
If `__cplusplus` macro points to a C++ 11 or later, the library exposes API that takes into 
account C++ standard Unicode strings and move semantics. With an older compiler, it is still
possible to use the same functionality, just in a little less convenient way

In case you do not trust the `__cplusplus` macro or, for instance, do not want to include
the C++ 11 helper functions even with a modern compiler, define `UTF_CPP_CPLUSPLUS` macro
before including `utf8.h` and assign it a value for the standard you want to use - the values are the same as for the `__cplusplus` macro. This can be also useful with compilers that are conservative in setting the `__cplusplus` macro even if they have a good support for a recent standard edition - Microsoft's Visual C++ is one example.

<!-- TOC --><a name="checking-if-a-file-contains-valid-utf-8-text"></a>
### Checking if a file contains valid UTF-8 text

Here is a function that checks whether the content of a file is valid UTF-8 encoded text without reading the content into the memory:

```cpp
bool valid_utf8_file(const char* file_name)
{
    ifstream ifs(file_name);
    if (!ifs)
        return false; // even better, throw here

    istreambuf_iterator<char> it(ifs.rdbuf());
    istreambuf_iterator<char> eos;

    return utf8::is_valid(it, eos);
}
```

Because the function `utf8::is_valid()` works with input iterators, we were able to pass an `istreambuf_iterator` to `it` and read the content of the file directly without loading it to the memory first.

Note that other functions that take input iterator arguments can be used in a similar way. For instance, to read the content of a UTF-8 encoded text file and convert the text to UTF-16, just do something like:

```cpp
    utf8::utf8to16(it, eos, back_inserter(u16string));
```

<!-- TOC --><a name="ensure-that-a-string-contains-valid-utf-8-text"></a>
### Ensure that a string contains valid UTF-8 text

If we have some text that "probably" contains UTF-8 encoded text and we want to replace any invalid UTF-8 sequence with a replacement character, something like the following function may be used:

```cpp
void fix_utf8_string(std::string& str)
{
    std::string temp;
    utf8::replace_invalid(str.begin(), str.end(), back_inserter(temp));
    str = temp;
}
```

The function will replace any invalid UTF-8 sequence with a Unicode replacement character. There is an overloaded function that enables the caller to supply their own replacement character.


<!-- TOC --><a name="points-of-interest"></a>
## Points of interest

<!-- TOC --><a name="design-goals-and-decisions"></a>
#### Design goals and decisions

The library was designed to be:

1.  Generic: for better or worse, there are many C++ string classes out there, and the library should work with as many of them as possible.
2.  Portable: the library should be portable both across different platforms and compilers. The only non-portable code is a small section that declares unsigned integers of different sizes: three typedefs. They can be changed by the users of the library if they don't match their platform. The default setting should work for Windows (both 32 and 64 bit), and most 32 bit and 64 bit Unix derivatives. Support for post C++03 language features is included for modern compilers at API level only, so the library should work even with pretty old compilers.
3.  Lightweight: follow the "pay only for what you use" guideline.
4.  Unintrusive: avoid forcing any particular design or even programming style on the user. This is a library, not a framework.

<!-- TOC --><a name="alternatives"></a>
#### Alternatives

For alternatives and comparisons, I recommend the following article: [The Wonderfully Terrible World of C and C++ Encoding APIs (with Some Rust)](https://thephd.dev/the-c-c++-rust-string-text-encoding-api-landscape), by JeanHeyd Meneide. In the article, this library is compared with:

- [simdutf](https://github.com/simdutf/simdutf)
- [iconv](https://www.gnu.org/software/libiconv/)
- [boost.text](https://github.com/tzlaine/text)
- [ICU](https://unicode-org.github.io/icu/userguide/conversion/converters.html)
- [encoding_rs](https://github.com/hsivonen/encoding_rs)
- [Windows API functions for converting text between encodings](https://learn.microsoft.com/en-us/windows/win32/api/stringapiset/)
- [ztd.text](https://github.com/soasis/text/)

The article presents author's view of the quality of the API design, but also some speed benchmarks.

<!-- TOC --><a name="reference"></a>