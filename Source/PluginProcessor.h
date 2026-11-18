#pragma once
#include <JuceHeader.h>
#include <array>
#include <atomic>
#include <vector>

class YokutonAudioProcessor final : public juce::AudioProcessor
{
public:
    YokutonAudioProcessor();
    ~YokutonAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "YOKUTON AI VOCAL"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    juce::AudioProcessorValueTreeState parameters;
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

private:
    struct Shifter
    {
        void prepare (int maxSamples);
        float process (float input, float ratio);
        void reset();
        std::vector<float> ring;
        int writePos = 0;
        double phase = 0.0;
        static constexpr int grain = 1024;
        static constexpr int baseDelay = 1536;
    };

    std::array<Shifter, 2> shifters;
    double currentSampleRate = 44100.0;
    float detectedMidi = 60.0f;
    float envelope = 0.0f;
    int detectCounter = 0;
    std::array<float, 2048> pitchWindow {};
    int pitchWrite = 0;

    float estimateMidiNote() const;
    float quantiseNote (float midi, int key, int scale) const;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (YokutonAudioProcessor)
};
