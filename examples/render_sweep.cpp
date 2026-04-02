/**
 * @file render_sweep.cpp
 * @brief Example executable that renders a frequency sweep to a WAV file.
 */
#include "bassline_junkie_dsp/blit_oscillator.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

void writeU16(std::ostream& stream, std::uint16_t value)
{
    const unsigned char bytes[2] = {
        static_cast<unsigned char>(value & 0xFFU),
        static_cast<unsigned char>((value >> 8U) & 0xFFU)
    };
    stream.write(reinterpret_cast<const char*>(bytes), sizeof(bytes));
}

void writeU32(std::ostream& stream, std::uint32_t value)
{
    const unsigned char bytes[4] = {
        static_cast<unsigned char>(value & 0xFFU),
        static_cast<unsigned char>((value >> 8U) & 0xFFU),
        static_cast<unsigned char>((value >> 16U) & 0xFFU),
        static_cast<unsigned char>((value >> 24U) & 0xFFU)
    };
    stream.write(reinterpret_cast<const char*>(bytes), sizeof(bytes));
}

void writePcm16Mono(const std::filesystem::path& path, const std::vector<float>& samples, std::uint32_t sampleRate)
{
    std::ofstream output(path, std::ios::binary);
    if (!output) {
        throw std::runtime_error("failed to open output file: " + path.string());
    }

    constexpr std::uint16_t channels = 1U;
    constexpr std::uint16_t bitsPerSample = 16U;
    constexpr std::uint16_t audioFormat = 1U;
    const std::uint16_t blockAlign = static_cast<std::uint16_t>(channels * bitsPerSample / 8U);
    const std::uint32_t byteRate = sampleRate * blockAlign;
    const std::uint32_t dataSize = static_cast<std::uint32_t>(samples.size() * sizeof(std::int16_t));
    const std::uint32_t riffSize = 36U + dataSize;

    output.write("RIFF", 4);
    writeU32(output, riffSize);
    output.write("WAVE", 4);

    output.write("fmt ", 4);
    writeU32(output, 16U);
    writeU16(output, audioFormat);
    writeU16(output, channels);
    writeU32(output, sampleRate);
    writeU32(output, byteRate);
    writeU16(output, blockAlign);
    writeU16(output, bitsPerSample);

    output.write("data", 4);
    writeU32(output, dataSize);

    for (float sample : samples) {
        const float clamped = std::clamp(sample, -1.0F, 1.0F);
        const auto scaled = static_cast<std::int16_t>(std::lround(clamped * 32767.0F));
        writeU16(output, static_cast<std::uint16_t>(scaled));
    }

    if (!output) {
        throw std::runtime_error("failed while writing WAV data: " + path.string());
    }
}

} // namespace

int main(int argc, char** argv)
{
    try {
        const std::filesystem::path outputPath = argc > 1 ? std::filesystem::path{argv[1]} : std::filesystem::path{"blit_sweep.wav"};
        const bassline_junkie::dsp::Waveform waveform = argc > 2
            ? bassline_junkie::dsp::waveformFromString(argv[2])
            : bassline_junkie::dsp::Waveform::Saw;

        constexpr std::uint32_t sampleRate = 48'000U;
        constexpr double durationSeconds = 12.0;
        constexpr double startFrequency = 50.0;
        constexpr double endFrequency = 20'000.0;
        constexpr double outputGain = 0.90;

        const std::size_t totalSamples = static_cast<std::size_t>(std::llround(durationSeconds * static_cast<double>(sampleRate)));
        std::vector<float> samples;
        samples.reserve(totalSamples);

        bassline_junkie::dsp::BlitOscillator oscillator(static_cast<double>(sampleRate), waveform);

        for (std::size_t index = 0; index < totalSamples; ++index) {
            const double progress = totalSamples > 1 ? static_cast<double>(index) / static_cast<double>(totalSamples - 1U) : 1.0;
            const double frequency = startFrequency * std::pow(endFrequency / startFrequency, progress);
            oscillator.setFrequency(frequency);
            samples.push_back(static_cast<float>(oscillator.processSample() * outputGain));
        }

        writePcm16Mono(outputPath, samples, sampleRate);

        std::cout << "Rendered " << outputPath.string()
                  << " using " << bassline_junkie::dsp::waveformToString(waveform)
                  << " sweep from " << startFrequency << " Hz to " << endFrequency << " Hz\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "render_sweep failed: " << error.what() << '\n';
        return 1;
    }
}

