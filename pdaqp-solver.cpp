#include "pdaqp-solver.h"

#include "apply_feedback.h"
#include "constants.hpp"
#include "problem-def.hpp"
#include "tree_walker.h"

template <bool force_feedback_fn_list>
Solution
pdaqpSolver(Parameter parameter) {
    const auto feedback_id = pdaqp_solver::treeWalker(parameter);
    return pdaqp_solver::applyFeedback<force_feedback_fn_list>(feedback_id, std::move(parameter));
}

template Solution pdaqpSolver<true>(Parameter);
template Solution pdaqpSolver<false>(Parameter);
