cmake_minimum_required(VERSION 3.16)

project(BasslineJunkieDSP VERSION 0.1.0 LANGUAGES CXX)

option(BASSLINE_JUNKIE_DSP_BUILD_EXAMPLES "Build the example executable" ON)

add_library(bassline_junkie_dsp
    src/blit_oscillator.cpp
)

target_include_directories(bassline_junkie_dsp
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_features(bassline_junkie_dsp PUBLIC cxx_std_17)

if (MSVC)
    target_compile_options(bassline_junkie_dsp PRIVATE /W4)
else()
    target_compile_options(bassline_junkie_dsp PRIVATE -Wall -Wextra -Wpedantic)
endif()

if (BASSLINE_JUNKIE_DSP_BUILD_EXAMPLES)
    add_executable(render_sweep
        examples/render_sweep.cpp
    )

    target_link_libraries(render_sweep
        PRIVATE
            bassline_junkie_dsp
    )

    target_compile_features(render_sweep PRIVATE cxx_std_17)

    if (MSVC)
        target_compile_options(render_sweep PRIVATE /W4)
    else()
        target_compile_options(render_sweep PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endif()

