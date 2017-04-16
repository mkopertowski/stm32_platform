//*****************************************************************************
/** \file    disp.c
 *  \author  Mirek Kopertowski m.kopertowski/at/post.pl
 *  \brief
 */
//*****************************************************************************
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <drivers/SSD1306.h>
#include <font.h>
#include <app/texts.h>
#include <app/disp.h>
//#include <app/measurements.h>
//#include <app/app.h>

#define DEBUG_ON
#include <debug.h>

#define DISPLAY_LINE_COUNT    3
#define DISPLAY_SOFTKEY_COUNT 2

#define FONT_SPACING 2
#define FONT_HEIGHT 20

const uint8_t aDisplayDef[] =
{

#define CREATE_DSP(A,B,C,D,E,F) B,C,D,
#include <app/disp_def.h>
#undef CREATE_DSP

};

const E_TXT_ID aSoftKeysDef[] =
{

#define CREATE_DSP(A,B,C,D,E,F) E,F,
#include <app/disp_def.h>
#undef CREATE_DSP

};

const uint8_t aBatterySymbol[] =
{
  0x0C, 0x0C, 0x3F, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x3F
};

static void vShowStaticTexts(E_DSP_ID);
static void vShowDynamicContent(E_DSP_ID);

// Show functions for dynamic display elements
static void vShowOxygenPressureBIG(uint8_t,uint8_t);
static void vShowBatterySymbol(uint8_t);
static void vShowAtmPressure(uint16_t,uint8_t);
static void vShowCellSettings(uint8_t);

void DSP_vInit(void)
{
    SSD1306_Init();
}

void DSP_vShowDisplay(E_DSP_ID eDispId)
{
    SSD1316_Clear();

    vShowStaticTexts(eDispId);

    vShowDynamicContent(eDispId);

    // send virtual display to LCD
    SSD1316_Refresh();
}

void DSP_vShowTimerDisplay(E_DSP_ID eDispId, uint8_t   ui8Seconds)
{
    /*
  // start timer
  //SYS_vSetAppTimer(ui8Seconds);
  // show display
  DSP_vShowDisplay(eDispId);
  */
}

void DSP_vUpdateDisplay(E_DSP_ID eDspId)
{
    /*
  vShowDynamicContent(eDspId);

  // send virtual display to LCD
  //LCD_vUpdate();
   */
}

static void vShowStaticTexts(E_DSP_ID eDspId)
{
    uint8_t ui8StartPos;
    uint8_t x,y;
    char *pString;

    // show display texts
    ui8StartPos = eDspId * DISPLAY_LINE_COUNT;
    for(y=0;y<DISPLAY_LINE_COUNT;y++)
    {
        pString = TXT_pcGetText(aDisplayDef[ui8StartPos+y]);
        if(pString)
        {
            SSD1306_Draw_Text(pString,0,y*FONT_HEIGHT,Tahoma16,FONT_SPACING);
            //x = LCD_ui8GetStringPosForAlignment(pString,LCD_STYLE_CENTER | DISPLAY_BOLD_FONT);
            //LCD_vGotoXY(x,y);
            //LCD_vPuts_P(pString,DISPLAY_BOLD_FONT);
        }
    }

/*
    // show softkeys
    ui8StartPos = eDspId * DISPLAY_SOFTKEY_COUNT;
    // left softkey
    pString = TXT_pcGetText(pgm_read_byte(&aSoftKeysDef[ui8StartPos]));
    if(pString)
    {
        LCD_vGotoXY(0,DISPLAY_LINE_COUNT);
        LCD_vPuts_P(pString,LCD_STYLE_INVERSE);
    }
    // right softkey
    pString = TXT_pcGetText(pgm_read_byte(&aSoftKeysDef[ui8StartPos+1]));
    if(pString)
    {
        x = LCD_ui8GetStringPosForAlignment(pString,LCD_STYLE_RIGHT | LCD_STYLE_INVERSE);
        LCD_vGotoXY(x,DISPLAY_LINE_COUNT);
        LCD_vPuts_P(pString,LCD_STYLE_INVERSE);
    }
*/
}

static void vShowDynamicContent(E_DSP_ID eDispId)
{
    /*
  switch(eDispId)
  {
    case DSP_ID_CALIBRATION:
      //vShowOxygenPressure(2,LCD_STYLE_OVERWRITE | LCD_STYLE_BOLD);
      vShowOxygenPressureBIG(2,LCD_STYLE_BIG | LCD_STYLE_OVERWRITE);
      break;
    case DSP_ID_OXYGEN:
      vShowOxygenPressureBIG(2,LCD_STYLE_BIG | LCD_STYLE_OVERWRITE);
      vShowBatterySymbol(APP_ui8GetBatteryLevel());
      break;
    case DSP_ID_AMBIENT_PRESSURE:
    case DSP_ID_AMBIENT_PRESSURE_SAVED:
      vShowAtmPressure(APP_ui16GetAtmPressure(),2);
      break;
    case DSP_ID_AMBIENT_PRESSURE_SET:
      vShowAtmPressure(APP_ui16GetAtmPressure(),3);
      break;
    case DSP_ID_SETTINGS_02CELLS:
      vShowCellSettings(APP_ui8GetCellNr());
      break;

    default:
      // nothing to do for display
      break;
  }
  */
}

