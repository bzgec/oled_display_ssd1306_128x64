#include "OLEDDisplay.h"
#include "esp_log.h"
#include <string.h>
#include <math.h>
#include "esp_task_wdt.h"
#include "OLEDDisplayFonts.h"

static BYTE fs_abyBuffer[OLED_DISPLAY_BUFFER_SIZE/8];
static EOLED_DISPLAY_PIXEL_COLOR fs_eDisplayColor = eWhite;
static EOLED_DISPLAY_TEXT_ALIGNMENT fs_eTextAlignment = eText_align_left;
static const char* fs_pszTextFont = ArialMT_Plain_10;
FontTableLookupFunction fontTableLookupFunction = oled_display_fontTableLookupFunction_test;

//void inline oled_display_drawInternal(SHORT sXMove, SHORT sYMove, SHORT sWidth, SHORT sHeight, const BYTE* c_pbyData, WORD wOffset, WORD wBytesInData);
static void oled_display_drawInternal(SHORT sXMove, SHORT sYMove, SHORT sWidth, SHORT sHeight, const BYTE* c_pbyData, WORD wOffset, WORD wBytesInData);
static void oled_display_drawStringInternal(SHORT sXMove, SHORT sYMove, char* pcText, WORD wTextLength, WORD wTextWidth);


void i2c_master_init(BYTE bySDA_pin, BYTE bySCL_pin)
{
	i2c_config_t i2c_config = 
	{
		.mode = I2C_MODE_MASTER,
		.sda_io_num = bySDA_pin,
		.scl_io_num = bySCL_pin,
		.sda_pullup_en = GPIO_PULLUP_ENABLE,
		.scl_pullup_en = GPIO_PULLUP_ENABLE,
		.master.clk_speed = 1000000
	};
	i2c_param_config(I2C_NUM_0, &i2c_config);
	i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);
}

void oled_display_init(void) 
{
	esp_err_t espRc;
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();

	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
	i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_STREAM, true);

	// setting charge pump
	i2c_master_write_byte(cmd, OLED_CMD_CHARGE_PUMP_SETTING, true);
	i2c_master_write_byte(cmd, OLED_CMD_CHARGE_PUMP_ENABLE, true);

	//i2c_master_write_byte(cmd, OLED_CMD_SET_SEGMENT_REMAP, true); // reverse left-right mapping
	//i2c_master_write_byte(cmd, OLED_CMD_SET_COM_SCAN_MODE, true); // reverse up-bottom mapping

	i2c_master_write_byte(cmd, OLED_CMD_SET_CONTRAST, true);
	i2c_master_write_byte(cmd, SSD1306_MAX_CONTRAST, true);

	//i2c_master_write_byte(cmd, OLED_CMD_SET_DISPLAY_CLK_DIV, true);
	//i2c_master_write_byte(cmd, SSD1306_MAX_CONTRAST, true);

	i2c_master_write_byte(cmd, OLED_CMD_DISPLAY_ON, true);
	i2c_master_stop(cmd);

	espRc = i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
	if (espRc == ESP_OK) 
	{
		ESP_LOGI(TAG_DISPLAY, "OLED configured successfully");
	} else 
	{
		ESP_LOGE(TAG_DISPLAY, "OLED configuration failed. code: 0x%.2X", espRc);
	}
	i2c_cmd_link_delete(cmd);
}

void oled_display_sendCommand(BYTE byCommand)
{
	i2c_cmd_handle_t cmd;
	esp_err_t espRc;

	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
	i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_SINGLE, true);
	i2c_master_write_byte(cmd, byCommand, true);
	i2c_master_stop(cmd);

	espRc = i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
	if (espRc != ESP_OK)
	{
		ESP_LOGE(TAG_DISPLAY, "Command sending failed. code: 0x%.2X", espRc);
	}
	i2c_cmd_link_delete(cmd);
}

BOOL oled_display_check(void)
{
  i2c_cmd_handle_t cmd;
	esp_err_t espRc;

	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
	i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_SINGLE, true);
	i2c_master_write_byte(cmd, OLED_CMD_NOP, true);
	i2c_master_stop(cmd);

	espRc = i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd);
	if (espRc != ESP_OK)
	{
		ESP_LOGE(TAG_DISPLAY, "Display not responding code: 0x%.2X", espRc);
    return FALSE;
	}
  else
  {
    return TRUE;
  }
  
}

void oled_display_contrast_set(BYTE byContrast)
{
	i2c_cmd_handle_t cmd;
	esp_err_t espRc;

	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
	i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_STREAM, true);
	i2c_master_write_byte(cmd, OLED_CMD_SET_CONTRAST, true);
	i2c_master_write_byte(cmd, byContrast, true);
	i2c_master_stop(cmd);

	espRc = i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
	if (espRc != ESP_OK)
	{
		ESP_LOGE(TAG_DISPLAY, "Setting contrast failed. code: 0x%.2X", espRc);
	}	
	i2c_cmd_link_delete(cmd);
}

