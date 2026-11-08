#include "PluginEditor.h"
#include "PixelArt.h"

using namespace pixel;

namespace
{
constexpr int W = 800;
constexpr int H = 500;
constexpr int headerH = 40;

const juce::Rectangle<int> knobPanel   { 24, 216, 516, 156 };
const juce::Rectangle<int> modePanel   { 556, 216, 220, 156 };
const juce::Rectangle<int> amountPanel { 24, 388, 516, 84 };
const juce::Rectangle<int> infoPanel   { 556, 388, 220, 84 };

constexpr int knobCellW = 129;
constexpr int knobSize  = 84;

juce::Rectangle<int> knobBounds (int index)
{
    const int cellX = knobPanel.getX() + index * knobCellW;
    return { cellX + (knobCellW - knobSize) / 2, knobPanel.getY() + 36, knobSize, knobSize };
}
} // namespace

SCHVocalChorusAudioProcessorEditor::SCHVocalChorusAudioProcessorEditor (SCHVocalChorusAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    setLookAndFeel (&laf);
    setSize (W, H);

    auto setupKnob = [this] (juce::Slider& s)
    {
        s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        s.setRotaryParameters (juce::MathConstants<float>::pi * 1.2f,
                               juce::MathConstants<float>::pi * 2.8f, true);
        s.onValueChange = [this] { repaint(); };
        addAndMakeVisible (s);
    };
    for (auto* s : { &mixKnob, &detuneKnob, &delayKnob, &widthKnob })
        setupKnob (*s);

    amountSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    amountSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    amountSlider.onValueChange = [this] { repaint(); };
    addAndMakeVisible (amountSlider);

    modeBox.addItemList ({ "Classic", "Gospel", "Kids", "Shout", "Custom" }, 1);
    modeBox.onChange = [this] { repaint(); };
    addAndMakeVisible (modeBox);

    monoButton.onClick = [this] { repaint(); };
    addAndMakeVisible (monoButton);

    auto& vts = processor.apvts;
    mixAtt    = std::make_unique<SliderAtt> (vts, "mix",    mixKnob);
    detuneAtt = std::make_unique<SliderAtt> (vts, "detune", detuneKnob);
    delayAtt  = std::make_unique<SliderAtt> (vts, "delay",  delayKnob);
    widthAtt  = std::make_unique<SliderAtt> (vts, "width",  widthKnob);
    amountAtt = std::make_unique<SliderAtt> (vts, "amount", amountSlider);
    modeAtt   = std::make_unique<ComboAtt>  (vts, "mode",   modeBox);
    monoAtt   = std::make_unique<ButtonAtt> (vts, "mono",   monoButton);

    startTimerHz (4); // sparkle twinkle
}

SCHVocalChorusAudioProcessorEditor::~SCHVocalChorusAudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

void SCHVocalChorusAudioProcessorEditor::timerCallback()
{
    twinkle = (twinkle + 1) % 4;
    repaint (0, headerH, W, 180);
    repaint (infoPanel);
}

void SCHVocalChorusAudioProcessorEditor::resized()
{
    for (int i = 0; i < 4; ++i)
    {
        juce::Slider* s[] = { &mixKnob, &detuneKnob, &delayKnob, &widthKnob };
        s[i]->setBounds (knobBounds (i));
    }

    modeBox.setBounds (modePanel.getX() + 16, modePanel.getY() + 44, modePanel.getWidth() - 32, 36);
    monoButton.setBounds (modePanel.getX() + 16, modePanel.getY() + 96, modePanel.getWidth() - 32, 40);
    amountSlider.setBounds (amountPanel.getX() + 20, amountPanel.getY() + 38, amountPanel.getWidth() - 40, 34);
}