static void vShowOxygenPressureBIG(uint8_t ui8StartLine, uint8_t ui8Style)
{
    /*
    uint8_t i,val,ui8Reading;

    MES_vCalculateReadings();

    for(i=0;i<GL_NR_OF_O2CELLS;i++)
    {
        if(MES_bIsCellEnabled(i))
        {
            LCD_vGotoXY(30,i+ui8StartLine);

            ui8Reading = MES_ui8GetOxygenReading(i);

            //DEBUG("ui8Reading=%d\n",ui8Reading);

            // print 1st digit
      val = ui8Reading / 100;
      LCD_vPutChar('0'+val,ui8Style);

      // print dot
      LCD_vPutChar('.',ui8Style);

      // print 2nd digit
      ui8Reading -= val * 100;
      val = ui8Reading / 10;
      LCD_vPutChar('0'+val,ui8Style);

      // print 3rd digit
      ui8Reading -= val * 10;
      val = ui8Reading;
      LCD_vPutChar('0'+val,ui8Style);

    }

    ui8StartLine += 1;

  }
  */
}

static void vShowBatterySymbol(uint8_t ui8BatteryLevel)
{
    uint8_t i;

    /*
  LCD_vGotoXY(LCD_X_ADDRESS_RES-13,0);
  LCD_vPutBitmap_P(aBatterySymbol,12,1,LCD_STYLE_OVERWRITE);

  ui8BatteryLevel = ui8BatteryLevel / 25;

  LCD_vGotoXY(LCD_X_ADDRESS_RES-13+3+(4-ui8BatteryLevel)*2,0);
  for(i=0;i<ui8BatteryLevel*2;i++)
  {
    LCD_vPutByte(0x3F,LCD_STYLE_NONE);
  }
  */
}

static void vShowAtmPressure(uint16_t ui16AtmPressure, uint8_t ui8LineNr)
{
    /*
  uint8_t val;
  PGM_P pString;

  LCD_vClearLine(ui8LineNr);

  // print 1st digit
  val = ui16AtmPressure / 1000;
  if(val != 0)
  {
    LCD_vGotoXY(25,ui8LineNr);
    LCD_vPutChar('0'+val,LCD_STYLE_BOLD | LCD_STYLE_OVERWRITE);
  }
  else
  {
    LCD_vGotoXY(30,ui8LineNr);
  }

  // print 2nd digit
  ui16AtmPressure -= val * 1000;
  val = ui16AtmPressure / 100;
  LCD_vPutChar('0'+val,LCD_STYLE_BOLD | LCD_STYLE_OVERWRITE);

  // print 3rd digit
  ui16AtmPressure -= val * 100;
  val = ui16AtmPressure / 10;
  LCD_vPutChar('0'+val,LCD_STYLE_BOLD | LCD_STYLE_OVERWRITE);

  // print 4th digit
  ui16AtmPressure -= val * 10;
  val = ui16AtmPressure;
  LCD_vPutChar('0'+val,LCD_STYLE_BOLD | LCD_STYLE_OVERWRITE);

  pString = TXT_pcGetText(TXT_ID_HPA);
  LCD_vPuts_P(pString,LCD_STYLE_BOLD | LCD_STYLE_OVERWRITE);
  */
}

static void vShowCellSettings(uint8_t ui8CellNr)
{
    /*
  PGM_P pString;
  bool  bIsEnabled;

  /* o2 cell 1/2 */ /*
  LCD_vClearLine(2);
  LCD_vGotoXY(20,2);
  pString = TXT_pcGetText(TXT_ID_O2CELL);
  LCD_vPuts_P(pString,LCD_STYLE_BOLD | LCD_STYLE_OVERWRITE);
  LCD_vPutChar(' ',LCD_STYLE_BOLD | LCD_STYLE_OVERWRITE);
  LCD_vPutChar('1'+ui8CellNr,LCD_STYLE_BOLD | LCD_STYLE_OVERWRITE);

  bIsEnabled = EE_bIsSensorEnabled(ui8CellNr);

  LCD_vClearLine(4);
  LCD_vGotoXY(45,4);
  if(bIsEnabled)
  {
    pString = TXT_pcGetText(TXT_ID_ON);
  }
  else
  {
    pString = TXT_pcGetText(TXT_ID_OFF);
  }
  LCD_vPuts_P(pString,LCD_STYLE_BOLD | LCD_STYLE_OVERWRITE);
  */
}
