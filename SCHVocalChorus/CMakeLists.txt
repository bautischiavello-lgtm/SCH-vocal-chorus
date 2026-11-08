cmake_minimum_required(VERSION 3.22)

project(SCHVocalChorus VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---------------------------------------------------------------------------
# JUCE: use a local checkout with -DSCH_JUCE_DIR=path/to/JUCE, otherwise fetch.
# ---------------------------------------------------------------------------
set(SCH_JUCE_DIR "" CACHE PATH "Path to a local JUCE checkout (optional)")

if(SCH_JUCE_DIR)
    add_subdirectory(${SCH_JUCE_DIR} JUCE)
else()
    include(FetchContent)
    FetchContent_Declare(JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG        8.0.4
        GIT_SHALLOW    TRUE)
    FetchContent_MakeAvailable(JUCE)
endif()

juce_add_plugin(SCHVocalChorus
    COMPANY_NAME                "SCH Audio"
    PLUGIN_MANUFACTURER_CODE    Schx
    PLUGIN_CODE                 Svch
    FORMATS                     VST3
    PRODUCT_NAME                "SCH Vocal Chorus"
    IS_SYNTH                    FALSE
    NEEDS_MIDI_INPUT            FALSE
    NEEDS_MIDI_OUTPUT           FALSE
    IS_MIDI_EFFECT              FALSE
    VST3_CATEGORIES             Fx Modulation
    COPY_PLUGIN_AFTER_BUILD     FALSE)

target_sources(SCHVocalChorus PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/PixelLookAndFeel.cpp)

target_include_directories(SCHVocalChorus PRIVATE Source)

target_compile_definitions(SCHVocalChorus PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0)

target_link_libraries(SCHVocalChorus
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_gui_basics
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
