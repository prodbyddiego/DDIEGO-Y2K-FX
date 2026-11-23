cmake_minimum_required(VERSION 3.22)
project(DDIEGO_Y2K_FX VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.12
)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(DDIEGO_Y2K_FX
    COMPANY_NAME "DDIEGO"
    BUNDLE_ID "com.ddiego.y2kfx"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE DDie
    PLUGIN_CODE Y2K1
    FORMATS VST3
    PRODUCT_NAME "DDIEGO Y2K FX"
)

target_sources(DDIEGO_Y2K_FX PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/GranularEngine.cpp
    Source/ChopperEngine.cpp
)

target_include_directories(DDIEGO_Y2K_FX PRIVATE Source)

target_compile_definitions(DDIEGO_Y2K_FX PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
)

target_link_libraries(DDIEGO_Y2K_FX PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_audio_basics
    juce::juce_dsp
    juce::juce_gui_extra
)

if(MSVC)
    target_compile_options(DDIEGO_Y2K_FX PRIVATE /W3)
endif()
