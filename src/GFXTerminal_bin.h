#include <evlk_Terminal.h>
#include <evlk_Terminal_fontImpl.h>
#include <Print.h>
#include <Adafruit_GFX.h>
#include <Adafruit_GrayOLED.h>
#include <U8g2_for_Adafruit_GFX.h>

class fontBin : public _EVLK_TERMINAL_::font
{
public:
    bool color = true;
    bool bgcolor = false;
    bool Blank = false;

    void init() override;
    bool color_8(uint8_t c) override;
    bool bgColor_8(uint8_t c) override;
    bool sgr0() override;
    bool bold() override { return 0; };
    bool dim() override { return 0; };
    bool smso() override { return 0; };
    bool smul() override { return 0; };
    bool blink() override { return 0; };
    bool rev() override { return 0; };
    bool invis() override { return 0; };
    bool operator==(const font &f) const override;
    bool operator==(const fontBin &f) const;
    void operator=(const font &f) override;
    fontBin &operator=(const fontBin &f);
};
class fontFactoryBin : public _EVLK_TERMINAL_::fontFactory
{
    _EVLK_TERMINAL_::font *createFont() override { return new fontBin(); };
};

class GFXTerminal_Bin : public _EVLK_TERMINAL_::Terminal,
                        public Print
{
private:
    const size_t width;
    const size_t height;
    int16_t font_width;
    int16_t font_height;
    fontFactoryBin *factory;
    uint16_t BG;

    fontFactoryBin &newfact();

public:
    U8G2_FOR_ADAFRUIT_GFX u8g2;
    Adafruit_GrayOLED &gfx;

public:
    GFXTerminal_Bin(size_t width, size_t height, size_t logLen, size_t styleLen,
                    Adafruit_GrayOLED &oled, const uint8_t *u8g2_font = u8g2_font_4x6_tr);
    ~GFXTerminal_Bin();
    void setFont(const uint8_t *u8g2_font);
    void drawChar(int16_t row, int16_t column, unsigned char c, uint16_t color, uint16_t bg);
    void drawLine(const char *p, size_t column);
    void clearDisplay();
    void display();
    size_t write(uint8_t c) override;
    size_t write(const uint8_t *buffer, size_t size) override;
    void flush() override;
    using Print ::write;
};