void oled_display_setColor(EOLED_DISPLAY_PIXEL_COLOR eColor)
{
	fs_eDisplayColor = eColor;
}

void oled_display_setTextAlignment(EOLED_DISPLAY_TEXT_ALIGNMENT eTextAlignment) 
{
  fs_eTextAlignment = eTextAlignment;
}

void oled_display_setFont(const char* pszFontData) 
{
  fs_pszTextFont = pszFontData;
}

void oled_display_setFontTableLookupFunction(FontTableLookupFunction function) 
{
  fontTableLookupFunction = function;
}

void fillBuffer(void)
{
	for (WORD i = 0; i < OLED_DISPLAY_BUFFER_SIZE/8; i++)
	{
		fs_abyBuffer[i] = 0x0F;
		/*if (i >= 1)
		{
			break;
		}*/
	}
}

void printfBuffer(void)
{
	for (WORD i = 0; i < OLED_DISPLAY_BUFFER_SIZE/8; i++)
	{
		for(BYTE j = 0; j < 8; j++)
		{
			if ((fs_abyBuffer[i] & (1<<j)) == 0b0)
			{
				printf("0");
			}
			else
			{
				printf("1");
			}
		}
		if((i+1)%8 == 0)
		{
			printf("\n");
		}
	}
	printf("\n\n\n\n\n");
}
 
BOOL oled_display_drawHorizontalLine(BYTE byX, BYTE byY, WORD wLength) 
{
  if (byY >= OLED_DISPLAY_HEIGHT) 
	{ 
		return FALSE; 
	}

	if (wLength > OLED_DISPLAY_WIDTH) 
	{ 
		return FALSE; 
	}

  if ( (byX + wLength) > OLED_DISPLAY_WIDTH) 
	{
    wLength = (OLED_DISPLAY_WIDTH - byX);
  }

	WORD wStartingByte = (byY/8) * OLED_DISPLAY_WIDTH + byX;
	BYTE byDrawingBit = 1 << (byY%8);  // same as "1 << (y & 7)"

	for(BYTE i = 0; i < wLength; i++)
	{
		fs_abyBuffer[wStartingByte + i] |= byDrawingBit;
	}

	switch (fs_eDisplayColor) 
	{
    case eWhite:   
			for(BYTE i = 0; i < wLength; i++)
			{
				fs_abyBuffer[wStartingByte + i] |= byDrawingBit;
			}
			break;
    case eBlack:   
			byDrawingBit = ~byDrawingBit;   
			for(BYTE i = 0; i < wLength; i++)
			{
				fs_abyBuffer[wStartingByte + i] &= byDrawingBit;
			}
			break;
    case eInverse: 
			for(BYTE i = 0; i < wLength; i++)
			{
				fs_abyBuffer[wStartingByte + i] ^= byDrawingBit;
			}
			break;
  }

	/*
	// Could be done this way...
	BYTE * pabyBufferPtr = fs_abyBuffer;
  pabyBufferPtr += (byY >> 3) * OLED_DISPLAY_WIDTH;
  pabyBufferPtr += byX;

  BYTE byDrawBit = 1 << (y & 7);

  switch (fs_eDisplayColor) 
	{
    case eWhite:   
			while (wLength--) 
			{
    	    *pabyBufferPtr++ |= byDrawBit;
    	}; 
			break;
    case eBlack:   
			byDrawBit = ~byDrawBit;   
			while (wLength--) 
			{
        *pabyBufferPtr++ &= byDrawBit;
      }; 
			break;
    case eInverse: 
			while (wLength--) 
			{
        *pabyBufferPtr++ ^= byDrawBit;
      }; 
			break;
  }*/

	return TRUE;
}

