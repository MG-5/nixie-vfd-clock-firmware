#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

/// Font that contains glyph for rendering strings.
class Font
{
public:
    static constexpr size_t NumberOfGlyphs = 128; //! Number of glyphs stored in Font

public:
    /// Visual representation of a single character.
    using Glyph = uint16_t;

    /// Array of NumberOfGlyphs Glyph pointers.
    ///
    /// Each entry must point to a valid Glyph object; nullptr is not allowed.
    /// If a glyph for a character should not be provided by the font, it should
    /// instead point to a "missing character" glyph.
    using GlyphTable = std::array<Glyph *, NumberOfGlyphs>;

    /// Creates a new font object.
    /// \param tab Reference to a GlyphTable object containing the glyph pointers
    explicit Font(const GlyphTable &tab) : glyphs{tab} {};

    [[nodiscard]] Glyph &getGlyph(uint8_t character) const
    {
        return *glyphs[character];
    }

private:
    const GlyphTable &glyphs;
};

extern Font font;
