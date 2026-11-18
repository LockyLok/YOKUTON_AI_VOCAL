#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class YokutonAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit YokutonAudioProcessorEditor (YokutonAudioProcessor&);
    ~YokutonAudioProcessorEditor(YokutonAudioProcessor&) override = default;
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    struct NeonLook final : juce::LookAndFeel_V4
    {
        void drawRotarySlider (juce::Graphics&, int, int, int, int, float, float, float,
                               juce::Slider&) override;
    };

    YokutonAudioProcessor& processor;
    NeonLook look;
    juce::ComboBox character, key, scale;
    juce::Slider speed, amount, formant, sustain, pitch, mix;
    juce::ToggleButton bypass { "GLOBAL BYPASS" };
    juce::Label title, subtitle, speedLabel, amountLabel, formantLabel, sustainLabel, pitchLabel, mixLabel;
    using ComboAttach = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using SliderAttach = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttach = juce::AudioProcessorValueTreeState::ButtonAttachment;
    std::unique_ptr<ComboAttach> characterAttach, keyAttach, scaleAttach;
    std::unique_ptr<SliderAttach> speedAttach, amountAttach, formantAttach, sustainAttach, pitchAttach, mixAttach;
    std::unique_ptr<ButtonAttach> bypassAttach;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (YokutonAudioProcessorEditor)
};
