#include <algorithm>
#include <cmath>
#include <tuple>
#include <utility>

#include "hyperplane_math.hpp"
#include "tree_walker.h"

namespace {

using vector_math::Vector;

/** Runtime implementation of hyperplane::isInsideHalfplane.
 *
 * Given a runtime value hp_id, explicitly seek the corresponding compile-time
 * function hyperplane::isInsideHalfplane<> ordered by HalfSpaceID.
 *
 * The compiler generates the function pointer table to evaluate the
 * corresponding half-space constraint logic. Dispatch cost is O(1).
 */
bool
isInsideHalfspaceImpl(const uint16_t hp_id, const Parameter& p) {
    static_assert(pdaqp_halfplanes.size() % (n_parameter + 1) == 0);
    constexpr auto n_hyperplanes = pdaqp_halfplanes.size() / (n_parameter + 1);
    constexpr auto hyperplane_list{hyperplane::makeHalfspaceList(std::make_index_sequence<n_hyperplanes>{})};

    return hyperplane_list[hp_id](p);
}

}  // namespace

FeedbackID
treeWalker(const Parameter parameter) {
    uint16_t id = 0;
    uint16_t next_id = pdaqp_jump_list.front();

    /** CVXPyGen's default implementation of binary decision tree. */
    while (id != next_id) {
        id = next_id + isInsideHalfspaceImpl(pdaqp_hp_list[id], parameter);
        next_id = id + pdaqp_jump_list[id];
    }
    return {pdaqp_hp_list[id]};
}
