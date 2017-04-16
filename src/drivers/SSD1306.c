#include <stdlib.h>
#include <stm32f10x.h>
#include <SSD1306.h>
#include <font.h>

#define SSD1306_RES_ON      GPIO_WriteBit(GPIOA,GPIO_Pin_0,Bit_SET);        //RC0
#define SSD1306_RES_OFF     GPIO_WriteBit(GPIOA,GPIO_Pin_0,Bit_RESET);
#define SSD1306_DC_ON       GPIO_WriteBit(GPIOA,GPIO_Pin_1,Bit_SET);        //RC4
#define SSD1306_DC_OFF      GPIO_WriteBit(GPIOA,GPIO_Pin_1,Bit_RESET);
#define SSD1306_CS_ON       GPIO_WriteBit(GPIOA,GPIO_Pin_2,Bit_SET);        //RC5
#define SSD1306_CS_OFF      GPIO_WriteBit(GPIOA,GPIO_Pin_2,Bit_RESET);

unsigned char   SSD1306_Buffer[1024];
unsigned short  SSD1306_Counter;
unsigned char   GLCD_Dirty_Pages;

uint8_t SPI1_Transfer(uint8_t data)
{
    SPI1    -> DR                   =data&0xFF;
    while(SPI1->SR&0x80);
}

void Delay_Cycles(unsigned long D_C)
{
    unsigned long i;
    for(i=0;i<D_C;i++);
}

void SSD1306_Command(char Command)
{
    SSD1306_DC_OFF;             // DC is low for command data

    SSD1306_CS_OFF;             // Select the chip

    SPI1_Transfer(Command);

    SSD1306_CS_ON;              // Unselect the chip
}

void SSD1306_Data(char Data)
{
    SSD1306_DC_ON;              // DC is low for command data

    SSD1306_CS_OFF;             // Select the chip

    SPI1_Transfer(Data);

    SSD1306_CS_ON;              // Unselect the chip
}

void SSD1306_Init(void)
{
    //General_SPI_Init();

    SSD1306_CS_ON;

    SSD1306_RES_ON;
    Delay_Cycles(10000);
    SSD1306_RES_OFF;
    Delay_Cycles(10000);
    SSD1306_RES_ON;

    // Init sequence for 128x64 OLED module
    SSD1306_Command(SSD1306_DISPLAYOFF);                    // 0xAE
    SSD1306_Command(SSD1306_SETDISPLAYCLOCKDIV);            // 0xD5
    SSD1306_Command(0x80);                                  // the suggested ratio 0x80
    SSD1306_Command(SSD1306_SETMULTIPLEX);                  // 0xA8
    SSD1306_Command(0x3F);
    SSD1306_Command(SSD1306_SETDISPLAYOFFSET);              // 0xD3
    SSD1306_Command(0x0);                                   // no offset
    SSD1306_Command(SSD1306_SETSTARTLINE | 0x0);            // line #0
    SSD1306_Command(SSD1306_CHARGEPUMP);                    // 0x8D
    SSD1306_Command(0x14);                                                                   // Charge Pump Active
    SSD1306_Command(SSD1306_MEMORYMODE);                    // 0x20
    SSD1306_Command(0x00);                                  // 0x0 act like ks0108
    SSD1306_Command(SSD1306_SEGREMAP | 0x1);
    SSD1306_Command(SSD1306_COMSCANDEC);
    SSD1306_Command(SSD1306_SETCOMPINS);                    // 0xDA
    SSD1306_Command(0x12);
    SSD1306_Command(SSD1306_SETCONTRAST);                   // 0x81
    SSD1306_Command(0xFF);
    SSD1306_Command(SSD1306_SETPRECHARGE);                  // 0xd9
    SSD1306_Command(0xF1);
    SSD1306_Command(SSD1306_SETVCOMDETECT);                 // 0xDB
    SSD1306_Command(0x40);
    SSD1306_Command(SSD1306_DISPLAYALLON_RESUME);           // 0xA4
    SSD1306_Command(SSD1306_NORMALDISPLAY);                 // 0xA6

    SSD1306_Command(SSD1306_SETLOWCOLUMN  | 0x0);                    // low col = 0
    SSD1306_Command(SSD1306_SETHIGHCOLUMN | 0x0);                  // hi col = 0
    SSD1306_Command(SSD1306_SETSTARTLINE  | 0x0);                    // line #0

    for(SSD1306_Counter=0;SSD1306_Counter<1024;SSD1306_Counter++)
        SSD1306_Data(0x00);

    SSD1306_Command(SSD1306_DISPLAYON);                                      // turn on oled panel
}

