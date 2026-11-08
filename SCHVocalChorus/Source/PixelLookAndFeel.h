#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

// Custom retro pixel-art LookAndFeel: rotary knobs, horizontal slider,
// combo box and toggle button, all drawn on a pixel grid.
class PixelLookAndFeel : public juce::LookAndFeel_V4
{
public:
    PixelLookAndFeel();

    void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height,
                           float sliderPosProportional, float rotaryStartAngle,
                           float rotaryEndAngle, juce::Slider&) override;

    void drawLinearSlider (juce::Graphics&, int x, int y, int width, int height,
                           float sliderPos, float minSliderPos, float maxSliderPos,
                           juce::Slider::SliderStyle, juce::Slider&) override;

    void drawComboBox (juce::Graphics&, int width, int height, bool isButtonDown,
                       int buttonX, int buttonY, int buttonW, int buttonH,
                       juce::ComboBox&) override;

    void positionComboBoxText (juce::ComboBox&, juce::Label&) override;

    void drawToggleButton (juce::Graphics&, juce::ToggleButton&,
                           bool shouldDrawButtonAsHighlighted,
                           bool shouldDrawButtonAsDown) override;

private:
    static constexpr int ps = 4; // pixel size of the knob grid
};
