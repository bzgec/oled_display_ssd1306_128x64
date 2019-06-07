#ifndef __OLEDDisplay_h
#define __OLEDDisplay_h

/*
 * Modified from:
 *   - https://github.com/yanbe/ssd1306-esp-idf-i2c
 *   - https://github.com/ThingPulse/esp8266-oled-ssd1306
 * 
 * First call this two functions:
 *   - void i2c_master_init(BYTE bySDA_pin, BYTE bySCL_pin);
 *   - void oled_display_init();
 * 
 * Use some of functions below to draw data in buffer.
 * Then call oled_display_send_buffer() to display buffer on the display.
 * Before drawing new data to buffer call oled_display_clearBuffer().
 * 
 * Use functions below to configure display:
 *   - oled_display_sendCommand(BYTE byCommand);  -> check "ssd1306.h" for commands
 *   - oled_display_contrast_set(BYTE byContrast);
 *   - oled_display_setColor(EOLED_DISPLAY_PIXEL_COLOR eColor);
 *   - oled_display_setTextAlignment(EOLED_DISPLAY_TEXT_ALIGNMENT eTextAlignment);
 *   - oled_display_setFont(const char* pszFontData);
 */

#include "Types.h"
#include "ssd1306.h"
#include "driver/i2c.h"
#include "OLEDDisplayFonts_headerInclude.h" 

// Header Values (check "OLEDDisplayFonts.h" file)
#define JUMPTABLE_BYTES 4

#define JUMPTABLE_LSB   1
#define JUMPTABLE_SIZE  2
#define JUMPTABLE_WIDTH 3
#define JUMPTABLE_START 4

#define WIDTH_POS 0
#define HEIGHT_POS 1
#define FIRST_CHAR_POS 2
#define CHAR_NUM_POS 3


#ifndef _SWAP_INT16_T
#define _SWAP_INT16_T(a, b) { SHORT t = a; a = b; b = t; }
#endif  // _SWAP_INT16_T

#ifndef MAX
#define MAX(a,b) ((a)>(b)?(a):(b))
#endif  // MAX

#ifndef PGM_READ_BYTE
#define PGM_READ_BYTE(addr) (*(const unsigned char *)(addr))
#endif  // PGM_READ_BYTE

typedef enum 
{
  eBlack = 0,
  eWhite = 1,
  eInverse = 2
} EOLED_DISPLAY_PIXEL_COLOR;

typedef enum 
{
  eText_align_left = 0,
  eText_align_right = 1,
  eText_align_center = 2,
  eText_align_center_both = 3
} EOLED_DISPLAY_TEXT_ALIGNMENT;

typedef BYTE (*FontTableLookupFunction)(const BYTE ch);

void i2c_master_init(BYTE bySDA_pin, BYTE bySCL_pin);
void oled_display_init();
void oled_display_sendCommand(BYTE byCommand);
BOOL oled_display_check();
void oled_display_contrast_set(BYTE byContrast);
void oled_display_setColor(EOLED_DISPLAY_PIXEL_COLOR eColor);
void oled_display_setTextAlignment(EOLED_DISPLAY_TEXT_ALIGNMENT eTextAlignment);
void oled_display_setFont(const char* pszFontData);

// Configure OLED display to your needs
void oled_display_configure(void);

// Send buffer from uC to OLED
void oled_display_send_buffer(void);

// Clear buffer (uC)
void oled_display_clearBuffer(void);

// Set the function that will convert utf-8 to font table index
void oled_display_setFontTableLookupFunction(FontTableLookupFunction function);

// Fill buffer with some pattern
void fillBuffer(void);

// Print buffer to serial output
void printfBuffer(void);

// Set color of the pixels
void oled_display_setColor(EOLED_DISPLAY_PIXEL_COLOR eColor);

// Draw a pixel at given position
void oled_display_setPixel(SHORT sX, SHORT sY);

// Draw a line horizontally
BOOL oled_display_drawHorizontalLine(BYTE byX, BYTE byY, WORD wLength);

// Draw a line vertically
BOOL oled_display_drawVerticalLine(BYTE byX, BYTE byY, SHORT sLength);

// Draw a line from position 0 to position 1
void oled_display_drawLine(SHORT sX0, SHORT sY0, SHORT sX1, SHORT sY1);

// Draw the border of a rectangle at the given location
void oled_display_drawRect(SHORT sX, SHORT sY, SHORT sWidth, SHORT sHeight);

// Fill the rectangle
void oled_display_fillRect(SHORT sX, SHORT sY, SHORT sWidth, SHORT sHeight);

// Draw the border of a circle
void oled_display_drawCircle(SHORT sX0, SHORT sY0, SHORT sRadius); 

// Draw all Quadrants specified in the quads bit mask
// Quadrants are numberd like this:
//   0010 | 0001
//  ------|-----
//   0100 | 1000
void oled_display_drawCircleQuads(SHORT sX0, SHORT sY0, SHORT sRadius, BYTE byQuads);

// Fill circle
void oled_display_fillCircle(SHORT sX0, SHORT sY0, SHORT sRadius);

// Draws a rounded progress bar with the outer dimensions given by width and height. Progress is
// a unsigned byte value between 0 and 100
void oled_display_drawProgressBar(WORD wX, WORD wY, WORD wWidth, WORD wHeight, BYTE byProgress);

// Draw a bitmap in the internal image format
void oled_display_drawFastImage(SHORT sXMove, SHORT sYMove, SHORT sWidth, SHORT sHeight, const BYTE* c_pbImage);

// Draws a rounded progress bar with the outer dimensions given by width and height. Progress is
// a unsigned byte value between 0 and 100
void oled_display_drawProgressBar(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t progress);

// Draw a bitmap in the internal image format
void oled_display_drawFastImage(int16_t x, int16_t y, int16_t width, int16_t height, const uint8_t *image);

// Draw a XBM
void oled_display_drawXbm(SHORT sXMove, SHORT sYMove, SHORT sWidth, SHORT sHeight, const BYTE* c_pbXbm);

/* Text functions */

// Draws a string at the given location
void oled_display_drawString(SHORT sXStart, SHORT sYStart, char* pcText_recieved);

// Draws a String with a maximum width at the given location.
// If the given String is wider than the specified width
// The text will be wrapped to the next line at a space or dash
void oled_display_drawStringMaxWidth(SHORT sXMove, SHORT sYMove, WORD wMaxLineWidth, char* pcStrUser);

// Returns the width of the const char* with the current
// font settings
WORD oled_display_getStringWidth(const char* c_pcText, WORD wLength);

// converts utf8 characters to extended ascii
char* oled_display_utf8ascii(char* pszStr);

BYTE oled_display_fontTableLookupFunction_test(const BYTE c_byCh);

#endif  // __OLEDDisplay_h