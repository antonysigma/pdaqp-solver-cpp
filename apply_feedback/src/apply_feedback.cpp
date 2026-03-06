#include "apply_feedback.h"

#include <algorithm>
// #include <armadillo>
#include <cmath>

#include "problem-def.hpp"

namespace {

template <uint16_t n_rows, uint16_t n_cols, typename T = DataFormat>
struct Mat {
    using value_type = T;
    std::array<T, n_rows * n_cols> data{};
};

using vector_math::Vector;

template <uint16_t M, uint16_t N, bool using_arma = false>
auto
matrixMultiplyAdd(const Mat<M, N>& a_v, const Vector<N>& b_v, const Vector<M>& c_v) {
    using Decimal = std::conditional_t<                                        //
        std::is_same_v<decltype(a_v.data.front() * b_v.data.front()), float>,  //
        float, AccuDataFormat>;
    Vector<M, Decimal> result{};

#ifdef ARMA_INCLUDES
    if constexpr (using_arma) {
        static_assert(std::is_same_v<Decimal, float>);
        const arma::fmat a{const_cast<float*>(a_v.data.data()), M, N, false, true};
        const arma::fcolvec b{const_cast<float*>(b_v.data.data()), N, false, true};
        const arma::fcolvec c{const_cast<float*>(c_v.data.data()), M, false, true};
        arma::fcolvec res{result.data.data(), M, false, true};
        res = a * b + c;
    } else {
#endif
        for (uint32_t i = 0; i < M; ++i) {
            result.data[i] = Decimal{c_v.data[i]};
        }

        for (uint32_t i = 0; i < M; ++i) {
            for (uint32_t j = 0; j < N; ++j) {
                result.data[i] += a_v.data[i * N + j] * b_v.data[j];
            }
        }
#ifdef ARMA_INCLUDES
    }
#endif
    return result;
}

template <size_t feedback_id>
consteval Mat<n_solution, n_parameter>
feedbackScale() {
    Mat<n_solution, n_parameter> v;
    using Decimal = Mat<n_solution, n_parameter>::value_type;

    const auto offset = feedback_id * (n_parameter + 1) * n_solution;
    constexpr auto stride = n_parameter + 1;
    for (size_t i = 0; i < n_solution; i++) {
        std::transform(pdaqp_feedbacks.begin() + offset + stride * i,
                       pdaqp_feedbacks.begin() + offset + stride * i + n_parameter,
                       v.data.begin() + n_parameter * i,
                       [](const float& item) { return Decimal{item}; });
    }
    return v;
}

template <size_t feedback_id>
consteval Vector<n_solution>
feedbackOffset() {
    Vector<n_solution> v;
    using T = Vector<n_solution>::value_type;

    const auto offset = feedback_id * (n_parameter + 1) * n_solution;
    constexpr auto stride = n_parameter + 1;
    for (size_t i = 0; i < n_solution; i++) {
        v.data[i] = T{pdaqp_feedbacks[offset + stride * i + n_parameter]};
    }
    return v;
}

template <size_t... Is>
consteval auto
makeScaleList(std::index_sequence<Is...>) {
    return std::array{feedbackScale<Is>()...};
}

template <size_t... Is>
consteval auto
makeOffsetList(std::index_sequence<Is...>) {
    return std::array{feedbackOffset<Is>()...};
}

namespace pdaqp_solver_internal {

template <size_t feedback_id>
auto
applyFeedbackFn(const Parameter& p) {
    constexpr auto scale{feedbackScale<feedback_id>()};
    constexpr auto offset{feedbackOffset<feedback_id>()};

    return matrixMultiplyAdd(scale, p, offset);
}

template <size_t... Is>
consteval auto
makeFeedbackFnList(std::index_sequence<Is...>) {
    return std::array{applyFeedbackFn<Is>...};
}

constexpr size_t n_feedbacks = pdaqp_feedbacks.size() / (n_parameter + 1) / n_solution;
constexpr auto scale_list{makeScaleList(std::make_index_sequence<n_feedbacks>{})};
constexpr auto offset_list{makeOffsetList(std::make_index_sequence<n_feedbacks>{})};

}  // namespace pdaqp_solver_internal

}  // namespace

namespace pdaqp_solver {
template <bool force_function_table>
Solution
applyFeedback(const FeedbackID id, const Parameter p) {
    using pdaqp_solver_internal::n_feedbacks;
    using pdaqp_solver_internal::offset_list;
    using pdaqp_solver_internal::scale_list;

    constexpr auto feedback_function_list{
        pdaqp_solver_internal::makeFeedbackFnList(std::make_index_sequence<n_feedbacks>{})};
    const auto id_clamped = std::min(n_feedbacks - 1, size_t(id.value));

    const auto resultFn = [&, id_clamped, p]() {
        if constexpr (force_function_table) {
            return feedback_function_list[id_clamped](p);
        } else {
            return matrixMultiplyAdd(scale_list[id_clamped], p, offset_list[id_clamped]);
        }
    };

    using Decimal = Solution::value_type;
    using IntermediateDecimal =
        std::invoke_result_t<decltype(pdaqp_solver_internal::applyFeedbackFn<0>),
                             const Parameter&>::value_type;

    if constexpr (std::is_same_v<Decimal, float> || std::is_same_v<Decimal, IntermediateDecimal>) {
        return resultFn();
    } else {
        Solution solution;

        const auto result = resultFn();
        std::transform(result.data.begin(), result.data.end(), solution.data.begin(),
                       [](const auto& x) { return Decimal::truncateFrom(x); });
        return solution;
    }
}

template Solution applyFeedback<true>(const FeedbackID, const Parameter);
template Solution applyFeedback<false>(const FeedbackID, const Parameter);
}  // namespace pdaqp_solver
