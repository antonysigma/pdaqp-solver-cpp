#include <algorithm>
#include <cmath>
#include <utility>

#include "hyperplane_math.hpp"
#include "tree_walker.h"

//
#include <boost/sml.hpp>

namespace {

using vector_math::Vector;

/** A stem node or leaf node in the binary tree. */
template <uint16_t tree_node_id>
struct Node {};

/** One iteration step. */
struct Tick {
    const Vector<n_parameter>& parameter;
};

constexpr size_t n_tree_nodes = pdaqp_jump_list.size();

/** Generate one row of the state transition table.
 *
 * @tparam row_id The row ID of the state transition table.
 *
 * Given the row_id, determine if we should walk to the left branch of the
 * binary tree, or the right branch. If left branch, ensure that the input
 * parameter is inside the corresponding Halfspace. Otherwise, walk to the right
 * branch.
 *
 * If the jump value of the pdaqp_jump_list is zero, terminate the finite state
 * machine by transitioning a terminal state. Also denote the feedback ID decoded from the
 */
template <size_t row_id>
consteval auto
makeTransition() {
    using hyperplane::isInsideHalfspace;
    constexpr uint16_t node_id = row_id / 2;
    constexpr bool is_right_of_binary_tree_event = (row_id % 2 == 0);

    constexpr auto decoded_jump_list{hyperplane::decodeJumpList()};

    // Actions

    using namespace boost::sml;
    auto FromState = state<Node<node_id>>;

    if constexpr (std::holds_alternative<FeedbackID>(decoded_jump_list[node_id])) {
        constexpr auto hp_id{std::get<FeedbackID>(decoded_jump_list[node_id])};
        constexpr auto outputHPId = [=](FeedbackID& result) { result = hp_id; };

        return FromState + event<Tick> / outputHPId = X;
    } else  {
        constexpr auto this_node = std::get<hyperplane::JumpNode>(decoded_jump_list[node_id]);
        constexpr uint16_t next_id = node_id + this_node.jump + is_right_of_binary_tree_event;
        auto ToState = state<Node<next_id>>;

        if constexpr (is_right_of_binary_tree_event) {
            if constexpr (row_id == 0) {
                return *FromState + event<Tick>[isInsideHalfspace<this_node.id.value>{}] = ToState;
            } else {
                return FromState + event<Tick>[isInsideHalfspace<this_node.id.value>{}] = ToState;
            }
        } else {
            static_assert(row_id != 0);
            return FromState + event<Tick> = ToState;
        }
    }
}

/** Compile the pdaqp_jump_list to a finite state machine (FSM).
 *
 * Explore all nodes of the binary decision trees. Map the stem node IDs to a
 * State "Node<id>". Connect all states with state transition events. Guard the
 * events with the event conditional hyperplane::isInsideHalfplane.
 *
 * Identify all leaf nodes, terminate the FSM with a terminal state. Report the
 * corresponding feedback ID.
 */
template <size_t... Is>
consteval auto
makeTransitionTable(std::index_sequence<Is...>) {
    using namespace boost::sml;
    return make_transition_table(makeTransition<Is>()...);
}

/** A finite state machine. */
struct StateMachine {
    auto operator()() {
        using namespace boost::sml;

        static_assert(n_tree_nodes <= 50);
        return makeTransitionTable(std::make_index_sequence<n_tree_nodes * 2>{});
    }
};

/** Implement the FSM with nested switch statements. */
using dispatch_t = boost::sml::dispatch<boost::sml::back::policies::switch_stm>;
}  // namespace

namespace pdaqp_solver {
FeedbackID
treeWalker(const Parameter parameter) {
    FeedbackID result{};

    /** Binary decision tree with state transition logic.
     *
     * Decode the pdaqp_jump_list to a table to state transition logic (SML),
     * where each table row corresponds to an edge connecting the binary tree
     * nodes.
     *
     * Generate the nested switch statements or if-else statements on the fly at
     * compile time. Switch statements is favored for now (LLVM 17) because it
     * can detect and deduplicate the tail call `dot(n, p) <= const`. The
     * compiler can in turn hardcode the corresponding hyperplane normal and
     * offsets for each branch, eliminating the memory address calculations at
     * runtime.
     *
     * For small binary decision trees, the compiler is able to see through the
     * isInsideHalfplane functions in all (CPU conditional) branches, and inline
     * all function calls to the main function. For CPU/MCUs with hardware
     * branch predictions, the hot tree traversal paths are prioritized in the
     * CPU instruction queue.
     *
     */
    constexpr auto Converged = boost::sml::X;
    for (boost::sml::sm<StateMachine, dispatch_t> state_machine{result};

         !state_machine.is(Converged);) {
        state_machine.process_event(Tick{parameter});
    }

    return result;
}
}  // namespace pdaqp_solver