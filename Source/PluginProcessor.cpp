#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
    constexpr float pi = 3.14159265358979323846f;
    float readParam (const juce::AudioProcessorValueTreeState& s, const char* id)
    {
        return s.getRawParameterValue (id)->load();
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout YokutonAudioProcessor::createParameterLayout()
{
    using namespace juce;
    AudioProcessorValueTreeState::ParameterLayout p;
    p.add (std::make_unique<AudioParameterChoice> ("mode", "Character",
        StringArray { "Kawaii", "Hyperpop", "Idol", "Cyber Anime" }, 0));
    p.add (std::make_unique<AudioParameterChoice> ("key", "Key",
        StringArray { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" }, 0));
    p.add (std::make_unique<AudioParameterChoice> ("scale", "Scale",
        StringArray { "Chromatic", "Major", "Minor" }, 1));
    p.add (std::make_unique<AudioParameterFloat> ("speed", "Speed",
        NormalisableRange<float> (0.0f, 100.0f, 0.1f), 82.0f));
    p.add (std::make_unique<AudioParameterFloat> ("amount", "Amount",
        NormalisableRange<float> (0.0f, 100.0f, 0.1f), 100.0f));
    p.add (std::make_unique<AudioParameterFloat> ("formant", "Formant",
        NormalisableRange<float> (-12.0f, 12.0f, 0.1f), 4.0f));
    p.add (std::make_unique<AudioParameterFloat> ("sustain", "Sustain",
        NormalisableRange<float> (0.0f, 100.0f, 0.1f), 35.0f));
    p.add (std::make_unique<AudioParameterFloat> ("pitch", "Pitch",
        NormalisableRange<float> (-12.0f, 12.0f, 0.1f), 0.0f));
    p.add (std::make_unique<AudioParameterFloat> ("mix", "Mix",
        NormalisableRange<float> (0.0f, 100.0f, 0.1f), 100.0f));
    p.add (std::make_unique<AudioParameterBool> ("bypass", "Global Bypass", false));
    return p;
}

YokutonAudioProcessor::YokutonAudioProcessor()
    : AudioProcessor (BusesProperties().withInput ("Input", juce::AudioChannelSet::stereo(), true)
                                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      parameters (*this, nullptr, "YOKUTON_STATE", createParameterLayout())
{
}

void YokutonAudioProcessor::Shifter::prepare (int)
{
    ring.assign (8192, 0.0f);
    reset();
}
void YokutonAudioProcessor::Shifter::reset()
{
    std::fill (ring.begin(), ring.end(), 0.0f);
    writePos = 0;
    phase = 0.0;
}
float YokutonAudioProcessor::Shifter::process (float input, float ratio)
{
    if (ring.empty()) return input;
    ring[(size_t) writePos] = input;
    ratio = juce::jlimit (0.5f, 2.0f, ratio);
    if (std::abs (ratio - 1.0f) < 0.002f)
    {
        writePos = (writePos + 1) % (int) ring.size();
        return input;
    }

    const float shift = ratio - 1.0f;
    phase += std::abs (shift) / (float) grain;
    if (phase >= 1.0) phase -= 1.0;
    const float p1 = (float) phase;
    const float p2 = std::fmod (p1 + 0.5f, 1.0f);
    const float w1 = 0.5f - 0.5f * std::cos (2.0f * pi * p1);
    const float w2 = 1.0f - w1;

    auto readAt = [this] (float offset)
    {
        const float pos = (float) writePos - (float) baseDelay - offset * (float) grain;
        const int size = (int) ring.size();
        float wrapped = std::fmod (pos, (float) size);
        if (wrapped < 0.0f) wrapped += (float) size;
        const int i0 = (int) wrapped;
        const int i1 = (i0 + 1) % size;
        const float frac = wrapped - (float) i0;
        return ring[(size_t) i0] * (1.0f - frac) + ring[(size_t) i1] * frac;
    };

    const float out = readAt (p1) * w1 + readAt (p2) * w2;
    writePos = (writePos + 1) % (int) ring.size();
    return out;
}

void YokutonAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    for (auto& s : shifters) s.prepare (samplesPerBlock);
    pitchWindow.fill (0.0f);
    pitchWrite = detectCounter = 0;
    detectedMidi = 60.0f;
}

bool YokutonAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto in = layouts.getMainInputChannelSet();
    const auto out = layouts.getMainOutputChannelSet();
    return in == out && (out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo());
}

float YokutonAudioProcessor::estimateMidiNote() const
{
    // Autocorrelation pitch estimate, suitable for a single prominent vocal note.
    const int n = (int) pitchWindow.size();
    float rms = 0.0f;
    for (float v : pitchWindow) rms += v * v;
    rms = std::sqrt (rms / (float) n);
    if (rms < 0.003f) return detectedMidi;

    const int minLag = juce::jmax (2, (int) (currentSampleRate / 1000.0));
    const int maxLag = juce::jmin (n / 2, (int) (currentSampleRate / 75.0));
    float best = 0.0f;
    int bestLag = 0;
    for (int lag = minLag; lag <= maxLag; ++lag)
    {
        float corr = 0.0f;
        for (int i = 0; i < n - lag; i += 3)
            corr += pitchWindow[(size_t) i] * pitchWindow[(size_t) (i + lag)];
        if (corr > best) { best = corr; bestLag = lag; }
    }
    if (bestLag == 0) return detectedMidi;
    const float hz = (float) currentSampleRate / (float) bestLag;
    return juce::jlimit (36.0f, 96.0f, 69.0f + 12.0f * std::log2 (hz / 440.0f));
}

float YokutonAudioProcessor::quantiseNote (float midi, int key, int scale) const
{
    static constexpr int major[] = { 0, 2, 4, 5, 7, 9, 11 };
    static constexpr int minor[] = { 0, 2, 3, 5, 7, 8, 10 };
    const int nearest = juce::roundToInt (midi);
    if (scale == 0) return (float) nearest;
    float bestNote = (float) nearest;
    float bestDistance = 1000.0f;
    for (int octave = -1; octave <= 1; ++octave)
        for (int i = 0; i < 7; ++i)
        {
            const int degree = scale == 1 ? major[i] : minor[i];
            const float candidate = (float) (60 + key + octave * 12 + degree);
            const float d = std::abs (midi - candidate);
            if (d < bestDistance) { bestDistance = d; bestNote = candidate; }
        }
    return bestNote;
}

void YokutonAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int channels = getTotalNumInputChannels();
    const int samples = buffer.getNumSamples();
    for (int ch = getTotalNumInputChannels(); ch < getTotalNumOutputChannels(); ++ch)
        buffer.clear (ch, 0, samples);

    if (readParam (parameters, "bypass") > 0.5f) return;

    const int mode = (int) readParam (parameters, "mode");
    const int key = (int) readParam (parameters, "key");
    const int scale = (int) readParam (parameters, "scale");
    const float speed = readParam (parameters, "speed") / 100.0f;
    const float amount = readParam (parameters, "amount") / 100.0f;
    const float formant = readParam (parameters, "formant");
    const float pitch = readParam (parameters, "pitch");
    const float mix = readParam (parameters, "mix") / 100.0f;
    const float sustain = readParam (parameters, "sustain") / 100.0f;

    // Character presets set a musical starting point; all controls remain user-adjustable.
    const float modePitch = (mode == 0 ? 3.0f : mode == 1 ? 5.0f : mode == 2 ? 2.0f : 7.0f);
    const float formantBias = (mode == 0 ? 2.5f : mode == 1 ? 1.0f : mode == 2 ? 0.5f : -1.0f);
    const float totalPitch = pitch + modePitch * 0.35f;
    const float formantTone = juce::jlimit (-0.25f, 0.25f, (formant + formantBias) / 48.0f);
    const float attack = 0.02f + (1.0f - speed) * 0.30f;
    const float release = 0.08f + (1.0f - speed) * 0.45f;

    for (int i = 0; i < samples; ++i)
    {
        float mono = 0.0f;
        for (int ch = 0; ch < channels; ++ch) mono += buffer.getSample (ch, i);
        mono /= (float) juce::jmax (1, channels);
        pitchWindow[(size_t) pitchWrite] = mono;
        pitchWrite = (pitchWrite + 1) % (int) pitchWindow.size();
        if (++detectCounter >= 512)
        {
            detectCounter = 0;
            const float estimate = estimateMidiNote();
            // Sustain adds note-hold behaviour: downward note transitions settle more slowly.
            const float heldRelease = release * (1.0f - 0.65f * sustain);
            const float smoothing = estimate > detectedMidi ? attack : heldRelease;
            detectedMidi += (estimate - detectedMidi) * juce::jlimit (0.05f, 0.9f, smoothing);
        }

        const float target = quantiseNote (detectedMidi, key, scale) + totalPitch;
        const float correction = (target - detectedMidi) * amount;
        const float ratio = std::pow (2.0f, correction / 12.0f);
        for (int ch = 0; ch < channels; ++ch)
        {
            const float dry = buffer.getSample (ch, i);
            float wet = shifters[(size_t) juce::jmin (ch, 1)].process (dry, ratio);
            // Gentle spectral tilt approximates a brighter/darker character; this is not a
            // dedicated formant-preserving spectral processor.
            static thread_local std::array<float, 2> previous {};
            const float high = wet - previous[(size_t) ch];
            previous[(size_t) ch] = wet;
            wet = wet + high * formantTone;
            buffer.setSample (ch, i, dry * (1.0f - mix) + wet * mix);
        }
    }
}

void YokutonAudioProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, dest);
}
void YokutonAudioProcessor::setStateInformation (const void* data, int size)
{
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, size));
    if (xml != nullptr && xml->hasTagName (parameters.state.getType()))
        parameters.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessorEditor* YokutonAudioProcessor::createEditor()
{
    return new YokutonAudioProcessorEditor (*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new YokutonAudioProcessor();
}
