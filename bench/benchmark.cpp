#include <iostream>
#include <chrono>
#include <string>
#include <vector>
#include "utf8.h"

int main() {
    // Create test data with various UTF-8 sequences
    std::string utf8_data;
    // Add some ASCII
    utf8_data += "Hello World! ";
    // Add 2-byte sequences (Latin Extended)
    utf8_data += "\xc3\xa9\xc3\xa0"; // éà
    // Add 3-byte sequences (Cyrillic)
    utf8_data += "\xd1\x88\xd0\xbd\xd0\xb8\xd1\x86\xd0\xbb\xd0\xb0"; // шницла
    // Add 4-byte sequences (Emojis)
    utf8_data += "\xf0\x9f\x98\x80\xf0\x9f\x98\x81"; // 😀😁
    // Repeat to make it larger
    std::string base = utf8_data;
    for (int i = 0; i < 20; ++i) {
        utf8_data += base;
    }

    const int iterations = 100000;

    // Benchmark checked next
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        auto it = utf8_data.begin();
        auto end = utf8_data.end();
        while (it != end) {
            utf8::next(it, end);
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration_checked = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    // Benchmark unchecked next
    start = std::chrono::high_resolution_clock::now();
    uint32_t sum = 0;
    for (int i = 0; i < iterations; ++i) {
        auto it = utf8_data.begin();
        while (it != utf8_data.end()) {
            sum += utf8::unchecked::next(it);
        }
    }
    end = std::chrono::high_resolution_clock::now();
    auto duration_unchecked = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    // Benchmark find_invalid
    start = std::chrono::high_resolution_clock::now();
    std::size_t invalid_index_sum = 0;
    for (int i = 0; i < iterations; ++i) {
        invalid_index_sum += utf8::find_invalid(utf8_data);
    }
    end = std::chrono::high_resolution_clock::now();
    auto duration_find_invalid = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    // Calculate total data processed
    size_t total_bytes = static_cast<size_t>(iterations) * utf8_data.size();
    double total_mb = static_cast<double>(total_bytes) / (1024.0 * 1024.0);

    // Output results in CSV format
    std::cout << "Function,Time_us,MB_Processed,MB_per_sec,Sum" << std::endl;
    
    double checked_time_sec = static_cast<double>(duration_checked.count()) / 1e6;
    double checked_mbs = total_mb / checked_time_sec;
    std::cout << "utf8::next," << duration_checked.count() << "," << total_mb << "," << checked_mbs << "," << "N/A" << std::endl;
    
    double unchecked_time_sec = static_cast<double>(duration_unchecked.count()) / 1e6;
    double unchecked_mbs = total_mb / unchecked_time_sec;
    std::cout << "utf8::unchecked::next," << duration_unchecked.count() << "," << total_mb << "," << unchecked_mbs << "," << sum << std::endl;

    double find_invalid_time_sec = static_cast<double>(duration_find_invalid.count()) / 1e6;
    double find_invalid_mbs = total_mb / find_invalid_time_sec;
    std::cout << "utf8::find_invalid," << duration_find_invalid.count() << "," << total_mb << "," << find_invalid_mbs << "," << invalid_index_sum << std::endl;

    return 0;
}