BOOL oled_display_drawVerticalLine(BYTE byX, BYTE byY, SHORT sLength)
{
	if (byY >= OLED_DISPLAY_HEIGHT) 
	{ 
		return FALSE; 
	}

	if (byX >= OLED_DISPLAY_WIDTH) 
	{ 
		return FALSE; 
	}

  if ( (byY + sLength) > OLED_DISPLAY_HEIGHT) 
	{
    sLength = (OLED_DISPLAY_HEIGHT - byY);
  }

	WORD wDrawingPosition = (byY/8) * OLED_DISPLAY_WIDTH + byX;
	BYTE byDrawingByte_offset = byY%8;  // same as "y & 7"
	BYTE byDrawingByte;

	if(byDrawingByte_offset)
	{
		byDrawingByte_offset = 8 - byDrawingByte_offset;
		byDrawingByte = ~(0xFF >> (byDrawingByte_offset));
		switch (fs_eDisplayColor) 
		{
      case eWhite:   fs_abyBuffer[wDrawingPosition] |=  byDrawingByte; break;
      case eBlack:   fs_abyBuffer[wDrawingPosition] &= ~byDrawingByte; break;
      case eInverse: fs_abyBuffer[wDrawingPosition] ^=  byDrawingByte; break;
    }
		sLength -= byDrawingByte_offset;

		// if we drew too many bits we need to clean them
		if(sLength < 0)
		{
			byDrawingByte = ~((1 << abs(sLength)) - 1); 
			switch (fs_eDisplayColor) 
			{
    	  case eWhite:   fs_abyBuffer[wDrawingPosition] &=  byDrawingByte; break;
    	  case eBlack:   fs_abyBuffer[wDrawingPosition] |= ~byDrawingByte; break;
    	  case eInverse: fs_abyBuffer[wDrawingPosition] ^=  byDrawingByte; break;
    	}

			return TRUE;
		}

		wDrawingPosition += OLED_DISPLAY_WIDTH;
	}


	if(sLength >= 8)
	{
		switch (fs_eDisplayColor) 
		{
  	  case eBlack:   
  	  case eWhite:   
				byDrawingByte = (fs_eDisplayColor == eWhite) ? 0xFF : 0x00;
				do
				{
					fs_abyBuffer[wDrawingPosition] |= byDrawingByte;
					wDrawingPosition += OLED_DISPLAY_WIDTH;
					sLength -= 8;
				} while(sLength >= 8);
				break;
			case eInverse:
				do
				{
					fs_abyBuffer[wDrawingPosition] |= ~fs_abyBuffer[wDrawingPosition];
					wDrawingPosition += OLED_DISPLAY_WIDTH;
					sLength -= 8;
				} while(sLength >= 8);
				break;
		}
	}

	if (sLength > 0) 
	{
    byDrawingByte = (1 << sLength) - 1;
    switch (fs_eDisplayColor) {
      case eWhite:
				fs_abyBuffer[wDrawingPosition] |= byDrawingByte; 
				break;
      case eBlack:
				fs_abyBuffer[wDrawingPosition] &= ~byDrawingByte; 
				break;
      case eInverse:
				fs_abyBuffer[wDrawingPosition] ^=  byDrawingByte; 
				break;
    }
  }
	

	/*
	// Could be done this way...
  uint8_t yOffset = y & 7;
  uint8_t drawBit;
  uint8_t *bufferPtr = buffer;

  bufferPtr += (y >> 3) * this->width();
  bufferPtr += x;

  if (yOffset) {
    yOffset = 8 - yOffset;
    drawBit = ~(0xFF >> (yOffset));

    if (length < yOffset) {
      drawBit &= (0xFF >> (yOffset - length));
    }

    switch (color) {
      case WHITE:   *bufferPtr |=  drawBit; break;
      case BLACK:   *bufferPtr &= ~drawBit; break;
      case INVERSE: *bufferPtr ^=  drawBit; break;
    }

    if (length < yOffset) return;

    length -= yOffset;
    bufferPtr += this->width();
  }

  if (length >= 8) {
    switch (color) {
      case WHITE:
      case BLACK:
        drawBit = (color == WHITE) ? 0xFF : 0x00;
        do {
          *bufferPtr = drawBit;
          bufferPtr += this->width();
          length -= 8;
        } while (length >= 8);
        break;
      case INVERSE:
        do {
          *bufferPtr = ~(*bufferPtr);
          bufferPtr += this->width();
          length -= 8;
        } while (length >= 8);
        break;
    }
  }

  if (length > 0) {
    drawBit = (1 << (length & 7)) - 1;
    switch (color) {
      case WHITE:   *bufferPtr |=  drawBit; break;
      case BLACK:   *bufferPtr &= ~drawBit; break;
      case INVERSE: *bufferPtr ^=  drawBit; break;
    }
  }*/

	return TRUE;
}

// Bresenham's algorithm - thx wikipedia and Adafruit_GFX
void oled_display_drawLine(SHORT sX0, SHORT sY0, SHORT sX1, SHORT sY1) 
{
  SHORT sSteep = abs(sY1 - sY0) > abs(sX1 - sX0);
  if (sSteep) 
	{
    _SWAP_INT16_T(sX0, sY0);
    _SWAP_INT16_T(sX1, sY1);
  }

  if (sX0 > sX1) 
	{
    _SWAP_INT16_T(sX0, sX1);
    _SWAP_INT16_T(sY0, sY1);
  }

  SHORT sDx, sDy;
  sDx = sX1 - sX0;
  sDy = abs(sY1 - sY0);

  SHORT sErr = sDx / 2;
  SHORT sYstep;

  if (sY0 < sY1) 
	{
    sYstep = 1;
  } 
	else 
	{
    sYstep = -1;
  }

  for (; sX0<=sX1; sX0++) 
	{
    if (sSteep) 
		{
      oled_display_setPixel(sY0, sX0);
    } 
		else 
		{
      oled_display_setPixel(sX0, sY0);
    }
    sErr -= sDy;
    if (sErr < 0) 
		{
      sY0 += sYstep;
      sErr += sDx;
    }
  }
}

