cmake_minimum_required(VERSION 3.22)

project(MathiSynth VERSION 1.0.1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build a universal binary on macOS (Apple Silicon + Intel) so it loads in any Ableton/FL build.
if(APPLE)
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "" FORCE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "" FORCE)
endif()

# Static MSVC runtime on Windows so the .vst3 has no VC++ redistributable dependency.
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# ---------------------------------------------------------------------------
# JUCE: use a local checkout if JUCE_DIR is given, otherwise fetch a pinned tag.
#   cmake -B build -DJUCE_DIR=/path/to/JUCE
# ---------------------------------------------------------------------------
set(JUCE_DIR "" CACHE PATH "Optional path to a local JUCE checkout")
if(JUCE_DIR)
    add_subdirectory(${JUCE_DIR} ${CMAKE_BINARY_DIR}/JUCE)
else()
    include(FetchContent)
    FetchContent_Declare(JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG        8.0.9
        GIT_SHALLOW    TRUE)
    FetchContent_MakeAvailable(JUCE)
endif()

# ---------------------------------------------------------------------------
# Plugin target
# ---------------------------------------------------------------------------
set(MATHISYNTH_FORMATS VST3 Standalone)
if(APPLE)
    list(APPEND MATHISYNTH_FORMATS AU)   # Ableton on macOS also accepts AU
endif()

juce_add_plugin(MathiSynth
    COMPANY_NAME                "Mathi Audio"
    PRODUCT_NAME                "MathiSynth"
    BUNDLE_ID                   "com.mathiaudio.mathisynth"
    PLUGIN_MANUFACTURER_CODE    Mthi
    PLUGIN_CODE                 MtSy
    IS_SYNTH                    TRUE
    NEEDS_MIDI_INPUT            TRUE
    NEEDS_MIDI_OUTPUT           FALSE
    IS_MIDI_EFFECT              FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    VST3_CATEGORIES             Instrument Synth
    AU_MAIN_TYPE                kAudioUnitType_MusicDevice
    COPY_PLUGIN_AFTER_BUILD     FALSE      # set TRUE (or pass -DMATHISYNTH_COPY=ON) to auto-install
    FORMATS                     ${MATHISYNTH_FORMATS})

option(MATHISYNTH_COPY "Copy the built plugin into the system plugin folder" OFF)
if(MATHISYNTH_COPY)
    set_target_properties(MathiSynth PROPERTIES JUCE_COPY_PLUGIN_AFTER_BUILD TRUE)
endif()

set(MATHISYNTH_SOURCES
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/Parameters.cpp
    Source/dsp/SynthEngine.cpp
    Source/dsp/Effects.cpp
    Source/presets/FactoryPresets.cpp
    Source/presets/PresetManager.cpp
    Source/ui/MathiLookAndFeel.cpp
    Source/ui/Section.cpp)

target_sources(MathiSynth PRIVATE ${MATHISYNTH_SOURCES})
target_include_directories(MathiSynth PRIVATE Source)

target_compile_definitions(MathiSynth PUBLIC
    MATHISYNTH_VERSION_STRING="${PROJECT_VERSION}"
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0)

target_link_libraries(MathiSynth
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

# ---------------------------------------------------------------------------
# Headless render test: renders every factory preset and checks the output.
#   cmake --build build --target MathiSynthTests && ./build/MathiSynthTests
# ---------------------------------------------------------------------------
option(MATHISYNTH_BUILD_TESTS "Build the offline preset render test" ON)
if(MATHISYNTH_BUILD_TESTS)
    juce_add_console_app(MathiSynthTests PRODUCT_NAME "MathiSynthTests")
    target_sources(MathiSynthTests PRIVATE tests/RenderTest.cpp ${MATHISYNTH_SOURCES})
    target_include_directories(MathiSynthTests PRIVATE Source)
    target_compile_definitions(MathiSynthTests PRIVATE
        MATHISYNTH_VERSION_STRING="${PROJECT_VERSION}"
        JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 MATHISYNTH_HEADLESS_TEST=1)
    target_link_libraries(MathiSynthTests PRIVATE
        juce::juce_audio_utils juce::juce_dsp
        juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)
endif()
