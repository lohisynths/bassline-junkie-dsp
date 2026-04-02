# Bassline Junkie DSP

Bassline Junkie DSP is a small C++17 audio DSP library built with CMake. It focuses on classic bandlimited oscillator generation for subtractive synthesis workflows and ships with a simple WAV rendering example.

## What It Does

- Provides a BLIT-style oscillator for `saw`, `square`, and `triangle` waveforms
- Uses Nyquist-limited harmonic synthesis to reduce aliasing
- Includes a compact example that renders a frequency sweep to a WAV file
- Keeps the codebase modern and easy to extend

## Build

```bash
cmake -S . -B build -G "Unix Makefiles"
cmake --build build
```

The default build creates:

- `bassline_junkie_dsp`: the library target
- `render_sweep`: the example executable

## Example

Render a logarithmic sweep from 50 Hz to 20 kHz:

```bash
./build/render_sweep
```

You can also choose the output path and waveform:

```bash
./build/render_sweep sweep.wav saw
./build/render_sweep sweep.wav square
./build/render_sweep sweep.wav triangle
```

The example writes a 12 second, mono, 48 kHz, 16-bit PCM WAV file.

## Library Overview

The public API lives in `include/bassline_junkie_dsp/blit_oscillator.hpp`.

Core entry points:

- `bassline_junkie::dsp::BlitOscillator`
- `bassline_junkie::dsp::Waveform`
- `bassline_junkie::dsp::waveformFromString()`
- `bassline_junkie::dsp::waveformToString()`

## Project Layout

- `include/` public headers
- `src/` library implementation
- `examples/` the WAV rendering example
- `CMakeLists.txt` top-level build system

## Notes

This is a compact reference implementation, not a full synth engine. If you want to extend it next, typical follow-ups are parameter smoothing, stereo output, envelopes, filters, and a faster harmonic summation path.
