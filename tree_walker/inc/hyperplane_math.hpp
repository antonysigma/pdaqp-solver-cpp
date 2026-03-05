#include <algorithm>
#include <cmath>
#include <utility>
#include <variant>

#include "problem-def.hpp"
#include "types.hpp"
#include "vector-math.hpp"

/** Hyperplane maths. */
namespace hyperplane {

using vector_math::dot;
using vector_math::Vector;

/** Fixed point for floating point decimal. */
using Decimal = Vector<n_parameter>::value_type;

/** Decode the hyperplane normal vectors.
 *
 * @tparam hp_id hyperplane ID
 *
 * Given a hyperplane ID at compile-time, extract all values in single-precision
 * floating point. If fixed point math is specified, convert all values to
 * fixed-point at compile time.
 */
template <uint16_t hp_id>
consteval Vector<n_parameter>
hyperplaneNormal() {
    Vector<n_parameter> v;

    const auto offset = hp_id * (n_parameter + 1);
    std::transform(pdaqp_halfplanes.begin() + offset,
                   pdaqp_halfplanes.begin() + offset + n_parameter, v.data.begin(),
                   [](const float& item) { return Decimal{item}; });
    return v;
}

/** Decode the hyperplane offset scalar values.
 *
 * @tparam hp_id hyperplane ID
 *
 * Given a hyperplane ID at compile-time, extract the scalar value in
 * single-precision floating point. If fixed point math is specified, convert
 * all it to fixed-point at compile time.
 */
template <uint16_t hp_id>
consteval auto
hyperplaneOffset() {
    return Decimal{pdaqp_halfplanes[hp_id * (n_parameter + 1) + n_parameter]};
}

/** Evaluate the hyperplane constraint.
 *
 * Given the hyperplane ID at compile-time, decode a constant hyperplane normal
 * vector and offset at compile time. The compiler will either move it to the
 * read-only data (.rodata) section of the executable. Or, for fixed-point
 * maths, the compiler may inject it to the CPU/MCU instructions as immediate
 * operands (IMM) to eliminate the RODATA traffic.
*/
template <uint16_t hp_id>
struct isInsideHalfspace {
    bool operator()(const Vector<n_parameter> parameter) const {
        constexpr auto normal{hyperplaneNormal<hp_id>()};
        constexpr auto offset{hyperplaneOffset<hp_id>()};

        return dot(normal, parameter) <= offset;
    }
};

/** A branching node in a generic binary decision tree. */
struct JumpNode {
    uint16_t jump{};
    HalfspaceID id{};
};

/** A node can be a stem or a leaf. In this case, a jump or the feedback ID. */
using JumpOrFeedback = std::variant<JumpNode, FeedbackID>;

/** Decode the CVXGen's raw pdaqp_hp_list on demand.
 *
 * It seems that the raw array `pdaqp_hp_list` is a mix of feedback IDs and
 * hyperplane IDs. Decode the list so we can tell the node from a Stem or Leaf
 * by the C++ type.
 *
 * This compile-time function is primarily used by tree_walker::buildHeapTree()
 * , and the intermediate representation (IR) is discarded on the fly. The
 * compiler/linker option `-gc-sections` may also eliminate `pdaqp_hp_list` and
 * `pdaqp_jump_list` from the executable on the fly.
 */
consteval auto
decodeJumpList() {
    std::array<JumpOrFeedback, pdaqp_jump_list.size()> decoded;

    for (uint16_t idx = 0; idx < decoded.size(); idx++) {
        if (pdaqp_jump_list[idx] == 0) {
            decoded[idx] = JumpOrFeedback{std::in_place_type<FeedbackID>, pdaqp_hp_list[idx]};
        } else {
            decoded[idx] = JumpOrFeedback{std::in_place_type<JumpNode>, pdaqp_jump_list[idx],
                                          HalfspaceID{pdaqp_hp_list[idx]}};
        }
    }
    return decoded;
}

/** Generate a hard-coded decision function.
 *
 * Given the half_plane::isInstideHalfspace function and a hyperplane ID at
 * compile time, generate a hardcoded function. The compiler can hardcode the
 * address of the normal vector from the read-only memory of the executable
 * (RODATA).
 *
 * For fixed-point maths, the compiler may directly inject the normal vector
 * coefficients into the CPU/MCU instructions as immediate operands (IMM),
 * eliminating RODATA traffic.
*/
template <size_t hp_id>
bool
isInsideHalfspaceFn(const Parameter& p) {
    return hyperplane::isInsideHalfspace<hp_id>{}(p);
};


/** Generate a look up table (LUT) of half-spaces. */
template <size_t... Is>
consteval auto
makeHalfspaceList(std::index_sequence<Is...>) {
    return std::array{hyperplane::isInsideHalfspaceFn<Is>...};
}

}  // namespace hyperplane