void SSD1306_Contrast(unsigned char Contrast)
{
    SSD1306_Command(SSD1306_SETCONTRAST);                   // 0x81
    SSD1306_Command(Contrast);
}

void SSD1306_Invert(unsigned char Invert)
{
    if (Invert)
    {
    SSD1306_Command(SSD1306_INVERTDISPLAY);
  }
    else
    {
    SSD1306_Command(SSD1306_NORMALDISPLAY);
  }
}

void SSD1316_Clear(void)
{
    for(SSD1306_Counter=0;SSD1306_Counter<1024;SSD1306_Counter++)
        SSD1306_Buffer[SSD1306_Counter]=0x00;
}

void SSD1316_Refresh(void)
{
    for(SSD1306_Counter=0;SSD1306_Counter<1024;SSD1306_Counter++)
        SSD1306_Data(SSD1306_Buffer[SSD1306_Counter]);
}

/*****************************For Font Usage*************************************/
void SSD1306_Pixel(unsigned char x, unsigned char y, unsigned char Colour)
{
    unsigned short ArrayPos=0;

    if (x > 128 || y > 64) return;

    // Real screen coordinates are 0-63, not 1-64.
    x                            -= 1;
    y                            -= 1;

    ArrayPos                    = x + ((y / 8) * 128);

    GLCD_Dirty_Pages |= 1 << (ArrayPos / 128);

    if(Colour)
        SSD1306_Buffer[ArrayPos] |= 1 << (y % 8);
    else
        SSD1306_Buffer[ArrayPos] &= 0xFF ^ 1 << (y % 8);
}

void SSD1306_Draw_Line(int x1, int y1, int x2, int y2, char Colour)
{
    int xinc1, yinc1, den, num, numadd, numpixels, curpixel, xinc2, yinc2;

    int deltax  = abs(x2 - x1);     // The difference between the x's
    int deltay  = abs(y2 - y1);     // The difference between the y's
    int x           = x1;               // Start x off at the first pixel
    int y           = y1;               // Start y off at the first pixel

    if (x2 >= x1)                       // The x-values are increasing
    {
      xinc1 = 1;
        xinc2 = 1;
    }
    else
    {                                                               // The x-values are decreasing
      xinc1 = -1;
      xinc2 = -1;
    }

    if (y2 >= y1)                       // The y-values are increasing
    {
      yinc1 = 1;
      yinc2 = 1;
    }
    else                                // The y-values are decreasing
    {
      yinc1 = -1;
      yinc2 = -1;
    }

    if (deltax >= deltay)               // There is at least one x-value for every y-value
    {
      xinc1         = 0;                // Don't change the x when numerator >= denominator
      yinc2         = 0;                // Don't change the y for every iteration
      den           = deltax;
      num           = deltax / 2;
      numadd        = deltay;
      numpixels = deltax;               // There are more x-values than y-values
    }
    else                                // There is at least one y-value for every x-value
    {
      xinc2         = 0;                // Don't change the x for every iteration
      yinc1         = 0;                // Don't change the y when numerator >= denominator
      den           = deltay;
      num           = deltay / 2;
      numadd        = deltax;
      numpixels = deltay;               // There are more y-values than x-values
    }

    for (curpixel = 0; curpixel <= numpixels; curpixel++)
    {
      SSD1306_Pixel(x, y, Colour);      // Draw the current pixel
      num += numadd;                    // Increase the numerator by the top of the fraction

      if (num >= den)                   // Check if numerator >= denominator
      {
            num -= den;                     // Calculate the new numerator value
            x += xinc1;                     // Change the x as appropriate
            y += yinc1;                     // Change the y as appropriate
      }

      x += xinc2;                       // Change the x as appropriate
      y += yinc2;                       // Change the y as appropriate
    }
}

