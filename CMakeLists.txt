cmake_minimum_required(VERSION 3.22)
project(DDIEGO_Y2K_FX VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(JUCE_SOURCE_DIR "" CACHE PATH "Optional path to a JUCE 8 source checkout")
if(JUCE_SOURCE_DIR)
    if(NOT EXISTS "${JUCE_SOURCE_DIR}/CMakeLists.txt")
        message(FATAL_ERROR "JUCE_SOURCE_DIR must point to a JUCE source checkout.")
    endif()
    add_subdirectory("${JUCE_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/JUCE")
else()
    include(FetchContent)
    FetchContent_Declare(
        JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 8.0.12
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(JUCE)
endif()

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

juce_generate_juce_header(DDIEGO_Y2K_FX)

target_sources(DDIEGO_Y2K_FX PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/GranularEngine.cpp
    Source/ChopperEngine.cpp
    Source/SampleData.h
)

target_include_directories(DDIEGO_Y2K_FX PRIVATE Source)
target_compile_features(DDIEGO_Y2K_FX PRIVATE cxx_std_17)

target_compile_definitions(DDIEGO_Y2K_FX PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_USE_MP3AUDIOFORMAT=1
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_REPORT_APP_USAGE=0
)

target_link_libraries(DDIEGO_Y2K_FX PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_dsp
    juce::juce_gui_extra
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags
)
