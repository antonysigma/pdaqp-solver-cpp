#include <fmt/format.h>
#include <fmt/ranges.h>

#include <cstring>

#include "apply_feedback.h"
#include "constants.hpp"
#include "problem-def.hpp"
#include "tree_walker.h"
#include "fixed_math.hpp"

/** Teach fmtlib how to print fixed point decimals. */
template <typename T, uint16_t Q>
struct fmt::formatter<math::fixed<T, Q>> : formatter<std::string_view> {
    constexpr auto format(const math::fixed<T, Q>& p, fmt::format_context& ctx) const {
        // Format the Point object into the output buffer.
        // We use format_to to write to the context's output iterator.
        // return fmt::format_to(ctx.out(), "{}.{}", (p.data >> Q), (p.data & (1UL << Q)));
        return fmt::format_to(ctx.out(), "{}", static_cast<float>(p));
    }
};

/** A demo of PDA-QP solver.
 *
 * Usage: ./pda-qp <<ascii-encoded parameter array, in little-endian>>
 *
 * Given the string argument representing the binary parameter array, decoded
 * the values in little-endian representation. Walk along the binary decision
 * tree to determine the half-space within constraint. Perform affine transform
 * to solve the quadratic programming problem.
*/
int
main(const int argc, const char** argv) {
    if (argc != 2 || std::strlen(argv[1]) < sizeof(Parameter)) {
        return 1;
    }

    const Parameter parameter = *reinterpret_cast<const Parameter*>(argv[1]);

    const auto feedback_id = treeWalker(parameter);
    const auto solution = applyFeedback<true>(feedback_id, std::move(parameter));

    fmt::print("Solution: {}\n", solution.data);
    return 0;
}
