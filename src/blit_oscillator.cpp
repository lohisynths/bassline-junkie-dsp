/**
 * @file blit_oscillator.cpp
 * @brief Implementation of the BLIT-style bandlimited oscillator.
 */
#include "bassline_junkie_dsp/blit_oscillator.hpp"
#include "bassline_junkie_dsp/fast_trig.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <stdexcept>
#include <string>

namespace bassline_junkie::dsp {

namespace {

[[nodiscard]] std::string lowercase(std::string_view text)
{
    std::string result{text};
    std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return result;
}

} // namespace

BlitOscillator::BlitOscillator(double sampleRate, Waveform waveform)
    : sampleRate_(0.0)
    , frequency_(0.0)
    , waveform_(waveform)
    , phase_(0.0)
{
    setSampleRate(sampleRate);
}

void BlitOscillator::setSampleRate(double sampleRate)
{
    if (sampleRate <= 0.0) {
        throw std::invalid_argument("sampleRate must be greater than zero");
    }

    sampleRate_ = sampleRate;
    setFrequency(frequency_);
}

void BlitOscillator::setFrequency(double frequency) noexcept
{
    if (sampleRate_ <= 0.0) {
        frequency_ = 0.0;
        return;
    }

    frequency_ = std::clamp(frequency, 0.0, 0.5 * sampleRate_);
}

void BlitOscillator::setWaveform(Waveform waveform) noexcept
{
    waveform_ = waveform;
}

double BlitOscillator::sampleRate() const noexcept
{
    return sampleRate_;
}

double BlitOscillator::frequency() const noexcept
{
    return frequency_;
}

Waveform BlitOscillator::waveform() const noexcept
{
    return waveform_;
}

void BlitOscillator::reset(double phase) noexcept
{
    phase_ = phase - std::floor(phase);
    if (phase_ < 0.0) {
        phase_ += 1.0;
    }
}

double BlitOscillator::processSample()
{
    if (sampleRate_ <= 0.0 || frequency_ <= 0.0) {
        return 0.0;
    }

    const double sample = [&]() {
        switch (waveform_) {
        case Waveform::Saw:
            return processSaw();
        case Waveform::Square:
            return processSquare();
        case Waveform::Triangle:
            return processTriangle();
        }

        return 0.0;
    }();

    phase_ += frequency_ / sampleRate_;
    phase_ -= std::floor(phase_);
    return sample;
}

std::size_t BlitOscillator::harmonicLimit() const noexcept
{
    if (frequency_ <= 0.0) {
        return 1U;
    }

    const double nyquist = 0.5 * sampleRate_;
    return std::max<std::size_t>(1U, static_cast<std::size_t>(std::floor(nyquist / frequency_)));
}

double BlitOscillator::processSaw() const noexcept
{
    const std::size_t maxHarmonic = harmonicLimit();
    const double angle = fast_trig::kTwoPi * phase_;
    double sum = 0.0;

    for (std::size_t harmonic = 1; harmonic <= maxHarmonic; ++harmonic) {
        const double harmonicValue = static_cast<double>(harmonic);
        sum += fast_trig::fastSin(angle * harmonicValue) / harmonicValue;
    }

    return (2.0 / fast_trig::kPi) * sum;
}

double BlitOscillator::processSquare() const noexcept
{
    const std::size_t maxHarmonic = harmonicLimit();
    const double angle = fast_trig::kTwoPi * phase_;
    const fast_trig::SinCos base = fast_trig::fastSinCosPhase(angle);
    const double stepSine = 2.0 * base.sin * base.cos;
    const double stepCosine = base.cos * base.cos - base.sin * base.sin;

    double sine = base.sin;
    double cosine = base.cos;
    double sum = sine;

    for (std::size_t harmonic = 3; harmonic <= maxHarmonic; harmonic += 2) {
        const double nextSine = sine * stepCosine + cosine * stepSine;
        const double nextCosine = cosine * stepCosine - sine * stepSine;
        sine = nextSine;
        cosine = nextCosine;
        sum += sine / static_cast<double>(harmonic);
    }

    return (4.0 / fast_trig::kPi) * sum;
}

double BlitOscillator::processTriangle() const noexcept
{
    const std::size_t maxHarmonic = harmonicLimit();
    const double angle = fast_trig::kTwoPi * phase_;
    const fast_trig::SinCos base = fast_trig::fastSinCosPhase(angle);
    const double stepSine = 2.0 * base.sin * base.cos;
    const double stepCosine = base.cos * base.cos - base.sin * base.sin;

    double sine = base.sin;
    double cosine = base.cos;
    double sum = sine;
    double sign = 1.0;

    for (std::size_t harmonic = 3; harmonic <= maxHarmonic; harmonic += 2) {
        const double nextSine = sine * stepCosine + cosine * stepSine;
        const double nextCosine = cosine * stepCosine - sine * stepSine;
        sine = nextSine;
        cosine = nextCosine;
        sign = -sign;
        const double harmonicValue = static_cast<double>(harmonic);
        sum += sign * sine / (harmonicValue * harmonicValue);
    }

    return (8.0 / (fast_trig::kPi * fast_trig::kPi)) * sum;
}

constexpr std::string_view BlitOscillator::waveformName(Waveform waveform) noexcept
{
    switch (waveform) {
    case Waveform::Saw:
        return "saw";
    case Waveform::Square:
        return "square";
    case Waveform::Triangle:
        return "triangle";
    }

    return "unknown";
}

Waveform waveformFromString(std::string_view name)
{
    const std::string normalized = lowercase(name);

    if (normalized == "saw") {
        return Waveform::Saw;
    }

    if (normalized == "square") {
        return Waveform::Square;
    }

    if (normalized == "triangle" || normalized == "tri") {
        return Waveform::Triangle;
    }

    throw std::invalid_argument("unsupported waveform label: " + normalized);
}

std::string_view waveformToString(Waveform waveform) noexcept
{
    return BlitOscillator::waveformName(waveform);
}

} // namespace bassline_junkie::dsp
