/**
 * @file fast_trig.hpp
 * @brief Fast trigonometric helpers used by the oscillator implementation.
 */
#pragma once

#include <cmath>

namespace bassline_junkie::dsp::fast_trig {

inline constexpr double kPi = 3.141592653589793238462643383279502884;
inline constexpr double kTwoPi = 2.0 * kPi;
inline constexpr double kHalfPi = 0.5 * kPi;

struct SinCos {
    double sin;
    double cos;
};

[[nodiscard]] inline SinCos evaluateSinCos(double x) noexcept
{
    const double x2 = x * x;
    const double x4 = x2 * x2;
    const double x6 = x4 * x2;
    const double x8 = x4 * x4;
    const double x10 = x8 * x2;

    const double sine = x * (
        1.0
        - x2 * (1.0 / 6.0)
        + x4 * (1.0 / 120.0)
        - x6 * (1.0 / 5040.0)
        + x8 * (1.0 / 362880.0)
        - x10 * (1.0 / 39916800.0)
    );

    const double cosine = (
        1.0
        - x2 * (1.0 / 2.0)
        + x4 * (1.0 / 24.0)
        - x6 * (1.0 / 720.0)
        + x8 * (1.0 / 40320.0)
        - x10 * (1.0 / 3628800.0)
    );

    return {sine, cosine};
}

[[nodiscard]] inline SinCos fastSinCos(double x) noexcept
{
    // Reduce to [-pi, pi] first, then fold to [-pi/2, pi/2] where the
    // polynomial is accurate enough for oscillator harmonics.
    x = std::remainder(x, kTwoPi);

    if (x > kHalfPi) {
        x = kPi - x;
    } else if (x < -kHalfPi) {
        x = -kPi - x;
    }

    return evaluateSinCos(x);
}

[[nodiscard]] inline SinCos fastSinCosPhase(double angle) noexcept
{
    // `angle` is known to be in [0, 2pi), so a cheap wrap is enough.
    if (angle > kPi) {
        angle -= kTwoPi;
    }

    if (angle > kHalfPi) {
        angle = kPi - angle;
    } else if (angle < -kHalfPi) {
        angle = -kPi - angle;
    }

    return evaluateSinCos(angle);
}

[[nodiscard]] inline double fastSin(double x) noexcept
{
    return fastSinCos(x).sin;
}

} // namespace bassline_junkie::dsp::fast_trig
