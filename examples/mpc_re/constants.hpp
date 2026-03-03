#pragma once
#include <cstdint>

#include "fixed_math.hpp"

constexpr uint16_t n_parameter = 6;
constexpr uint16_t n_solution = 13;

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

using DataFormat = math::fixed<int16_t, 14>;
using AccuDataFormat = math::fixed<int32_t, 14 * 2 - log2ceil(n_parameter* n_solution) - 1>;
// using DataFormat = float;
// using AccuDataFormat = float;