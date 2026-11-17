#include <ft2build.h>
#include FT_FREETYPE_H
#include <stdio.h>
#include "slot-font.h"

static int Check(int condition, const char* message) {
    if (condition) return 1;
    puts(message);
    return 0;
}

int main(void) {
    FT_Library library = 0;
    FT_Face face = 0;
    if (!Check(FT_Init_FreeType(&library) == 0, "FT_Init_FreeType failed")) return 1;
    if (!Check(FT_New_Memory_Face(library, SlotFont, sizeof(SlotFont), 0, &face) == 0, "TTF open or imported slot creation failed")) return 2;
    FT_GlyphSlot created = face->glyph;
    if (!Check(created && created->internal, "slot was not initialized")) return 3;
    const unsigned sizes[] = {8, 13, 19};
    for (unsigned i = 0; i < sizeof(sizes) / sizeof(sizes[0]); ++i) {
        if (!Check(FT_Set_Pixel_Sizes(face, 0, sizes[i]) == 0, "pixel size failed")) return 4;
        FT_Int32 flags = FT_LOAD_NO_AUTOHINT | FT_LOAD_PEDANTIC;
        if (!Check(FT_Load_Char(face, 'A', flags | FT_LOAD_NO_HINTING) == 0, "unhinted glyph load failed")) return 5;
        if (!Check(face->glyph == created && created->outline.n_points == 4, "unhinted slot identity or outline changed")) return 6;
        FT_Pos unhinted = created->outline.points[0].x;
        if (!Check(unhinted != 128, "control glyph already has the hinted coordinate")) return 7;
        if (!Check(FT_Load_Char(face, 'A', flags) == 0, "native bytecode hinting failed")) return 8;
        if (!Check(face->glyph == created && created->outline.points[0].x == 128, "SCFS hint instruction did not run on the created slot")) return 9;
        if (!Check(FT_Render_Glyph(created, FT_RENDER_MODE_NORMAL) == 0, "hinted glyph rasterization failed")) return 10;
        if (!Check(created->bitmap.rows && created->bitmap.width && created->bitmap.buffer && created->bitmap.pixel_mode == FT_PIXEL_MODE_GRAY, "no grayscale bitmap")) return 11;
        unsigned coverage = 0;
        for (unsigned row = 0; row < created->bitmap.rows; ++row)
            for (unsigned col = 0; col < created->bitmap.width; ++col)
                coverage += created->bitmap.buffer[(long)row * created->bitmap.pitch + col];
        if (!Check(coverage > 0 && created->advance.x > 0, "empty bitmap or advance")) return 12;
        if (!Check(face->glyph == created, "rendering replaced the created slot")) return 13;
    }
    if (!Check(FT_Done_Face(face) == 0 && FT_Done_FreeType(library) == 0, "font or slot destruction failed")) return 14;
    puts("FT_New_GlyphSlot: real TTF, native SCFS hinting at 8/13/19 ppem, rasterization and destruction passed");
    return 0;
}