void oled_display_drawRect(SHORT sX, SHORT sY, SHORT sWidth, SHORT sHeight) 
{
  oled_display_drawHorizontalLine(sX, sY, sWidth);
  oled_display_drawVerticalLine(sX, sY, sHeight);
  oled_display_drawVerticalLine(sX + sWidth - 1, sY, sHeight);
  oled_display_drawHorizontalLine(sX, sY + sHeight - 1, sWidth);
}

void oled_display_fillRect(SHORT sX, SHORT sY, SHORT sWidth, SHORT sHeight)
{
  for (SHORT sTmp_x = sX; sTmp_x < sX + sWidth; sTmp_x++) 
	{
    oled_display_drawVerticalLine(sTmp_x, sY, sHeight);
  }
}

void oled_display_drawCircle(SHORT sX0, SHORT sY0, SHORT sRadius) 
{
  SHORT sX = 0, sY = sRadius;
	SHORT sDp = 1 - sRadius;
	do 
	{
		if (sDp < 0)
		{
			sDp = sDp + 2 * (++sX) + 3;
		}
		else
		{
			sDp = sDp + 2 * (++sX) - 2 * (--sY) + 5;
		}

		oled_display_setPixel(sX0 + sX, sY0 + sY);     //For the 8 octants
		oled_display_setPixel(sX0 - sX, sY0 + sY);
		oled_display_setPixel(sX0 + sX, sY0 - sY);
		oled_display_setPixel(sX0 - sX, sY0 - sY);
		oled_display_setPixel(sX0 + sY, sY0 + sX);
		oled_display_setPixel(sX0 - sY, sY0 + sX);
		oled_display_setPixel(sX0 + sY, sY0 - sX);
		oled_display_setPixel(sX0 - sY, sY0 - sX);

	} while (sX < sY);

  oled_display_setPixel(sX0 + sRadius, sY0);
  oled_display_setPixel(sX0, sY0 + sRadius);
  oled_display_setPixel(sX0 - sRadius, sY0);
  oled_display_setPixel(sX0, sY0 - sRadius);
}

void oled_display_drawCircleQuads(SHORT sX0, SHORT sY0, SHORT sRadius, BYTE byQuads) 
{
  SHORT sX = 0, sY = sRadius;
  SHORT sDp = 1 - sRadius;
  while (sX < sY) 
	{
    if (sDp < 0)
		{
      sDp = sDp + 2 * (++sX) + 3;
		}
    else
		{
      sDp = sDp + 2 * (++sX) - 2 * (--sY) + 5;
		}
    if (byQuads & 0x1) 
		{
      oled_display_setPixel(sX0 + sX, sY0 - sY);
      oled_display_setPixel(sX0 + sY, sY0 - sX);
    }
    if (byQuads & 0x2) 
		{
      oled_display_setPixel(sX0 - sY, sY0 - sX);
      oled_display_setPixel(sX0 - sX, sY0 - sY);
    }
    if (byQuads & 0x4) 
		{
      oled_display_setPixel(sX0 - sY, sY0 + sX);
      oled_display_setPixel(sX0 - sX, sY0 + sY);
    }
    if (byQuads & 0x8) 
		{
      oled_display_setPixel(sX0 + sX, sY0 + sY);
      oled_display_setPixel(sX0 + sY, sY0 + sX);
    }
  }
  if (byQuads & 0x1 && byQuads & 0x8) 
	{
    oled_display_setPixel(sX0 + sRadius, sY0);
  }
  if (byQuads & 0x4 && byQuads & 0x8) 
	{
    oled_display_setPixel(sX0, sY0 + sRadius);
  }
  if (byQuads & 0x2 && byQuads & 0x4) 
	{
    oled_display_setPixel(sX0 - sRadius, sY0);
  }
  if (byQuads & 0x1 && byQuads & 0x2) 
	{
    oled_display_setPixel(sX0, sY0 - sRadius);
  }
}


void oled_display_fillCircle(SHORT sX0, SHORT sY0, SHORT sRadius) 
{
  SHORT sX = 0, sY = sRadius;
	SHORT sDp = 1 - sRadius;
	do 
	{
		if (sDp < 0)
		{
			sDp = sDp + 2 * (++sX) + 3;
		}
		else
		{
			sDp = sDp + 2 * (++sX) - 2 * (--sY) + 5;
		}

    oled_display_drawHorizontalLine(sX0 - sX, sY0 - sY, 2*sX);
    oled_display_drawHorizontalLine(sX0 - sX, sY0 + sY, 2*sX);
    oled_display_drawHorizontalLine(sX0 - sY, sY0 - sX, 2*sY);
    oled_display_drawHorizontalLine(sX0 - sY, sY0 + sX, 2*sY);


	} while (sX < sY);
  oled_display_drawHorizontalLine(sX0 - sRadius, sY0, 2 * sRadius);
}

