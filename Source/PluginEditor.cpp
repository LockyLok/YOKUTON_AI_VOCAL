#include "PluginEditor.h"

namespace
{
    const juce::Colour bg (0xff32172f), panel (0xff54284f), pink (0xffff7bdc),
                       hot (0xffffb4ec), pale (0xffffe5f7), violet (0xffb75bcb);
}

void YokutonAudioProcessorEditor::NeonLook::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h,
    float pos, float start, float end, juce::Slider&)
{
    auto b = juce::Rectangle<float> ((float)x + 7, (float)y + 7, (float)w - 14, (float)h - 14);
    auto c = b.getCentre();
    const float r = juce::jmin (b.getWidth(), b.getHeight()) * 0.43f;
    g.setColour (juce::Colour (0x55300c30));
    g.fillEllipse (b);
    g.setColour (juce::Colour (0xff7c376f));
    g.drawEllipse (b, 2.0f);
    juce::Path arc;
    arc.addCentredArc (c.x, c.y, r, r, 0.0f, start, end, true);
    g.setColour (pink.withAlpha (0.25f));
    g.strokePath (arc, juce::PathStrokeType (7.0f));
    juce::Path value;
    value.addCentredArc (c.x, c.y, r, r, 0.0f, start, start + pos * (end - start), true);
    g.setColour (pink);
    g.strokePath (value, juce::PathStrokeType (5.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    const float a = start + pos * (end - start);
    juce::Point<float> p (c.x + std::cos (a) * r, c.y + std::sin (a) * r);
    g.setColour (pale);
    g.fillEllipse (p.x - 4, p.y - 4, 8, 8);
}

YokutonAudioProcessorEditor::YokutonAudioProcessorEditor (YokutonAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    setLookAndFeel (&look);
    setSize (900, 620);
    auto setupCombo = [this] (juce::ComboBox& c) { addAndMakeVisible (c); c.setColour (juce::ComboBox::backgroundColourId, panel); c.setColour (juce::ComboBox::textColourId, pale); c.setColour (juce::ComboBox::outlineColourId, pink); };
    setupCombo (character); character.addItem ("KAWAII", 1); character.addItem ("HYPERPOP", 2); character.addItem ("IDOL", 3); character.addItem ("CYBER ANIME", 4);
    setupCombo (key); for (int i=0;i<12;++i) key.addItem (juce::StringArray{"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"}[i], i+1);
    setupCombo (scale); scale.addItem ("CHROMATIC",1); scale.addItem ("MAJOR",2); scale.addItem ("MINOR",3);

    auto setupSlider = [this] (juce::Slider& s, juce::Label& l, const juce::String& name) {
        addAndMakeVisible (s);
        s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        // Use one explicit 270-degree sweep so the arc, pointer and value move together.
        s.setRotaryParameters (juce::MathConstants<float>::pi * 1.25f,
                              juce::MathConstants<float>::pi * 2.75f, true);
        s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 70, 20);
        s.setColour (juce::Slider::textBoxTextColourId, pale);
        s.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        addAndMakeVisible (l); l.setText (name, juce::dontSendNotification);
        l.setJustificationType (juce::Justification::centred); l.setColour (juce::Label::textColourId, pale);
    };
    setupSlider (speed, speedLabel, "SPEED");
    setupSlider (amount, amountLabel, "AMOUNT");
    setupSlider (formant, formantLabel, "FORMANT");
    setupSlider (sustain, sustainLabel, "SUSTAIN");
    setupSlider (pitch, pitchLabel, "PITCH");
    setupSlider (mix, mixLabel, "MIX");
    addAndMakeVisible (bypass);
    bypass.setColour (juce::ToggleButton::textColourId, pale);
    auto& a = processor.parameters;
    characterAttach = std::make_unique<ComboAttach> (a, "mode", character);
    keyAttach = std::make_unique<ComboAttach> (a, "key", key);
    scaleAttach = std::make_unique<ComboAttach> (a, "scale", scale);
    speedAttach = std::make_unique<SliderAttach> (a, "speed", speed);
    amountAttach = std::make_unique<SliderAttach> (a, "amount", amount);
    formantAttach = std::make_unique<SliderAttach> (a, "formant", formant);
    sustainAttach = std::make_unique<SliderAttach> (a, "sustain", sustain);
    pitchAttach = std::make_unique<SliderAttach> (a, "pitch", pitch);
    mixAttach = std::make_unique<SliderAttach> (a, "mix", mix);
    bypassAttach = std::make_unique<ButtonAttach> (a, "bypass", bypass);
}

void YokutonAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (bg);
    auto bounds = getLocalBounds().toFloat();
    juce::ColourGradient grad (juce::Colour (0xffa23d91), 0, 0, juce::Colour (0xff35152f), 900, 620, false);
    g.setGradientFill (grad); g.fillRect (bounds);
    g.setColour (juce::Colour (0x33ffffff));
    for (int i=0;i<22;++i) {
        const float x = (float)((i*137)%900), y = (float)((i*83)%620);
        g.drawEllipse (x, y, 12 + (i%4)*4, 12 + (i%4)*4, 1.0f);
    }
    g.setColour (panel.withAlpha (0.88f)); g.fillRoundedRectangle (18, 18, 864, 584, 22);
    g.setColour (pink); g.drawRoundedRectangle (18, 18, 864, 584, 22, 2.0f);
    g.setColour (pale); g.setFont (juce::Font (juce::FontOptions (27.0f, juce::Font::bold)));
    g.drawText ("YOKUTON", 42, 28, 250, 38, juce::Justification::centredLeft);
    g.setFont (juce::Font (juce::FontOptions (12.0f)));
    g.drawText ("AI VOCAL // ANIME CHARACTER ENGINE", 44, 65, 320, 20, juce::Justification::centredLeft);
    g.setFont (juce::Font (juce::FontOptions (13.0f, juce::Font::bold)));
    g.drawText ("♡  NEON VOCAL SYSTEM  ♡", 570, 39, 270, 24, juce::Justification::centredRight);

    // Central energy orb inspired by the reference, drawn as original vector artwork.
    auto orb = juce::Rectangle<float> (300, 125, 255, 255);
    g.setColour (pink.withAlpha (0.14f)); g.fillEllipse (orb.expanded (12));
    g.setColour (pink.withAlpha (0.5f)); g.drawEllipse (orb, 7.0f);
    g.setColour (hot.withAlpha (0.22f)); g.fillEllipse (orb.reduced (32));
    g.setColour (pale); g.drawEllipse (orb.reduced (55), 3.0f);
    g.setColour (pink); g.setFont (juce::Font (juce::FontOptions (64.0f, juce::Font::bold)));
    g.drawText ("ϟ", orb.toNearestInt(), juce::Justification::centred);
    g.setColour (pale.withAlpha (0.8f)); g.setFont (juce::Font (juce::FontOptions (12.0f)));
    g.drawText ("VOICE CORE // LIVE", 300, 390, 255, 22, juce::Justification::centred);

    g.setColour (pink.withAlpha (0.65f)); g.fillRoundedRectangle (35, 440, 830, 1.5f, 1.0f);
    g.setColour (pale); g.setFont (juce::Font (juce::FontOptions (12.0f)));
    g.drawText ("KEY & SCALE", 44, 493, 110, 20, juce::Justification::centredLeft);
    g.drawText ("CHARACTER PRESETS  //  01 — 04", 44, 112, 230, 20, juce::Justification::centredLeft);
    g.setColour (pink.withAlpha (0.15f)); g.fillRoundedRectangle (32, 468, 836, 112, 14);
    g.setColour (pink); g.drawRoundedRectangle (32, 468, 836, 112, 14, 1.0f);
    g.setColour (pale); g.setFont (juce::Font (juce::FontOptions (10.0f)));
    g.drawText ("PITCH CORRECTION  •  FORMANT COLOUR  •  CHARACTER SHAPING", 330, 575, 520, 14, juce::Justification::centredRight);
}

void YokutonAudioProcessorEditor::resized()
{
    character.setBounds (44, 138, 220, 34);
    speed.setBounds (570, 130, 135, 135); speedLabel.setBounds (570, 105, 135, 22);
    amount.setBounds (720, 130, 135, 135); amountLabel.setBounds (720, 105, 135, 22);
    formant.setBounds (570, 280, 135, 135); formantLabel.setBounds (570, 258, 135, 22);
    sustain.setBounds (720, 280, 135, 135); sustainLabel.setBounds (720, 258, 135, 22);
    key.setBounds (160, 500, 100, 30); scale.setBounds (270, 500, 145, 30);
    pitch.setBounds (430, 480, 100, 90); pitchLabel.setBounds (430, 468, 100, 18);
    mix.setBounds (545, 480, 100, 90); mixLabel.setBounds (545, 468, 100, 18);
    bypass.setBounds (680, 500, 160, 30);
}

YokutonAudioProcessorEditor::~YokutonAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}
