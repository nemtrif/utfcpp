#include <iostream>
#include <chrono>
#include <string>
#include <vector>
#include "utf8.h"

static std::string make_ascii_data() {
    std::string s;
    std::string base =
        "Hello World! This is pure ASCII text. "
        "The quick brown fox jumps over the lazy dog. "
        "<html><body>ASCII only content</body></html> ";
    s.reserve(base.size() * 50);
    for (int i = 0; i < 50; ++i) {
        s += base;
    }
    return s;
}

static std::string make_cyrillic_html_data() {
    // Serbian Cyrillic + ASCII in HTML-like content
    std::string s;
    std::string base =
        "<html><body>"
        "Hello World! "
        "Ово је <em>пример</em> текста на <i>српској</i> ћирилици. "
        "Шницла, ћевапи, <b>доручак</b>. "
        "Добродошли у UTF-8 тестирање! "
        "</body></html>";
    s.reserve(base.size() * 50);
    for (int i = 0; i < 50; ++i) {
        s += base;
    }
    return s;
}

static std::string make_mixed_data() {
    std::string utf8_data;
    utf8_data += "Hello World! ";
    utf8_data += "\xc3\xa9\xc3\xa0"; // éà
    utf8_data += "\xd1\x88\xd0\xbd\xd0\xb8\xd1\x86\xd0\xbb\xd0\xb0"; // шницла
    utf8_data += "\xf0\x9f\x98\x80\xf0\x9f\x98\x81"; // 😀😁

    std::string base = utf8_data;
    for (int i = 0; i < 20; ++i) {
        utf8_data += base;
    }
    return utf8_data;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: benchmark <scenario>\n";
        std::cerr << "Scenarios: ascii, cyrillic, mixed\n";
        return 1;
    }

    std::string scenario = argv[1];
    std::string utf8_data;

    if (scenario == "ascii") {
        utf8_data = make_ascii_data();
    } else if (scenario == "cyrillic") {
        utf8_data = make_cyrillic_html_data();
    } else if (scenario == "mixed") {
        utf8_data = make_mixed_data();
    } else {
        std::cerr << "Unknown scenario: " << scenario << "\n";
        return 1;
    }

    const int iterations = 100000;

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        auto it = utf8_data.begin();
        auto end = utf8_data.end();
        while (it != end) {
            utf8::next(it, end);
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration_checked =
        std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    start = std::chrono::high_resolution_clock::now();
    uint32_t sum = 0;
    for (int i = 0; i < iterations; ++i) {
        auto it = utf8_data.begin();
        while (it != utf8_data.end()) {
            sum += utf8::unchecked::next(it);
        }
    }
    end = std::chrono::high_resolution_clock::now();
    auto duration_unchecked =
        std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    start = std::chrono::high_resolution_clock::now();
    std::size_t invalid_index_sum = 0;
    for (int i = 0; i < iterations; ++i) {
        invalid_index_sum += utf8::find_invalid(utf8_data);
    }
    end = std::chrono::high_resolution_clock::now();
    auto duration_find_invalid =
        std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    size_t total_bytes =
        static_cast<size_t>(iterations) * utf8_data.size();
    double total_mb = static_cast<double>(total_bytes) / (1024.0 * 1024.0);

    std::cout << "Function,Time_us,MB_Processed,MB_per_sec,Sum\n";

    double checked_time_sec = duration_checked.count() / 1e6;
    double checked_mbs = total_mb / checked_time_sec;
    std::cout << "utf8::next," << duration_checked.count() << ","
              << total_mb << "," << checked_mbs << ",N/A\n";

    double unchecked_time_sec = duration_unchecked.count() / 1e6;
    double unchecked_mbs = total_mb / unchecked_time_sec;
    std::cout << "utf8::unchecked::next," << duration_unchecked.count() << ","
              << total_mb << "," << unchecked_mbs << "," << sum << "\n";

    double find_invalid_time_sec = duration_find_invalid.count() / 1e6;
    double find_invalid_mbs = total_mb / find_invalid_time_sec;
    std::cout << "utf8::find_invalid," << duration_find_invalid.count() << ","
              << total_mb << "," << find_invalid_mbs << "," << invalid_index_sum << "\n";

    return 0;
}