void oled_display_drawProgressBar(WORD wX, WORD wY, WORD wWidth, WORD wHeight, BYTE byProgress) 
{
  WORD wRadius = wHeight / 2;
  WORD wXRadius = wX + wRadius;
  WORD wYRadius = wY + wRadius;
  WORD wDoubleRadius = 2 * wRadius;
  WORD wInnerRadius = wRadius - 2;

  oled_display_setColor(eWhite);
  oled_display_drawCircleQuads(wXRadius, wYRadius, wRadius, 0b00000110);
  oled_display_drawHorizontalLine(wXRadius, wY, wWidth - wDoubleRadius + 1);
  oled_display_drawHorizontalLine(wXRadius, wY + wHeight, wWidth - wDoubleRadius + 1);
  oled_display_drawCircleQuads(wX + wWidth - wRadius, wYRadius, wRadius, 0b00001001);

  WORD wMaxProgressWidth = (wWidth - wDoubleRadius + 1) * byProgress / 100;

  oled_display_fillCircle(wXRadius, wYRadius, wInnerRadius);
  oled_display_fillRect(wXRadius + 1, wY + 2, wMaxProgressWidth, wHeight - 3);
  oled_display_fillCircle(wXRadius + wMaxProgressWidth, wYRadius, wInnerRadius);
}

void oled_display_drawFastImage(SHORT sXMove, SHORT sYMove, SHORT sWidth, SHORT sHeight, const BYTE* c_pbImage) 
{
  oled_display_drawInternal(sXMove, sYMove, sWidth, sHeight, c_pbImage, 0, 0);
}

void oled_display_drawXbm(SHORT sXMove, SHORT sYMove, SHORT sWidth, SHORT sHeight, const BYTE* c_pbXbm) 
{
  SHORT sWidthInXbm = (sWidth + 7) / 8;
  BYTE byData = 0;

  for(SHORT sY = 0; sY < sHeight; sY++) 
	{
    for(SHORT sX = 0; sX < sWidth; sX++ ) 
		{
      if (sX & 7) 
			{
        byData >>= 1; // Move a bit
      } 
			else 
			{  // Read new data every 8 bit
        byData = PGM_READ_BYTE(c_pbXbm + (sX / 8) + sY * sWidthInXbm);
      }
      // if there is a bit draw it
      if (byData & 0x01) 
			{
        oled_display_setPixel(sXMove + sX, sYMove + sY);
      }
    }
  }
}

void oled_display_drawString(SHORT sXStart, SHORT sYStart, char* pcText_recieved) 
{
  WORD wLineHeight = PGM_READ_BYTE(fs_pszTextFont + HEIGHT_POS);

  // char* text must be freed!
  char* pcText = oled_display_utf8ascii(pcText_recieved);

  WORD wYOffset = 0;
  // If the string should be centered vertically too
  // we need to now how heigh the string is.
  if (fs_eTextAlignment == eText_align_center_both) 
	{
    WORD wLb = 0;
    // Find number of linebreaks in text
    for (WORD i=0;pcText[i] != 0; i++) 
		{
      wLb += (pcText[i] == 10);
    }
    // Calculate center
    wYOffset = (wLb * wLineHeight) / 2;
  }

  WORD wLine = 0;
  char* pcTextPart = strtok(pcText,"\n");
  while (pcTextPart != NULL) 
	{
    WORD wLength = strlen(pcTextPart);
    oled_display_drawStringInternal(sXStart, sYStart - wYOffset + (wLine++) * wLineHeight, pcTextPart, wLength, oled_display_getStringWidth(pcTextPart, wLength));
    pcTextPart = strtok(NULL, "\n");
  }
  free(pcText);
}

void oled_display_drawStringMaxWidth(SHORT sXMove, SHORT sYMove, WORD wMaxLineWidth, char* pcStrUser)
{
  WORD wFirstChar  = PGM_READ_BYTE(fs_pszTextFont + FIRST_CHAR_POS);
  WORD wLineHeight = PGM_READ_BYTE(fs_pszTextFont + HEIGHT_POS);

  char* pcText = oled_display_utf8ascii(pcStrUser);

  WORD wLength = strlen(pcText);
  WORD wLastDrawnPos = 0;
  WORD wLineNumber = 0;
  WORD wStrWidth = 0;

  WORD wPreferredBreakpoint = 0;
  WORD wWidthAtBreakpoint = 0;

  for (WORD i = 0; i < wLength; i++) 
	{
    wStrWidth += PGM_READ_BYTE(fs_pszTextFont + JUMPTABLE_START + (pcText[i] - wFirstChar) * JUMPTABLE_BYTES + JUMPTABLE_WIDTH);

    // Always try to break on a space or dash
    if (pcText[i] == ' ' || pcText[i]== '-') 
		{
      wPreferredBreakpoint = i;
      wWidthAtBreakpoint = wStrWidth;
    }

    if (wStrWidth >= wMaxLineWidth) 
		{
      if (wPreferredBreakpoint == 0) 
			{
        wPreferredBreakpoint = i;
        wWidthAtBreakpoint = wStrWidth;
      }
      oled_display_drawStringInternal(sXMove, sYMove + (wLineNumber++) * wLineHeight , &pcText[wLastDrawnPos], wPreferredBreakpoint - wLastDrawnPos, wWidthAtBreakpoint);
      wLastDrawnPos = wPreferredBreakpoint + 1;
      // It is possible that we did not draw all letters to i so we need
      // to account for the width of the chars from `i - wPreferredBreakpoint`
      // by calculating the width we did not draw yet.
      wStrWidth = wStrWidth - wWidthAtBreakpoint;
      wPreferredBreakpoint = 0;
    }
  }

  // Draw last part if needed
  if (wLastDrawnPos < wLength) 
	{
    oled_display_drawStringInternal(sXMove, sYMove + wLineNumber * wLineHeight , &pcText[wLastDrawnPos], wLength - wLastDrawnPos, oled_display_getStringWidth(&pcText[wLastDrawnPos], wLength - wLastDrawnPos));
  }

  free(pcText);
}

