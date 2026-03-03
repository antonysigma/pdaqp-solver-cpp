#pragma once
#include <cstdint>

#include "types.hpp"

namespace pdaqp_solver {
template <bool force_function_table>
Solution applyFeedback(const FeedbackID, const Parameter p);
}