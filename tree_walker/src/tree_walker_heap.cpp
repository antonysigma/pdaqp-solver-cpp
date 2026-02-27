#include <algorithm>
#include <cmath>
#include <tuple>
#include <utility>
#include <variant>

#include "hyperplane_math.hpp"
#include "tree_walker.h"
#include "fixed_queue.hpp"

namespace {

using vector_math::Vector;
using nonstd::FixedQueue;

using Fn = bool (*)(const Parameter&);
using StemOrLeaf = std::variant<Fn, FeedbackID>;

/** The intermediate representation (IR) of the Eytzinger-style binary tree. */
template <size_t N>
struct HeapDecisionTree {
    std::array<StemOrLeaf, N + 1> nodes{};
};

/** The queued node, to be converted to Eytzinger-style binary tree. */
struct WorkItem {
    uint16_t heap_pos;
    uint16_t orig_id;
};

/** Walk along the pdaqp_jump_list to convert the binary-search tree to
 * Eytzinger-style.
 *
 * Walk the 1D array "pdaqp_jump_list" to explore all branches of the binary
 * tree. Sort all stem or leaf nodes such that the binary search algorithm
 * always walk from left to right along the 1D array, i.e. a Eytzinger-style
 * heap.
 */
template <size_t n_tree_nodes>
consteval HeapDecisionTree<n_tree_nodes>
buildHeapTree(std::array<hyperplane::JumpOrFeedback, n_tree_nodes> decoded_jump_list) {
    using hyperplane::JumpNode;
    static_assert(pdaqp_halfplanes.size() % (n_parameter + 1) == 0);
    constexpr size_t n_hyperplanes = pdaqp_halfplanes.size() / (n_parameter + 1);
    constexpr auto hyperplane_fn_list{hyperplane::makeHalfspaceList(std::make_index_sequence<n_hyperplanes>{})};

    HeapDecisionTree<n_tree_nodes> out{};

    // init
    FixedQueue<WorkItem, n_tree_nodes> q{};
    q.push_back(WorkItem{.heap_pos = 1, .orig_id = 0});

    while (!q.empty()) {
        const auto [pos, id] = q.pop_front();

        if (pos > n_tree_nodes) continue;
        if (id >= n_tree_nodes) continue;

        const auto& src_node = decoded_jump_list[id];
        const bool is_leaf = std::holds_alternative<FeedbackID>(src_node);
        out.nodes[pos] =
            is_leaf ? StemOrLeaf{std::get<FeedbackID>(src_node)}
                    : StemOrLeaf{std::in_place_type<Fn>,
                                 hyperplane_fn_list[std::get<JumpNode>(src_node).id.value]};

        if (!is_leaf) {
            const auto jump = std::get<JumpNode>(src_node).jump;
            const uint16_t left_id = uint16_t(id + jump);
            const uint16_t right_id = uint16_t(left_id + 1);

            q.push_back(WorkItem{.heap_pos = uint16_t(pos * 2), .orig_id = left_id});
            q.push_back(WorkItem{.heap_pos = uint16_t(pos * 2 + 1), .orig_id = right_id});
        }
    }

    return out;
}

}  // namespace

FeedbackID
treeWalker(const Parameter parameter) {
    constexpr auto tree{buildHeapTree(hyperplane::decodeJumpList())};

    /** Eytzinger-style binary decision search algorithm.
     *
     * Compared to the default LUT algorithm based on hp_list and jump_list as
     * raw array, this heap reprentation enables the compiler to see the lower
     * bound and upper bound of the iteration loop. The search always walks to
     * the right. Equipped with these info, compiler can completely unroll the
     * iteration loop for small binary trees.
    */
    for (uint16_t pos = 1; pos < tree.nodes.size();) {
        const auto& node = tree.nodes[pos];
        const bool is_leaf = std::holds_alternative<FeedbackID>(node);
        if (is_leaf) {
            return std::get<FeedbackID>(node);
        }

        const auto is_right_of_halfplane = std::get<Fn>(tree.nodes[pos])(parameter);
        pos = static_cast<uint16_t>(pos * 2 + is_right_of_halfplane);
    }

    return invalid_feedback_id;
}