void SCHVocalChorusAudioProcessorEditor::paint (juce::Graphics& g)
{
    // ---- canvas + outer frame -------------------------------------------
    g.fillAll (palette::canvasPink);

    // subtle pixel checker in the canvas
    g.setColour (palette::softPink.withAlpha (0.35f));
    for (int y = headerH + 4; y < H - 4; y += 16)
        for (int x = 4 + ((y / 16) % 2) * 8; x < W - 4; x += 16)
            g.fillRect (x, y, 4, 4);

    // ---- header bar ---------------------------------------------------------
    g.setColour (palette::headerPink);
    g.fillRect (0, 0, W, headerH);
    g.setColour (palette::black);
    g.fillRect (0, headerH - 2, W, 2);

    // waveform icon (black square, white bars)
    const juce::Rectangle<int> icon (8, 6, 28, 28);
    g.fillRect (icon);
    static const int bars[] = { 6, 12, 18, 10, 16, 8, 4 };
    g.setColour (palette::white);
    for (int i = 0; i < 7; ++i)
        g.fillRect (icon.getX() + 3 + i * 3, icon.getCentreY() - bars[i] / 2, 2, bars[i]);

    drawText (g, "SCH VOCAL CHORUS.VST3", 48, 13, 2, palette::black);

    // window controls: minimise, maximise, close
    for (int i = 0; i < 3; ++i)
    {
        const juce::Rectangle<int> b (W - 3 * 32 - 8 + i * 32, 6, 28, 28);
        drawBox (g, b, palette::white);
        g.setColour (palette::black);
        if (i == 0)
            g.fillRect (b.getX() + 7, b.getBottom() - 11, 14, 3);
        else if (i == 1)
            g.drawRect (b.reduced (7), 2);
        else
            for (int k = 0; k < 4; ++k)
            {
                g.fillRect (b.getX() + 7 + k * 4, b.getY() + 7 + k * 4, 4, 4);
                g.fillRect (b.getRight() - 11 - k * 4, b.getY() + 7 + k * 4, 4, 4);
            }
    }

    // ---- logo ---------------------------------------------------------------
    const int logoScale = 14;
    const int logoW = measureText ("SCH", logoScale);
    const int logoX = (W - logoW) / 2;
    drawLogoText (g, "SCH", logoX, 62, logoScale, palette::hotPink, palette::black, 8);

    const juce::String sub = "- VOCAL CHORUS -";
    drawTextAligned (g, sub, 0, W, 176, 3, palette::black, Align::centre);

    // avatars flanking the logo
    drawAvatar (g, 70, 52, 8, false);
    drawAvatar (g, W - 70 - 16 * 8, 52, 8, true);

    // sparkles around the logo (twinkle between sizes)
    struct Sp { int x, y, ph; };
    const Sp sparkles[] = { { logoX - 34, 70, 0 }, { logoX + logoW + 40, 84, 1 },
                            { logoX - 20, 150, 2 }, { logoX + logoW + 24, 152, 3 },
                            { W / 2, 52, 1 } };
    for (auto& s : sparkles)
        drawSparkle (g, s.x, s.y, 4, ((twinkle + s.ph) % 2) == 0, palette::sparkle);

    // ---- panels -------------------------------------------------------------
    drawPixelPanel (g, knobPanel, palette::headerPink);
    drawPixelPanel (g, modePanel, palette::headerPink);
    drawPixelPanel (g, amountPanel, palette::headerPink);
    drawPixelPanel (g, infoPanel, palette::white);

    // ---- knob labels + value read-outs ---------------------------------------
    const char* names[] = { "MIX", "DETUNE", "DELAY", "WIDTH" };
    juce::Slider* knobs[] = { &mixKnob, &detuneKnob, &delayKnob, &widthKnob };

    for (int i = 0; i < 4; ++i)
    {
        const int cellX = knobPanel.getX() + i * knobCellW;
        drawTextAligned (g, names[i], cellX, knobCellW, knobPanel.getY() + 14, 3, palette::black, Align::centre);

        const double v = knobs[i]->getValue();
        juce::String text;
        switch (i)
        {
            case 0: text = juce::String (juce::roundToInt (v * 100.0)) + "%"; break;
            case 1: text = (v > 0.049 ? "+" : "") + juce::String (juce::roundToInt (v)) + "C"; break;
            case 2: text = juce::String (juce::roundToInt (v)) + "MS"; break;
            default: text = juce::String (juce::roundToInt (v * 100.0)) + "%"; break;
        }

        const int vy = knobPanel.getBottom() - 30;
        const int tw = measureText (text, 3) + 16;
        const juce::Rectangle<int> tag (cellX + (knobCellW - tw) / 2, vy - 4, tw, 28);
        drawBox (g, tag, palette::white);
        drawTextAligned (g, text, tag.getX(), tag.getWidth(), vy + 3, 3, palette::black, Align::centre);
    }

    // ---- mode panel -----------------------------------------------------------
    drawTextAligned (g, "MODE", modePanel.getX(), modePanel.getWidth(), modePanel.getY() + 14, 3,
                     palette::black, Align::centre);

    // ---- amount bar ------------------------------------------------------------
    drawText (g, "AMOUNT", amountPanel.getX() + 20, amountPanel.getY() + 10, 3, palette::black);
    const int voices = juce::roundToInt (amountSlider.getValue());
    drawTextAligned (g, juce::String (voices) + (voices == 1 ? " VOICE" : " VOICES"),
                     amountPanel.getX(), amountPanel.getWidth() - 20, amountPanel.getY() + 10, 3,
                     palette::black, Align::right);

    // ---- info box ----------------------------------------------------------------
    drawTextAligned (g, "MORE VOICES.", infoPanel.getX(), infoPanel.getWidth(), infoPanel.getY() + 22, 2,
                     palette::black, Align::centre);
    drawTextAligned (g, "BIGGER SOUND.", infoPanel.getX(), infoPanel.getWidth(), infoPanel.getY() + 46, 2,
                     palette::hotPink.darker (0.2f), Align::centre);
    drawSparkle (g, infoPanel.getX() + 20, infoPanel.getY() + 22, 3, (twinkle % 2) == 0, palette::sparkle);
    drawSparkle (g, infoPanel.getRight() - 22, infoPanel.getBottom() - 24, 3, (twinkle % 2) == 1, palette::sparkle);

    // ---- outer window border (drawn last) ---------------------------------------
    g.setColour (palette::black);
    g.drawRect (0, 0, W, H, 4);
}
