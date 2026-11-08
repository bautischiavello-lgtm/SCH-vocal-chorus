#include "PixelLookAndFeel.h"
#include "PixelArt.h"

using namespace pixel;

PixelLookAndFeel::PixelLookAndFeel()
{
    setColour (juce::PopupMenu::backgroundColourId,            palette::canvasPink);
    setColour (juce::PopupMenu::textColourId,                  palette::black);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, palette::black);
    setColour (juce::PopupMenu::highlightedTextColourId,       palette::white);
    setColour (juce::ComboBox::textColourId,                   juce::Colours::transparentBlack);
}

void PixelLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h,
                                         float pos, float startAngle, float endAngle,
                                         juce::Slider&)
{
    const int size = juce::jmin (w, h);
    const float cx = (float) x + (float) w * 0.5f;
    const float cy = (float) y + (float) h * 0.5f;
    const float outerR = (float) size * 0.5f;
    const float discR  = outerR - 10.0f;

    // knob disc, built from 4x4 pixels
    for (int gy = (int) (cy - discR) / ps * ps; gy <= (int) (cy + discR); gy += ps)
        for (int gx = (int) (cx - discR) / ps * ps; gx <= (int) (cx + discR); gx += ps)
        {
            const float dx = (float) gx + ps * 0.5f - cx;
            const float dy = (float) gy + ps * 0.5f - cy;
            const float d  = std::sqrt (dx * dx + dy * dy);
            if (d > discR) continue;

            juce::Colour c = palette::black;
            if (d < discR - ps)
                c = (dx + dy < -discR * 0.5f) ? palette::white : palette::hotPink;   // highlight
            g.setColour (c);
            g.fillRect (gx, gy, ps, ps);
        }

    // notch ring: 11 ticks, the ones up to the current value light up
    const int numTicks = 11;
    for (int i = 0; i < numTicks; ++i)
    {
        const float t = (float) i / (float) (numTicks - 1);
        const float a = startAngle + t * (endAngle - startAngle);
        const float r = outerR - 3.0f;
        const int px = (int) std::round ((cx + std::sin (a) * r) / 2.0f) * 2 - 2;
        const int py = (int) std::round ((cy - std::cos (a) * r) / 2.0f) * 2 - 2;
        g.setColour (palette::black);
        g.fillRect (px - 1, py - 1, 6, 6);
        g.setColour (t <= pos + 0.001f ? palette::hotPink : palette::white);
        g.fillRect (px, py, 4, 4);
    }

    // pixel indicator line from centre
    const float ang = startAngle + pos * (endAngle - startAngle);
    for (float r = 4.0f; r <= discR - 4.0f; r += (float) ps)
    {
        const int px = (int) std::round ((cx + std::sin (ang) * r) / ps) * ps - ps / 2;
        const int py = (int) std::round ((cy - std::cos (ang) * r) / ps) * ps - ps / 2;
        g.setColour (palette::black);
        g.fillRect (px, py, ps, ps);
    }
    g.setColour (palette::black);
    g.fillRect ((int) cx - 4, (int) cy - 4, 8, 8);
}

void PixelLookAndFeel::drawLinearSlider (juce::Graphics& g, int x, int y, int w, int h,
                                         float sliderPos, float, float,
                                         juce::Slider::SliderStyle, juce::Slider&)
{
    const int trackH = 22;
    const auto track = juce::Rectangle<int> (x, y + (h - trackH) / 2, w, trackH);

    drawBox (g, track, palette::white);

    const int fillW = juce::jmax (0, (int) sliderPos - track.getX() - 2);
    g.setColour (palette::hotPink);
    g.fillRect (track.getX() + 2, track.getY() + 2, fillW, trackH - 4);

    // pixel stripes on the filled part
    g.setColour (palette::hotPink.brighter (0.25f));
    for (int sx = track.getX() + 4; sx < track.getX() + 2 + fillW; sx += 8)
        g.fillRect (sx, track.getY() + 4, 2, 4);

    // handle
    const int hw = 16;
    const auto handle = juce::Rectangle<int> ((int) sliderPos - hw / 2, y + 1, hw, h - 2);
    drawBox (g, handle, palette::hotPink);
    g.setColour (palette::white);
    g.fillRect (handle.getX() + 4, handle.getY() + 4, 4, handle.getHeight() - 8);
}

void PixelLookAndFeel::drawComboBox (juce::Graphics& g, int w, int h, bool down,
                                     int, int, int, int, juce::ComboBox& box)
{
    const auto r = juce::Rectangle<int> (0, 0, w, h);
    drawBox (g, r, down ? palette::softPink : palette::white);

    const int scale = 3;
    drawText (g, box.getText(), 12, (h - 7 * scale) / 2, scale, palette::black);

    // arrow button with pixel triangle
    const auto btn = juce::Rectangle<int> (w - h, 0, h, h);
    drawBox (g, btn, palette::hotPink);
    g.setColour (palette::black);
    const int cx = btn.getCentreX(), cy = btn.getCentreY() - 2;
    g.fillRect (cx - 8, cy - 2, 16, 4);
    g.fillRect (cx - 6, cy + 2, 12, 4);
    g.fillRect (cx - 4, cy + 6, 8, 4);
    g.fillRect (cx - 2, cy + 10, 4, 4);
}

void PixelLookAndFeel::positionComboBoxText (juce::ComboBox&, juce::Label& label)
{
    label.setBounds (0, 0, 0, 0); // text is painted by drawComboBox
}

void PixelLookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& b,
                                         bool highlighted, bool)
{
    const int boxSize = 28;
    const auto area = b.getLocalBounds();
    const auto box  = juce::Rectangle<int> (area.getX() + 2, area.getCentreY() - boxSize / 2,
                                            boxSize, boxSize);

    drawBox (g, box, highlighted ? palette::softPink : palette::white);

    if (b.getToggleState())
    {
        g.setColour (palette::hotPink);
        g.fillRect (box.reduced (6));
        g.setColour (palette::black);
        for (int i = 0; i < 4; ++i)   // pixel X
        {
            g.fillRect (box.getX() + 6 + i * 4, box.getY() + 6 + i * 4, 4, 4);
            g.fillRect (box.getRight() - 10 - i * 4, box.getY() + 6 + i * 4, 4, 4);
        }
    }

    const int scale = 3;
    drawText (g, b.getButtonText(), box.getRight() + 14,
              area.getCentreY() - (7 * scale) / 2, scale, palette::black);
}
