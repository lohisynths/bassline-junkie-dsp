/**
 * @file blit_oscillator.hpp
 * @brief BLIT-style bandlimited oscillator for saw, square, and triangle waves.
 */
#pragma once

#include <cstddef>
#include <string_view>

namespace bassline_junkie::dsp {

/**
 * @brief Supported oscillator waveforms.
 */
enum class Waveform {
    Saw,
    Square,
    Triangle
};

/**
 * @brief A bandlimited oscillator for classic subtractive-synth waveforms.
 *
 * The implementation uses a harmonic budget derived from the Nyquist limit,
 * which keeps the rendered saw, square, and triangle waveforms from folding
 * harsh aliasing back into the audible band.
 */
class BlitOscillator {
public:
    /**
     * @brief Construct an oscillator.
     *
     * @param sampleRate The host sample rate in Hz.
     * @param waveform The waveform to generate.
     */
    explicit BlitOscillator(double sampleRate, Waveform waveform = Waveform::Saw);

    /**
     * @brief Update the sample rate.
     *
     * @param sampleRate The host sample rate in Hz.
     */
    void setSampleRate(double sampleRate);

    /**
     * @brief Update the oscillator frequency.
     *
     * @param frequency The target frequency in Hz.
     */
    void setFrequency(double frequency) noexcept;

    /**
     * @brief Update the waveform.
     *
     * @param waveform The target waveform.
     */
    void setWaveform(Waveform waveform) noexcept;

    /**
     * @brief Read the current sample rate.
     *
     * @return The sample rate in Hz.
     */
    [[nodiscard]] double sampleRate() const noexcept;

    /**
     * @brief Read the current oscillator frequency.
     *
     * @return The frequency in Hz.
     */
    [[nodiscard]] double frequency() const noexcept;

    /**
     * @brief Read the current waveform.
     *
     * @return The waveform type.
     */
    [[nodiscard]] Waveform waveform() const noexcept;

    /**
     * @brief Reset the oscillator phase.
     *
     * @param phase Normalized phase in the range [0, 1).
     */
    void reset(double phase = 0.0) noexcept;

    /**
     * @brief Generate the next sample.
     *
     * @return The next oscillator sample, normalized to roughly [-1, 1].
     */
    [[nodiscard]] double processSample();

    /**
     * @brief Convert a waveform to a stable string label.
     *
     * @param waveform The waveform value.
     * @return A string label suitable for logs and CLI output.
     */
    [[nodiscard]] static constexpr std::string_view waveformName(Waveform waveform) noexcept;

private:
    [[nodiscard]] std::size_t harmonicLimit() const noexcept;

    [[nodiscard]] double processSaw() const noexcept;

    [[nodiscard]] double processSquare() const noexcept;

    [[nodiscard]] double processTriangle() const noexcept;

    double sampleRate_;
    double frequency_;
    Waveform waveform_;
    double phase_;
};

/**
 * @brief Parse a waveform label.
 *
 * @param name The waveform label.
 * @return The matching waveform.
 * @throw std::invalid_argument If the label is not recognized.
 */
[[nodiscard]] Waveform waveformFromString(std::string_view name);

/**
 * @brief Convert a waveform to text.
 *
 * @param waveform The waveform value.
 * @return A lowercase label.
 */
[[nodiscard]] std::string_view waveformToString(Waveform waveform) noexcept;

} // namespace bassline_junkie::dsp

