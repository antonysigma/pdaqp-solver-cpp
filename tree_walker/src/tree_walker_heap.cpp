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

enum type_t : uint8_t { HALFSPACE, FEEDBACK };

/** Stem or leaf node, packed to 16-bit struct.
 *
 * Binary representations:
 *
 * MSB               LSB
 *   **** **** **** ***0
 *   └─┬─────────────┘├┘
 *     id             t
 *     ^              ^ Boolean bit.
 *     ^              ^ True if the value is the FEEDBACK ID.
 *     ^ Feedback or halfspace ID (15-bit).
 */
class StemOrLeaf {
   public:
    constexpr StemOrLeaf() : raw{}, _type{} {}
    constexpr StemOrLeaf(FeedbackID id) : raw{id.value}, _type{FEEDBACK} {}
    constexpr StemOrLeaf(HalfspaceID id) : raw{id.value}, _type{HALFSPACE} {}

    constexpr uint16_t value() const { return raw; }

    constexpr type_t type() const { return _type; }

   private:
    uint16_t raw : 15;

    /** Putting the type bit at the LSB helps 8-bit MCUs to emit assembly code
     * `static_cast<uint8_t>(...) & 0b1`, which eliminates the clock cycles
     * spent on right shifting logic. */
    type_t _type : 1;
};

// using StemOrLeaf = std::variant<HalfspaceID, FeedbackID>;
static_assert(sizeof(StemOrLeaf) <= 2, "Too bulky");

/** The intermediate representation (IR) of the Eytzinger-style binary tree. */
template <size_t N>
requires(N < 65536 / 2) struct HeapDecisionTree {
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
        out.nodes[pos] = is_leaf ? StemOrLeaf{std::get<FeedbackID>(src_node)}
                                 : StemOrLeaf{std::get<JumpNode>(src_node).id};

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

constexpr auto tree{buildHeapTree(hyperplane::decodeJumpList())};

constexpr size_t n_hyperplanes = pdaqp_halfplanes.size() / (n_parameter + 1);
constexpr auto hyperplane_fn_list{
    hyperplane::makeHalfspaceList(std::make_index_sequence<n_hyperplanes>{})};

}  // namespace

FeedbackID
treeWalker(const Parameter parameter) {
    /** Eytzinger-style binary decision search algorithm.
     *
     * Compared to the default LUT algorithm based on hp_list and jump_list as
     * raw array, this heap reprentation enables the compiler to see the lower
     * bound and upper bound of the iteration loop. The search always walks to
     * the right. Equipped with these info, compiler can completely unroll the
     * iteration loop for small binary trees.
    */
    for (uint16_t pos = 1; pos < tree.nodes.size();) {
        const StemOrLeaf node = tree.nodes[pos];
        if (const bool is_leaf = node.value() == FEEDBACK; is_leaf) [[unlikely]] {
            return {node.value()};
        }

        const auto halfspace_id = node.value();
        const auto is_right_of_halfplane = hyperplane_fn_list[halfspace_id](parameter);
        pos = static_cast<uint16_t>(pos * 2 + is_right_of_halfplane);
    }

    return invalid_feedback_id;
}
