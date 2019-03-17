#ifndef __ssd1306_h
#define __ssd1306_h

#define TAG_DISPLAY "SSD1306"

// Following definitions are bollowed from 
// http://robotcantalk.blogspot.com/2015/03/interfacing-arduino-with-ssd1306-driven.html

// SLA (0x3C) + WRITE_MODE (0x00) =  0x78 (0b01111000)
#define OLED_I2C_ADDRESS   0x3C
#define OLED_DISPLAY_WIDTH    128  // width of display in dots/pixels
#define OLED_DISPLAY_HEIGHT   64   // height of display in dots/pixels
#define OLED_DISPLAY_BUFFER_SIZE  OLED_DISPLAY_HEIGHT*OLED_DISPLAY_WIDTH
#define SSD1306_MAX_CONTRAST     255  

/*
 * CONTROL BYTE
 * After the transmission of the slave address, either the control byte or the data byte may be sent across
 * the SDA. A control byte mainly consists of Co and D/C# bits following by six “0” ‘s.
 *   a. If the Co bit is set as logic “0”, the transmission of the following information will contain
 *      data bytes only.
 *   b. The D/C# bit determines the next data byte is acted as a command or a data. If the D/C# bit is
 *      set to logic “0”, it defines the following data byte as a command. If the D/C# bit is set to
 *      logic “1”, it defines the following data byte as a data which will be stored at the GDDRAM.
 *      The GDDRAM column address pointer will be increased by one automatically after each
 *      data write. 
 *         If D/C# pin is HIGH, D[7:0] is interpreted as display data written to Graphic Display Data RAM (GDDRAM).
 *         If it is LOW, the input at D[7:0] is interpreted as a command. Then data input will be decoded and written to
 *         the corresponding command register.)
*/
#define OLED_CONTROL_BYTE_CMD_SINGLE    0x80
#define OLED_CONTROL_BYTE_CMD_STREAM    0x00  // the transmission of the following information will contain data bytes only
#define OLED_CONTROL_BYTE_DATA_STREAM   0x40

// Fundamental commands (pg.28)
#define OLED_CMD_SET_CONTRAST           0x81    // follow with 0x7F
#define OLED_CMD_DISPLAY_RAM            0xA4  // Resume to RAM content display (RESET)
#define OLED_CMD_DISPLAY_ALLON          0xA5  // Entire display ON (it forces the entire display to be “ON”, regardless of the contents of the display data RAM. 0xA4 must be called after to continue normal displaying)
#define OLED_CMD_DISPLAY_NORMAL         0xA6  // Normal display (RESET)
#define OLED_CMD_DISPLAY_INVERTED       0xA7  // Inverse display
#define OLED_CMD_DISPLAY_OFF            0xAE  // Display OFF (sleep mode)(RESET)
#define OLED_CMD_DISPLAY_ON             0xAF  // Display ON in normal mode

// Addressing Command Table (pg.30)
#define OLED_CMD_SET_MEMORY_ADDR_MODE   0x20    // follow with 0x00 = HORZ mode = Behave like a KS108 graphic LCD
#define OLED_CMD_SET_COLUMN_RANGE       0x21    // can be used only in HORZ/VERT mode - follow with 0x00 and 0x7F = COL127
#define OLED_CMD_SET_PAGE_RANGE         0x22    // can be used only in HORZ/VERT mode - follow with 0x00 and 0x07 = PAGE7

// Addressing mode
#define SSD1306_HORIZONTAL_ADDRESSING_MODE   0x00
#define SSD1306_VERTICAL_ADDRESSING_MODE     0x01
#define SSD1306_PAGE_ADDRESSING_MODE         0x02


// Hardware Config (pg.31)
#define OLED_CMD_SET_DISPLAY_START_LINE 0x40
#define OLED_CMD_SET_SEGMENT_REMAP      0xA1    
#define OLED_CMD_SET_MUX_RATIO          0xA8    // follow with 0x3F = 64 MUX
#define OLED_CMD_SET_COM_SCAN_MODE      0xC8    
#define OLED_CMD_SET_DISPLAY_OFFSET     0xD3    // follow with 0x00
#define OLED_CMD_SET_COM_PIN_MAP        0xDA    // follow with 0x12
#define OLED_CMD_NOP                    0xE3    // NOP

// Timing and Driving Scheme (pg.32)
#define OLED_CMD_SET_DISPLAY_CLK_DIV    0xD5    // follow with 0x80 for default
#define OLED_CMD_SET_PRECHARGE          0xD9    // follow with 0xF1
#define OLED_CMD_SET_VCOMH_DESELCT      0xDB    // follow with 0x30

// Set Display Clock Divide Ratio/ Oscillator Frequency
#define OLED_DISPLAY_SPEED_DEFAULT      0x10
#define OLED_DISPLAY_SPEED_MAX          0xF0

// Charge Pump (pg.62)
#define OLED_CMD_CHARGE_PUMP_SETTING    0x8D    // follow with 0x14,
#define OLED_CMD_CHARGE_PUMP_ENABLE     0x14    // Enable charge pump during display on
#define OLED_CMD_CHARGE_PUMP_DISABLE    0x10    // Disable charge pump (RESET)

#endif // __ssd1306_h