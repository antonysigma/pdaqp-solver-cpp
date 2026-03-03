#pragma once
#include <cstdint>

constexpr size_t
log2ceil(size_t n) {
    if (n <= 1) return 0;
    size_t count = 0;
    size_t val = 1;
    while (val < n) {
        val <<= 1;
        count++;
    }
    return count;
}
