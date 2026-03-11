#include <algorithm>
#include <cmath>
#include <tuple>
#include <utility>

#include "hyperplane_math.hpp"
#include "tree_walker.h"

namespace {

using vector_math::Vector;

namespace pdaqp_solver_internal {
static_assert(pdaqp_halfplanes.size() % (n_parameter + 1) == 0);
constexpr auto n_hyperplanes = pdaqp_halfplanes.size() / (n_parameter + 1);
constexpr auto hyperplane_fn_list{
    hyperplane::makeHalfspaceList(std::make_index_sequence<n_hyperplanes>{})};

}  // namespace pdaqp_solver_internal
}  // namespace

namespace pdaqp_solver {
FeedbackID
treeWalker(const Parameter parameter) {
    uint16_t id = 0;
    uint16_t next_id = pdaqp_jump_list.front();

    /** CVXPyGen's default implementation of binary decision tree. */
    while (id != next_id) {
        const auto hp_id = pdaqp_hp_list[id];
        id = next_id + pdaqp_solver_internal::hyperplane_fn_list[hp_id](parameter);
        next_id = id + pdaqp_jump_list[id];
    }
    return {pdaqp_hp_list[id]};
}

FeedbackID
treeWalkerMocked(uint32_t walk_trajectory_preset) {
    uint16_t id = 0;
    uint16_t next_id = pdaqp_jump_list.front();

    while (id != next_id) {
        const auto hp_id = pdaqp_hp_list[id];
        id = next_id + (walk_trajectory_preset & 0x1);
        next_id = id + pdaqp_jump_list[id];
        walk_trajectory_preset >>= 1;
    }
    return {pdaqp_hp_list[id]};
}
}  // namespace pdaqp_solver
