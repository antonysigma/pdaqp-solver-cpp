#pragma once
#include <cstdint>

#include "fixed_math.hpp"
#include "log2ceil.hpp"

constexpr uint16_t n_parameter = 2;
constexpr uint16_t n_solution = 3;

using DataFormat = math::fixed<int16_t, 14>;
using AccuDataFormat = math::fixed<int32_t, 14 * 2 - log2ceil(n_parameter* n_solution) - 1>;
