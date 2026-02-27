#pragma once
#include <cmath>

#include "fixed_math.hpp"
#include "types.hpp"

/** Vector operations. */
namespace vector_math {

/** Vector-vector dot product.
 *
 * We expect the compiler to complete inline this function into the parent
 * function. Since Vector guarantees memory aligned data, the compiler can
 * perform auto-vectorization to generation SIMD instructions.
 */
template <uint16_t N, typename T>
inline auto
dot(const Vector<N, T>& a, const Vector<N, T>& b) {
    using U = decltype(a.data.front() * b.data.front());

    U sum{};
    for (uint16_t i = 0; i < N; i++) {
        sum += a.data[i] * b.data[i];
    }

    return sum;
}

}  // namespace vector_math
