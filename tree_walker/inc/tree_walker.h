#pragma once
#include "types.hpp"

namespace pdaqp_solver {
/** Query the feedback ID from a pre-computed binary decision tree.
 *
 * Given a parameter vector, walk the binary decision tree. Each node in the
 * binary tree determines if the paramter is left of or right of the hyperplane.
 *
 * Once the leaf node is reached, return the corresponding feedback ID.
 */
FeedbackID treeWalker(const Parameter);

/** Mock tree walker.
 *
 * Given a 32-bit binary sequence, walk the binary tree's left/right branches in accordance to the
 * corresponding bit in the sequence. Return the halfplane ID when the leaf node is reached.
 */
FeedbackID treeWalkerMocked(uint32_t walk_trajectory_preset);
}  // namespace pdaqp_solver
