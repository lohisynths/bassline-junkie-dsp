/**
 * @file blit_oscillator.cpp
 * @brief Implementation of the BLIT-style bandlimited oscillator.
 */
#include "bassline_junkie_dsp/blit_oscillator.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <stdexcept>
#include <string>

namespace bassline_junkie::dsp {

namespace {

constexpr double kPi = 3.141592653589793238462643383279502884;
constexpr double kTwoPi = 2.0 * kPi;

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
    const double angle = kTwoPi * phase_;
    double sum = 0.0;

    for (std::size_t harmonic = 1; harmonic <= maxHarmonic; ++harmonic) {
        sum += std::sin(angle * static_cast<double>(harmonic)) / static_cast<double>(harmonic);
    }

    return (2.0 / kPi) * sum;
}

double BlitOscillator::processSquare() const noexcept
{
    const std::size_t maxHarmonic = harmonicLimit();
    const double angle = kTwoPi * phase_;
    double sum = 0.0;

    for (std::size_t harmonic = 1; harmonic <= maxHarmonic; harmonic += 2) {
        sum += std::sin(angle * static_cast<double>(harmonic)) / static_cast<double>(harmonic);
    }

    return (4.0 / kPi) * sum;
}

double BlitOscillator::processTriangle() const noexcept
{
    const std::size_t maxHarmonic = harmonicLimit();
    const double angle = kTwoPi * phase_;
    double sum = 0.0;
    bool positive = true;

    for (std::size_t harmonic = 1; harmonic <= maxHarmonic; harmonic += 2) {
        const double harmonicValue = static_cast<double>(harmonic);
        const double sign = positive ? 1.0 : -1.0;
        sum += sign * std::sin(angle * harmonicValue) / (harmonicValue * harmonicValue);
        positive = !positive;
    }

    return (8.0 / (kPi * kPi)) * sum;
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
