#pragma once
#include "types.hpp"

/** The main entry point of the PDA-QP solver. */
template <bool force_feedback_fn_list>
Solution pdaqpSolver(Parameter parameter);