WORD oled_display_getStringWidth(const char* c_pcText, WORD wLength) 
{
  WORD wFirstChar = PGM_READ_BYTE(fs_pszTextFont + FIRST_CHAR_POS);

  WORD wStringWidth = 0;
  WORD wMaxWidth = 0;

  while (wLength--) 
	{
    wStringWidth += PGM_READ_BYTE(fs_pszTextFont + JUMPTABLE_START + (c_pcText[wLength] - wFirstChar) * JUMPTABLE_BYTES + JUMPTABLE_WIDTH);
    if (c_pcText[wLength] == 10) 
		{
      wMaxWidth = MAX(wMaxWidth, wStringWidth);
      wStringWidth = 0;
    }
  }

  return MAX(wMaxWidth, wStringWidth);
}

//void inline oled_display_drawInternal(SHORT sXMove, SHORT sYMove, SHORT sWidth, SHORT sHeight, const BYTE* c_pbyData, WORD wOffset, WORD wBytesInData) 
static void oled_display_drawInternal(SHORT sXMove, SHORT sYMove, SHORT sWidth, SHORT sHeight, const BYTE* c_pbyData, WORD wOffset, WORD wBytesInData) 
{
  if (sWidth < 0 || sHeight < 0) return;
  if (sYMove + sHeight < 0 || sYMove > OLED_DISPLAY_HEIGHT)  return;
  if (sXMove + sWidth  < 0 || sXMove > OLED_DISPLAY_WIDTH)   return;

  BYTE  byRasterHeight = 1 + ((sHeight - 1) >> 3); // fast ceil(height / 8.0)
  int8_t   yOffset      = sYMove & 7;

  wBytesInData = wBytesInData == 0 ? sWidth * byRasterHeight : wBytesInData;

  SHORT sInitYMove   = sYMove;
  int8_t  initYOffset = yOffset;


  for (WORD i = 0; i < wBytesInData; i++) 
	{
    // Reset if next horizontal drawing phase is started.
    if ( i % byRasterHeight == 0) 
		{
      sYMove   = sInitYMove;
      yOffset = initYOffset;
    }

    BYTE byCurrentByte = PGM_READ_BYTE(c_pbyData + wOffset + i);

    SHORT sXPos = sXMove + (i / byRasterHeight);
    SHORT sYPos = ((sYMove >> 3) + (i % byRasterHeight)) * OLED_DISPLAY_WIDTH;

//    int16_t yScreenPos = yMove + yOffset;
    SHORT sDataPos    = sXPos  + sYPos;

    if (sDataPos >=  0  && sDataPos < OLED_DISPLAY_BUFFER_SIZE/8 &&
        sXPos    >=  0  && sXPos    < OLED_DISPLAY_WIDTH ) 
		{

      if (yOffset >= 0) 
			{
        switch (fs_eDisplayColor) 
				{
          case eWhite:   fs_abyBuffer[sDataPos] |= byCurrentByte << yOffset; break;
          case eBlack:   fs_abyBuffer[sDataPos] &= ~(byCurrentByte << yOffset); break;
          case eInverse: fs_abyBuffer[sDataPos] ^= byCurrentByte << yOffset; break;
        }

        if (sDataPos < (OLED_DISPLAY_BUFFER_SIZE/8 - OLED_DISPLAY_WIDTH)) 
				{
          switch (fs_eDisplayColor) 
					{
            case eWhite:   fs_abyBuffer[sDataPos + OLED_DISPLAY_WIDTH] |= byCurrentByte >> (8 - yOffset); break;
            case eBlack:   fs_abyBuffer[sDataPos + OLED_DISPLAY_WIDTH] &= ~(byCurrentByte >> (8 - yOffset)); break;
            case eInverse: fs_abyBuffer[sDataPos + OLED_DISPLAY_WIDTH] ^= byCurrentByte >> (8 - yOffset); break;
          }
        }
      } 
			else 
			{
        // Make new offset position
        yOffset = -yOffset;

        switch (fs_eDisplayColor) 
				{
          case eWhite:   fs_abyBuffer[sDataPos] |= byCurrentByte >> yOffset; break;
          case eBlack:   fs_abyBuffer[sDataPos] &= ~(byCurrentByte >> yOffset); break;
          case eInverse: fs_abyBuffer[sDataPos] ^= byCurrentByte >> yOffset; break;
        }

        // Prepare for next iteration by moving one block up
        sYMove -= 8;

        // and setting the new yOffset
        yOffset = 8 - yOffset;
      }

      //yield();
			esp_task_wdt_reset();
    }
  }
}

