#include "GFXTerminal_bin.h"
void fontBin::init() { Blank = true; };
bool fontBin::color_8(uint8_t c)
{
    color = (bool)c;
    return true;
};
bool fontBin::bgColor_8(uint8_t c)
{
    bgcolor = (bool)c;
    return true;
};
bool fontBin::sgr0()
{
    color = true, bgcolor = false;
    return 1;
};
bool bold() { return 0; };
bool dim() { return 0; };
bool smso() { return 0; };
bool smul() { return 0; };
bool blink() { return 0; };
bool rev() { return 0; };
bool invis() { return 0; };
bool fontBin::operator==(const font &f) const
{
    const fontBin *F = static_cast<const fontBin *>(&f);
    return F && *this == *F;
};
bool fontBin::operator==(const fontBin &f) const { return color == f.color && bgcolor == f.bgcolor && Blank == f.Blank; };
void fontBin::operator=(const font &f)
{
    const fontBin *F = static_cast<const fontBin *>(&f);
    if (F)
        *this = *F;
};
fontBin &fontBin::operator=(const fontBin &f)
{
    color = f.color, bgcolor = f.bgcolor, Blank = f.Blank;
    return *this;
};

fontFactoryBin &GFXTerminal_Bin::newfact()
{
    factory = new fontFactoryBin();
    return *factory;
}
GFXTerminal_Bin::GFXTerminal_Bin(size_t width, size_t height, size_t logLen, size_t styleLen, Adafruit_GrayOLED &oled, const uint8_t *u8g2_font)
    : gfx(oled),
      width(width),
      height(height),
      Terminal(newfact(), 0, 0, logLen, styleLen)
{
    setFont(u8g2_font);
    u8g2.setFontMode(1);
    BG = 10857;
}
GFXTerminal_Bin::~GFXTerminal_Bin() { delete factory; }
void GFXTerminal_Bin::setFont(const uint8_t *u8g2_font)
{
    u8g2.begin(gfx);
    u8g2.setFont(u8g2_font);
    font_width = u8g2.getUTF8Width(" ");
    font_height = u8g2.getFontAscent() - u8g2.getFontDescent();
    resize(width / font_width, height / font_height);
}
void GFXTerminal_Bin::drawChar(int16_t row, int16_t column, unsigned char c, uint16_t color, uint16_t bg)
{
    if (row < 1 || column < 1)
        return;

    int16_t cursor_x = (row - 1) * font_width;
    int16_t cursor_y = column * font_height;
    int16_t descent = u8g2.getFontDescent();

    u8g2.setForegroundColor(color);
    // u8g2.setBackgroundColor(bg);
    gfx.fillRect(cursor_x, cursor_y - font_height - descent, font_width, font_height, bg);
    u8g2.setCursor(cursor_x, cursor_y);
    u8g2.write((uint8_t)c);
}
void GFXTerminal_Bin::drawLine(const char *p, size_t column)
{
    const char *h = p;
    const char *e = end(h);
    if (!e)
        return;
    while (p <= e)
    {
        char c = *p;
        const fontBin *s = static_cast<const fontBin *>(style(p));
        /* if (c == ' ') //* debug
             c = '.';*/
        // if (c == '\n')
        //     c = '\\';
        if (c == '\0')
            c = '*';
        if (p != Cursor())
            drawChar(p - h + 1, column, c, s->color, s->bgcolor);
        else
            drawChar(p - h + 1, column, c, s->bgcolor, s->color);
        p++;
    }
}
void GFXTerminal_Bin::clearDisplay() { return gfx.clearDisplay(); }
void GFXTerminal_Bin::display()
{
#define displayOffset 0;
    static size_t off = 0;
    const char *b = Focus();
    int16_t Height = Terminal::Height();
    for (size_t col = 1; col <= Height; col++)
    {
        drawLine(b, col + off);
        const char *s = down(b);
        if (!s)
            break;
        b = s;
    }
    gfx.display();
    off += displayOffset;
}
size_t GFXTerminal_Bin::write(uint8_t c)
{
    size_t s = Terminal::write(c);
    flush();
    return s;
};
size_t GFXTerminal_Bin::write(const uint8_t *buffer, size_t size)
{
    size_t s = Terminal::write(buffer, size);
    flush();
    return s;
}
void GFXTerminal_Bin::flush()
{
    clearDisplay();
    display();
}
