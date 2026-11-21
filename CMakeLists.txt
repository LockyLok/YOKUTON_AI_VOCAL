
cmake_minimum_required(VERSION 3.22)
project(YOKUTON_AI_VOCAL VERSION 0.1.0)

include(FetchContent)

FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.6
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(JUCE)

juce_add_plugin(YOKUTON
    COMPANY_NAME "YOKUTON"
    BUNDLE_ID "audio.yokuton.aivocal"
    PLUGIN_MANUFACTURER_CODE Yoku
    PLUGIN_CODE Yk01
    FORMATS VST3 Standalone
    PRODUCT_NAME "YOKUTON AI VOCAL"
    DESCRIPTION "Anime-style vocal pitch and character effect"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    VST3_CATEGORIES Fx Pitch\ Shift
)

target_sources(YOKUTON PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
)

# Generate the JUCE umbrella header used by PluginProcessor and PluginEditor
juce_generate_juce_header(YOKUTON)

juce_add_binary_data(YOKUTON_Assets
    SOURCES
        Resources/yokuton_anime_bg.png
)
target_link_libraries(YOKUTON PRIVATE YOKUTON_Assets)

target_compile_features(YOKUTON PRIVATE cxx_std_17)

target_compile_definitions(YOKUTON PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(YOKUTON
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
