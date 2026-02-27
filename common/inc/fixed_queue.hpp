#pragma once
#include <array>
#include <cstdint>

namespace nonstd {
/** A lightweight implementation of std::ring_span.
 *
 * Reference: https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/p0059r4.pdf
*/
template <typename T, size_t Capacity>
struct FixedQueue {
    std::array<T, Capacity> buf{};
    size_t head = 0;
    size_t tail = 0;

    constexpr bool empty() const { return head == tail; }
    constexpr size_t size() const { return tail - head; }

    constexpr void push_back(T&& v) {
        // assume caller never exceeds capacity
        buf[tail++] = v;
    }

    constexpr T pop_front() { return buf[head++]; }
};

}