Bounding_Box_T SSD1306_Draw_Char(unsigned char c, unsigned char x, unsigned char y, const unsigned char *font)
{
    unsigned short  pos;
    unsigned char   width;
    Bounding_Box_T  ret;
    unsigned char   i,j;
    int height          =font[FONT_HEADER_HEIGHT];

    ret.X1 = x;
    ret.Y1 = y;
    ret.X2 = x;
    ret.Y2 = y;

    // Read first byte, should be 0x01 for proportional
    if (font[FONT_HEADER_TYPE] != FONT_TYPE_PROPORTIONAL)                                                                                       return ret;
    // Check second byte, should be 0x02 for "vertical ceiling"
    if (font[FONT_HEADER_ORIENTATION] != FONT_ORIENTATION_VERTICAL_CEILING)                                                     return ret;
    // Check that font start + number of bitmaps contains c
    if (!(c >= font[FONT_HEADER_START] && c <= font[FONT_HEADER_START] + font[FONT_HEADER_LETTERS]))    return ret;

    // Adjust for start position of font vs. the char passed
    c -= font[FONT_HEADER_START];

    // Work out where in the array the character is
    pos = font[c * FONT_HEADER_START + 5];
    pos <<= 8;
    pos |= font[c * FONT_HEADER_START + 6];

    // Read first byte from this position, this gives letter width
    width = font[pos];

    // Draw left to right

    for (i = 0; i < width; i++)
    {
        // Draw top to bottom
        for (j = 0; j < font[FONT_HEADER_HEIGHT]; j++)
        {

            if (j % 8 == 0) pos++;

            if (font[pos] & 1 << (j % 8))
                SSD1306_Pixel(x + i, y + j, 1);
            else
                SSD1306_Pixel(x + i, y + j, 0);
        }
    }

    ret.X2 = ret.X1 + width - 1;
    // TODO: Return the actual height drawn, rather than the height of the
    //       font.
    ret.Y2 = ret.Y1 + height;
    ret.Y2 = ret.Y1 + font[FONT_HEADER_HEIGHT];

    return ret;
}

Bounding_Box_T SSD1306_Draw_Text(char *string, unsigned char x, unsigned char y, const unsigned char *font, unsigned char spacing)
{
    Bounding_Box_T ret,tmp;

    ret.X1 = x;
    ret.Y1 = y;

    spacing += 1;

    // BUG: As we move right between chars we don't actually wipe the space
    while (*string != 0)
    {
        tmp = SSD1306_Draw_Char(*string++, x, y, font);
        // Leave a single space between characters
        x = tmp.X2 + spacing;
    }

    ret.X2 = tmp.X2;
    ret.Y2 = tmp.Y2;

    return ret;
}

void SSD1306_Draw_Rectangle(int x1, int y1, int x2, int y2, char colour)
{
    // Top
    SSD1306_Draw_Line(x1, y1, x2, y1, colour);
    // Left
    SSD1306_Draw_Line(x1, y1, x1, y2, colour);
    // Bottom
    SSD1306_Draw_Line(x1, y2, x2, y2, colour);
    // Right
    SSD1306_Draw_Line(x2, y1, x2, y2, colour);
}

void SSD1306_Draw_Circle(unsigned char centre_x, unsigned char centre_y, unsigned char radius, unsigned char colour, unsigned char filled)
{
    signed char x = 0;
    signed char y = radius;
    signed char p = 1 - radius;

    if (!radius) return;

    if(filled)
    {
        for (x = 0; x < y; x++)
        {
            if (p < 0)
            {
                p += x * 2 + 3;
            }
            else
            {
                p += x * 2 - y * 2 + 5;
                y--;
            }

            SSD1306_Draw_Line(centre_x - x, centre_y - y, centre_x + x, centre_y - y, colour);
            SSD1306_Draw_Line(centre_x - y, centre_y - x, centre_x + y, centre_y - x, colour);
            SSD1306_Draw_Line(centre_x + x, centre_y + y, centre_x - x, centre_y + y, colour);
            SSD1306_Draw_Line(centre_x + y, centre_y + x, centre_x - y, centre_y + x, colour);
        }
    }
    else
    {

        for (x = 0; x < y; x++)
        {
            if (p < 0)
            {
                p += x * 2 + 3;
            }
            else
            {
                p += x * 2 - y * 2 + 5;
                y--;
            }

            SSD1306_Pixel(centre_x - x, centre_y - y, colour);
            SSD1306_Pixel(centre_x - y, centre_y - x, colour);
            SSD1306_Pixel(centre_x + y, centre_y - x, colour);
            SSD1306_Pixel(centre_x + x, centre_y - y, colour);
            SSD1306_Pixel(centre_x - x, centre_y + y, colour);
            SSD1306_Pixel(centre_x - y, centre_y + x, colour);
            SSD1306_Pixel(centre_x + y, centre_y + x, colour);
            SSD1306_Pixel(centre_x + x, centre_y + y, colour);
        }
    }
}
