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
}  // namespace pdaqp_solver