static void oled_display_drawStringInternal(SHORT sXMove, SHORT sYMove, char* pcText, WORD wTextLength, WORD wTextWidth) 
{
  BYTE byTextHeight       = PGM_READ_BYTE(fs_pszTextFont + HEIGHT_POS);
  BYTE byFirstChar        = PGM_READ_BYTE(fs_pszTextFont + FIRST_CHAR_POS);
  WORD wSizeOfJumpTable = PGM_READ_BYTE(fs_pszTextFont + CHAR_NUM_POS)  * JUMPTABLE_BYTES;

  BYTE byCursorX         = 0;
  BYTE byCursorY         = 0;

  switch (fs_eTextAlignment) 
	{
    case eText_align_center_both:
      sYMove -= byTextHeight >> 1;
    // Fallthrough
    case eText_align_center:
      sXMove -= wTextWidth >> 1; // divide by 2
      break;
    case eText_align_right:
      sXMove -= wTextWidth;
      break;
    case eText_align_left:
      break;
  }

  // Don't draw anything if it is not on the screen.
  if (sXMove + wTextWidth  < 0 || sXMove > OLED_DISPLAY_WIDTH ) {return;}
  if (sYMove + byTextHeight < 0 || sYMove > OLED_DISPLAY_WIDTH ) {return;}

  for (WORD j = 0; j < wTextLength; j++) 
	{
    SHORT sXPos = sXMove + byCursorX;
    SHORT sYPos = sYMove + byCursorY;

    BYTE byCode = pcText[j];
    if (byCode >= byFirstChar) 
		{
      BYTE byCharCode = byCode - byFirstChar;

      // 4 Bytes per char code
      BYTE byMsbJumpToChar    = PGM_READ_BYTE( fs_pszTextFont + JUMPTABLE_START + byCharCode * JUMPTABLE_BYTES );                  // MSB  \ JumpAddress
      BYTE byLsbJumpToChar    = PGM_READ_BYTE( fs_pszTextFont + JUMPTABLE_START + byCharCode * JUMPTABLE_BYTES + JUMPTABLE_LSB);   // LSB /
      BYTE byCharByteSize     = PGM_READ_BYTE( fs_pszTextFont + JUMPTABLE_START + byCharCode * JUMPTABLE_BYTES + JUMPTABLE_SIZE);  // Size
      BYTE byCurrentCharWidth = PGM_READ_BYTE( fs_pszTextFont + JUMPTABLE_START + byCharCode * JUMPTABLE_BYTES + JUMPTABLE_WIDTH); // Width

      // Test if the char is drawable
      if (!(byMsbJumpToChar == 255 && byLsbJumpToChar == 255)) 
			{
        // Get the position of the char data
        WORD wCharDataPosition = JUMPTABLE_START + wSizeOfJumpTable + ((byMsbJumpToChar << 8) + byLsbJumpToChar);
        oled_display_drawInternal(sXPos, sYPos, byCurrentCharWidth, byTextHeight, (BYTE*)fs_pszTextFont, wCharDataPosition, byCharByteSize);
      }

      byCursorX += byCurrentCharWidth;
    }
  }
}

// You need to free the char!
char* oled_display_utf8ascii(char* pcStr) 
{
  WORD wK = 0;
  //uint16_t wLength = pcStr.length() + 1;
  WORD wLength = strlen(pcStr) + 1;

  // Copy the string into a char array
  char* pcS = (char*) malloc(wLength * sizeof(char));
  if(!pcS) 
	{
    ESP_LOGI(TAG_DISPLAY, "[utf8ascii] Can't allocate another char array. Drop support for UTF-8.\n");
    //return (char*) pcStr.c_str();
    return pcStr;
  }
  //pcStr.toCharArray(pcS, wLength);
	strcpy(pcS, pcStr);

  wLength--;

  for (WORD i=0; i < wLength; i++) 
	{
    char c = (*fontTableLookupFunction)(pcS[i]);
    if (c!=0) 
		{
      pcS[wK++]=c;
    }
  }

  pcS[wK]=0;

  // This will leak 's' be sure to free it in the calling function.
  return pcS;
}

