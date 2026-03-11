#pragma once
#include <array>
#include <cstdint>

#include "constants.hpp"
#include "fixed_math.hpp"
#include "log2ceil.hpp"

/** The half-space identifier.
 *
 * The applyFeedback step use this ID to locate the half plane normal vector and
 * offset scalar from the read-only memory. Compiler may inline the
 * coefficieints into the CPU instructions as immediate operands (IMM) if
 * possible.
 */
struct HalfspaceID {
    uint16_t value{};
};

/** The half plane identifier.
 *
 * The hyperplane::isRightOf step use this ID to locate the Scale matrix and
 * Offset vector from the read-only memory. Compiler may inline the coefficients
 * into the CPU instructions as immediate operands (IMM) if possible.
 */
struct FeedbackID {
    uint16_t value{};
};

/** A shorthand for invalid IDs. Used when the half plane binary search tree is
 * corrupt. */
constexpr FeedbackID invalid_feedback_id{0xFFFF};

namespace vector_math {

/** A generic vector. */
template <uint16_t size, typename T = DataFormat>
struct Vector {
    using value_type = T;
    std::array<T, size> data{};
};

}  // namespace vector_math

/** Input parameters to the PDA-QP algorithm. */
using Parameter = vector_math::Vector<n_parameter>;

/** Output solution from the PDA-QP algorithm. */
using SolutionType = std::conditional_t<                         //
    std::is_same_v<DataFormat, float>,                           //
    float, math::fixed<int16_t, 14 - log2ceil(n_parameter) - 1>  //
    >;
using Solution = vector_math::Vector<n_solution, SolutionType>;
