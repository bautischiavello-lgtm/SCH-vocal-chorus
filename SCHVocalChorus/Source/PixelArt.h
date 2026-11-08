#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

// ============================================================================
//  Pixel-art helpers: 5x7 bitmap font, sprites, sparkles, palette.
//  Everything is drawn with fillRect on an integer grid - no fonts, no images.
// ============================================================================
namespace pixel
{
namespace palette
{
    inline const juce::Colour headerPink  { 0xFFFFB6C1 };
    inline const juce::Colour canvasPink  { 0xFFFFE4EA };
    inline const juce::Colour hotPink     { 0xFFFF6FA8 };
    inline const juce::Colour softPink    { 0xFFFFCAD6 };
    inline const juce::Colour black       { 0xFF000000 };
    inline const juce::Colour white       { 0xFFFFFFFF };
    inline const juce::Colour hair        { 0xFF3B2314 };
    inline const juce::Colour skin        { 0xFFF2C29B };
    inline const juce::Colour shirt       { 0xFF3A6FD8 };
    inline const juce::Colour sparkle     { 0xFFFFF27A };
}

struct Glyph { char c; const char* rows[7]; };

inline const Glyph* findGlyph (char c)
{
    static const Glyph glyphs[] = {
        { 'A', { "01110","10001","10001","11111","10001","10001","10001" } },
        { 'B', { "11110","10001","10001","11110","10001","10001","11110" } },
        { 'C', { "01110","10001","10000","10000","10000","10001","01110" } },
        { 'D', { "11110","10001","10001","10001","10001","10001","11110" } },
        { 'E', { "11111","10000","10000","11110","10000","10000","11111" } },
        { 'F', { "11111","10000","10000","11110","10000","10000","10000" } },
        { 'G', { "01110","10001","10000","10111","10001","10001","01111" } },
        { 'H', { "10001","10001","10001","11111","10001","10001","10001" } },
        { 'I', { "01110","00100","00100","00100","00100","00100","01110" } },
        { 'J', { "00111","00010","00010","00010","00010","10010","01100" } },
        { 'K', { "10001","10010","10100","11000","10100","10010","10001" } },
        { 'L', { "10000","10000","10000","10000","10000","10000","11111" } },
        { 'M', { "10001","11011","10101","10101","10001","10001","10001" } },
        { 'N', { "10001","11001","10101","10011","10001","10001","10001" } },
        { 'O', { "01110","10001","10001","10001","10001","10001","01110" } },
        { 'P', { "11110","10001","10001","11110","10000","10000","10000" } },
        { 'Q', { "01110","10001","10001","10001","10101","10010","01101" } },
        { 'R', { "11110","10001","10001","11110","10100","10010","10001" } },
        { 'S', { "01111","10000","10000","01110","00001","00001","11110" } },
        { 'T', { "11111","00100","00100","00100","00100","00100","00100" } },
        { 'U', { "10001","10001","10001","10001","10001","10001","01110" } },
        { 'V', { "10001","10001","10001","10001","10001","01010","00100" } },
        { 'W', { "10001","10001","10001","10101","10101","10101","01010" } },
        { 'X', { "10001","10001","01010","00100","01010","10001","10001" } },
        { 'Y', { "10001","10001","01010","00100","00100","00100","00100" } },
        { 'Z', { "11111","00001","00010","00100","01000","10000","11111" } },
        { '0', { "01110","10001","10011","10101","11001","10001","01110" } },
        { '1', { "00100","01100","00100","00100","00100","00100","01110" } },
        { '2', { "01110","10001","00001","00110","01000","10000","11111" } },
        { '3', { "11110","00001","00001","01110","00001","00001","11110" } },
        { '4', { "00010","00110","01010","10010","11111","00010","00010" } },
        { '5', { "11111","10000","11110","00001","00001","10001","01110" } },
        { '6', { "00110","01000","10000","11110","10001","10001","01110" } },
        { '7', { "11111","00001","00010","00100","01000","01000","01000" } },
        { '8', { "01110","10001","10001","01110","10001","10001","01110" } },
        { '9', { "01110","10001","10001","01111","00001","00010","01100" } },
        { '.', { "00000","00000","00000","00000","00000","01100","01100" } },
        { '%', { "11001","11010","00010","00100","01000","01011","10011" } },
        { '-', { "00000","00000","00000","11111","00000","00000","00000" } },
        { '+', { "00000","00100","00100","11111","00100","00100","00000" } },
        { ' ', { "00000","00000","00000","00000","00000","00000","00000" } },
    };
    for (auto& g : glyphs)
        if (g.c == c) return &g;
    return &glyphs[sizeof (glyphs) / sizeof (glyphs[0]) - 1]; // space
}

inline int measureText (const juce::String& text, int scale)
{
    const int n = text.length();
    return n > 0 ? n * 6 * scale - scale : 0;
}

inline void drawText (juce::Graphics& g, const juce::String& text, int x, int y, int scale,
                      juce::Colour colour)
{
    g.setColour (colour);
    int cx = x;
    for (auto ch : text.toUpperCase())
    {
        const auto* gl = findGlyph ((char) ch);
        for (int r = 0; r < 7; ++r)
            for (int c = 0; c < 5; ++c)
                if (gl->rows[r][c] == '1')
                    g.fillRect (cx + c * scale, y + r * scale, scale, scale);
        cx += 6 * scale;
    }
}

enum class Align { left, centre, right };

// Draws text aligned inside [x, x+w].
inline void drawTextAligned (juce::Graphics& g, const juce::String& text, int x, int w, int y,
                             int scale, juce::Colour colour, Align a)
{
    const int tw = measureText (text, scale);
    int px = x;
    if (a == Align::centre) px = x + (w - tw) / 2;
    if (a == Align::right)  px = x + w - tw;
    drawText (g, text, px, y, scale, colour);
}

// Logo-style text with outline + drop shadow.
inline void drawLogoText (juce::Graphics& g, const juce::String& text, int x, int y, int scale,
                          juce::Colour fill, juce::Colour outline, int shadow)
{
    drawText (g, text, x + shadow, y + shadow, scale, outline);   // shadow
    for (int dx = -2; dx <= 2; dx += 2)                           // outline
        for (int dy = -2; dy <= 2; dy += 2)
            if (dx != 0 || dy != 0)
                drawText (g, text, x + dx, y + dy, scale, outline);
    drawText (g, text, x, y, scale, fill);
}

// 2px pixel border box.
inline void drawBox (juce::Graphics& g, juce::Rectangle<int> r, juce::Colour fill,
                     juce::Colour border = palette::black, int thickness = 2)
{
    g.setColour (border);
    g.fillRect (r);
    g.setColour (fill);
    g.fillRect (r.reduced (thickness));
}

// Box with chipped corners for a more "pixel" look.
inline void drawPixelPanel (juce::Graphics& g, juce::Rectangle<int> r, juce::Colour fill)
{
    drawBox (g, r, fill);
    g.setColour (palette::black);
    g.fillRect (r.getRight() - 6, r.getBottom() - 6, 6, 6);  // notch the bottom-right
    g.setColour (palette::headerPink.darker (0.1f));
    g.fillRect (r.getRight() - 4, r.getBottom() - 4, 2, 2);
}

inline void drawSparkle (juce::Graphics& g, int cx, int cy, int px, bool big, juce::Colour c)
{
    g.setColour (palette::black);
    g.fillRect (cx - px, cy - px, px * 2, px * 2);
    g.setColour (c);
    g.fillRect (cx - px / 2, cy - px / 2, px, px);
    const int arm = big ? 3 : 2;
    for (int i = 1; i <= arm; ++i)
    {
        g.fillRect (cx - px / 2, cy - px / 2 - i * px, px, px);
        g.fillRect (cx - px / 2, cy - px / 2 + i * px, px, px);
        g.fillRect (cx - px / 2 - i * px, cy - px / 2, px, px);
        g.fillRect (cx - px / 2 + i * px, cy - px / 2, px, px);
    }
}

// 16x20 character sprite (dark curly hair, blue shirt), facing right.
inline void drawAvatar (juce::Graphics& g, int x, int y, int scale, bool faceLeft)
{
    static const char* rows[20] = {
        "....KKKKKKK.....",
        "..KKDDDDDDDKK...",
        ".KDDDDDDDDDDDK..",
        "KDDDDDDDDDDDDDK.",
        "KDDDDDDDDDDDDDK.",
        "KDDDDDDDDSSSSSK.",
        "KDDDDDDDSSSSSSSK",
        ".KDDDDDSSWKSSSK.",
        ".KDDDDSSSSSSSSK.",
        "..KDDDSSSSSSKK..",
        "..KDDDKSSSSSK...",
        "...KKKKSSSSK....",
        ".....KKSSSK.....",
        "....KKBBBBKK....",
        "...KBBBBBBBBK...",
        "..KBBBBBBBBBBK..",
        ".KBBBBBBBBBBBBK.",
        ".KBBBBBBBBBBBBK.",
        ".KBBBBBBBBBBBBK.",
        ".KKKKKKKKKKKKKK."
    };

    auto colourFor = [] (char ch) -> juce::Colour
    {
        switch (ch)
        {
            case 'K': return palette::black;
            case 'D': return palette::hair;
            case 'S': return palette::skin;
            case 'B': return palette::shirt;
            case 'W': return palette::white;
            default:  return juce::Colours::transparentBlack;
        }
    };

    for (int r = 0; r < 20; ++r)
        for (int c = 0; c < 16; ++c)
        {
            const char ch = rows[r][c];
            if (ch == '.') continue;
            const int col = faceLeft ? (15 - c) : c;
            g.setColour (colourFor (ch));
            g.fillRect (x + col * scale, y + r * scale, scale, scale);
        }
}
} // namespace pixel