void oled_display_setPixel(SHORT sX, SHORT sY) 
{
  if (sX < OLED_DISPLAY_WIDTH && sY < OLED_DISPLAY_HEIGHT) 
  {
    switch (fs_eDisplayColor) 
    {
      case eWhite:   fs_abyBuffer[sX + (sY/8) * OLED_DISPLAY_WIDTH] |=  (1 << (sY & 7)); break;
      case eBlack:   fs_abyBuffer[sX + (sY/8) * OLED_DISPLAY_WIDTH] &= ~(1 << (sY & 7)); break;
      case eInverse: fs_abyBuffer[sX + (sY/8) * OLED_DISPLAY_WIDTH] ^=  (1 << (sY & 7)); break;
    }
  }
}

// UTF-8 to font table index converter
// Code form http://playground.arduino.cc/Main/Utf8ascii
//FontTableLookupFunction fontTableLookupFunction = [](const byte ch) 
BYTE oled_display_fontTableLookupFunction_test(const BYTE c_byCh) 
{
  static BYTE s_byLASTCHAR;

  if (c_byCh < 128)  // Standard ASCII-set 0..0x7F handling
  { 
    s_byLASTCHAR = 0;
    return c_byCh;
  }

  BYTE byLast = s_byLASTCHAR;   // get last char
  s_byLASTCHAR = c_byCh;

  switch (byLast)  // conversion depnding on first UTF8-character
  {    
    case 0xC2: return (uint8_t) c_byCh;
    case 0xC3: return (uint8_t) (c_byCh | 0xC0);
    case 0x82: if (c_byCh == 0xAC) return (uint8_t) 0x80;    // special case Euro-symbol
  }

  return (uint8_t) 0; // otherwise: return zero, if character has to be ignored
}

void oled_display_clearBuffer(void) 
{
  memset(fs_abyBuffer, 0, OLED_DISPLAY_BUFFER_SIZE/8);
}

void oled_display_configure(void)
{
	i2c_cmd_handle_t cmd;
	esp_err_t espRc;
  BYTE byX_offset = 0;

	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
	i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_STREAM, true);

  // Setting horizontal addressing mode
  i2c_master_write_byte(cmd, OLED_CMD_SET_MEMORY_ADDR_MODE, true);
  i2c_master_write_byte(cmd, SSD1306_HORIZONTAL_ADDRESSING_MODE, true);

  // Specifie column start address and end address of the display data RAM
  i2c_master_write_byte(cmd, OLED_CMD_SET_COLUMN_RANGE, true);
  i2c_master_write_byte(cmd, byX_offset, true);
  i2c_master_write_byte(cmd, byX_offset + (OLED_DISPLAY_WIDTH - 1), true);

  // Specifie page start address and end address of the display data RAM
  i2c_master_write_byte(cmd, OLED_CMD_SET_PAGE_RANGE, true);
  i2c_master_write_byte(cmd, 0x0, true);
  i2c_master_write_byte(cmd, (OLED_DISPLAY_HEIGHT - 1)/8, true);

	i2c_master_stop(cmd);
	espRc = i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
	if (espRc != ESP_OK) 
	{
		ESP_LOGE(TAG_DISPLAY, "Sending buffer to display (setting commands)");
	}
	i2c_cmd_link_delete(cmd);
}

void oled_display_send_buffer(void)
{
  i2c_cmd_handle_t cmd;
	esp_err_t espRc;

  // Send buffer
	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
	i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_DATA_STREAM, true);
	i2c_master_write(cmd, fs_abyBuffer, OLED_DISPLAY_BUFFER_SIZE/8, true);
	i2c_master_stop(cmd);
  espRc = i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
	if (espRc != ESP_OK) 
	{
		ESP_LOGE(TAG_DISPLAY, "Sending buffer to display (buffer sending error)");
	}
	i2c_cmd_link_delete(cmd);
}

/*
// some base for scrolling... check pdf for more info...
void ssd1306_scroll(void) 
{
	esp_err_t espRc;

	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);

	i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
	i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_STREAM, true);

	i2c_master_write_byte(cmd, 0x29, true); // vertical and horizontal scroll (p29)
	i2c_master_write_byte(cmd, 0x00, true);
	i2c_master_write_byte(cmd, 0x00, true);
	i2c_master_write_byte(cmd, 0x07, true);
	i2c_master_write_byte(cmd, 0x01, true);
	i2c_master_write_byte(cmd, 0x3F, true);

	i2c_master_write_byte(cmd, 0xA3, true); // set vertical scroll area (p30)
	i2c_master_write_byte(cmd, 0x20, true);
	i2c_master_write_byte(cmd, 0x40, true);

	i2c_master_write_byte(cmd, 0x2F, true); // activate scroll (p29)

	i2c_master_stop(cmd);
	espRc = i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
	if (espRc == ESP_OK) 
  {
		ESP_LOGI(TAG_DISPLAY, "Scroll command succeeded");
	} 
  else 
  {
		ESP_LOGE(TAG_DISPLAY, "Scroll command failed. code: 0x%.2X", espRc);
	}

	i2c_cmd_link_delete(cmd);
}*/
