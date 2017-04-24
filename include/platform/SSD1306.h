#ifndef __SSD1306_H
#define __SSD1306_H

#define BLACK   0
#define WHITE   1
#define INVERSE 2

/* display coordinate start with 1 i.e. 1-64 and 1-128 */
#define SSD1306_LCDWIDTH    128
#define SSD1306_LCDHEIGHT   64

#define SSD1306_SETCONTRAST         0x81
#define SSD1306_DISPLAYALLON_RESUME 0xA4
#define SSD1306_DISPLAYALLON        0xA5
#define SSD1306_NORMALDISPLAY       0xA6
#define SSD1306_INVERTDISPLAY       0xA7
#define SSD1306_DISPLAYOFF          0xAE
#define SSD1306_DISPLAYON           0xAF
#define SSD1306_SETDISPLAYOFFSET    0xD3
#define SSD1306_SETCOMPINS          0xDA
#define SSD1306_SETVCOMDETECT       0xDB
#define SSD1306_SETDISPLAYCLOCKDIV  0xD5
#define SSD1306_SETPRECHARGE        0xD9
#define SSD1306_SETMULTIPLEX        0xA8
#define SSD1306_SETLOWCOLUMN        0x00
#define SSD1306_SETHIGHCOLUMN       0x10
#define SSD1306_SETSTARTLINE        0x40
#define SSD1306_MEMORYMODE          0x20
#define SSD1306_COLUMNADDR          0x21
#define SSD1306_PAGEADDR            0x22
#define SSD1306_COMSCANINC          0xC0
#define SSD1306_COMSCANDEC          0xC8
#define SSD1306_SEGREMAP            0xA0
#define SSD1306_CHARGEPUMP          0x8D
#define SSD1306_EXTERNALVCC         0x01
#define SSD1306_SWITCHCAPVCC        0x02

// For Scrolling
#define SSD1306_ACTIVATE_SCROLL                         0x2F
#define SSD1306_DEACTIVATE_SCROLL                       0x2E
#define SSD1306_SET_VERTICAL_SCROLL_AREA                0xA3
#define SSD1306_RIGHT_HORIZONTAL_SCROLL                 0x26
#define SSD1306_LEFT_HORIZONTAL_SCROLL                  0x27
#define SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL    0x29
#define SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL     0x2A

#define FONT_HEADER_TYPE                                0
#define FONT_HEADER_ORIENTATION                         1
#define FONT_HEADER_START                               2
#define FONT_HEADER_LETTERS                             3
#define FONT_HEADER_HEIGHT                              4
#define FONT_TYPE_FIXED                                 0
#define FONT_TYPE_PROPORTIONAL                          1
#define FONT_ORIENTATION_VERTICAL_CEILING               2

#define ALIGN_LEFT   0
#define ALIGN_CENTER 1
#define ALIGN_RIGHT  2

typedef struct {
    unsigned char X1;
    unsigned char Y1;
    unsigned char X2;
    unsigned char Y2;
} Bounding_Box_T;

extern void SSD1306_Command(char Command);
extern void SSD1306_Data(char Data);
extern void SSD1306_Contrast(unsigned char Contrast);
extern void SSD1316_Refresh(void);
extern void SSD1316_Clear(void);
extern void SSD1316_GotoXy(unsigned char x,unsigned char y);
extern void SSD1306_Invert(unsigned char Invert);
extern void SSD1306_Init(void);

extern void SSD1306_Pixel(unsigned char x, unsigned char y, unsigned char Colour);
void SSD1306_Draw_Line(int x1, int y1, int x2, int y2, char Colour);
Bounding_Box_T SSD1306_Draw_Char(unsigned char c, unsigned char x, unsigned char y, const unsigned char *font);
Bounding_Box_T SSD1306_Draw_Text(char *string, unsigned char x, unsigned char y, const unsigned char *font, unsigned char spacing);
Bounding_Box_T SSD1306_Draw_Aligned_Text(char *string, unsigned char align, unsigned char y, const unsigned char *font, unsigned char spacing);
extern void SSD1306_Draw_Rectangle(int x1, int y1, int x2, int y2, char colour);
extern void SSD1306_Draw_Circle(unsigned char centre_x, unsigned char centre_y, unsigned char radius, unsigned char colour, unsigned char filled);



#endif
