#pragma once
#include <cmath>
#include <concepts>
#include <cstdint>

/** Fixed decimal math operations.
 *
 * This file implements common operations of fixed-point decimal aritmetics
 * (0.123_q214 * 0.456_q214 + (-0.435_q214)) via C++ template metaprogramming.
 *
 * Refer to C++ enhancement proposal at
 * https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p0037r6.html#Motivation
 */
namespace math {

template <std::signed_integral T = int16_t, size_t Q = 14>
    requires(sizeof(T) <= 4) && (Q < sizeof(T) * 8) struct fixed {
    using value_type = T;
    static constexpr uint8_t _Q = Q;
    T data{};

    /** In-place truncation from floating point number. */
    constexpr explicit fixed(float v) : data{static_cast<T>(v * (1UL << Q))} {}

    /** Direct memory copy from raw 16-bit buffer. */
    constexpr explicit fixed(T v) : data{v} {}

    /** Default to zero initialization. */
    constexpr fixed() : data{} {}

    /** Pad trailing zeros to the decimal number. */
    template <size_t Q2>
        requires std::is_same_v<T, int32_t> && (Q >= Q2) constexpr fixed(fixed<int32_t, Q2> v)
        : data{v.data << (Q - Q2)} {}

    /** Up-cast from 16-bit to 32-bit fixed-point decimal. */
    template <size_t Q2>
        requires std::is_same_v<T, int32_t> && (Q >= Q2) constexpr fixed(fixed<int16_t, Q2> v)
        : data{static_cast<T>(v.data) << (Q - Q2)} {}

    /** Cast to floating point number for logging to console. */
    constexpr explicit operator float() const { return static_cast<float>(data) / (1UL << Q); }
};

static_assert(static_cast<float>(fixed<>{0.125f}) == 0.125f);
static_assert(static_cast<float>(fixed<>{-0.125f}) == -0.125f);

static_assert(fixed<int32_t, 12>{fixed<int32_t, 4>{static_cast<int32_t>(0xBEEF)}}.data ==
              0x00BE'EF00);
static_assert(!std::is_same_v<fixed<>, float>);

static_assert(static_cast<float>(fixed<int32_t, 14>{fixed<int16_t, 8>{0.125f}}) == 0.125f);
static_assert(static_cast<float>(fixed<int32_t, 14>{fixed<int16_t, 8>{-0.125f}}) == -0.125f);

constexpr auto
operator""_q214(const long double v) {
    return fixed<int16_t, 14>{static_cast<float>(v)};
};

static_assert(static_cast<float>(0.125_q214) == 0.125f);

constexpr auto
operator""_q428(const long double v) {
    return fixed<int32_t, 28>{static_cast<float>(v)};
};

static_assert(static_cast<float>(0.125_q214) == 0.125f);

template <std::signed_integral T, size_t Q>
constexpr auto
operator-(fixed<T, Q> v) {
    v.data = -v.data;
    return v;
}

static_assert(static_cast<float>(-0.125_q214) == -0.125f);

template <size_t Q>
constexpr auto
operator*(const fixed<int16_t, Q>& a, const fixed<int16_t, Q>& b) {
    return fixed<int32_t, Q * 2>{static_cast<int32_t>(a.data) * b.data};
}

static_assert(static_cast<float>(-0.25_q214 * 0.5_q214) == -0.125f);

template <size_t Q2, size_t Q>
constexpr bool
operator<=(const fixed<int32_t, Q2>& a, const fixed<int16_t, Q>& b) {
    static_assert(Q2 == Q * 2);
    return a.data <= (static_cast<int32_t>(b.data) << Q);
}

static_assert(0.125_q428 <= 0.125_q214);
static_assert(0.124_q428 <= 0.125_q214);
static_assert(-0.125_q428 <= -0.123_q214);
static_assert(-0.125_q428 <= -0.125_q214);
static_assert(-0.125_q428 <= 0.123_q214);

template <typename T, size_t Q>
constexpr bool
operator==(const fixed<T, Q>& a, const fixed<T, Q>& b) {
    return a.data == b.data;
}

static_assert(0.125_q428 == 0.125_q428);
static_assert(0.314_q428 == 0.314_q428);

template <size_t Q2, size_t Q>
constexpr fixed<int32_t, Q2>&
operator+=(fixed<int32_t, Q2>& accu, const fixed<int32_t, Q>& b) {
    static_assert(Q2 <= Q);
    accu.data += b.data >> (Q - Q2);
    return accu;
}

static_assert([]() {
    auto accu = 0.1_q428;
    accu += -0.25_q214 * 0.5_q214;
    return accu;
}() == -0.025_q428);

}  // namespace math
