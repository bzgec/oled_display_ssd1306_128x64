#include "driver/gpio.h"
#include "driver/i2c.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/task.h"

#include "main.h"
#include "OLEDDisplay.h"
#include <string.h>


void app_main(void)
{
	i2c_master_init(SDA_PIN, SCL_PIN);
	oled_display_init();

  while(1)
  {
		oled_display_clearBuffer();
		if(oled_display_drawHorizontalLine(0, 0, 128) == FALSE)
		{
			printf("ERROR DISPLAYING HORIZONTAL LINE\n");
		}
		if(oled_display_drawHorizontalLine(0, 63, 128) == FALSE)
		{
			printf("ERROR DISPLAYING HORIZONTAL LINE\n");
		}
		if(oled_display_drawVerticalLine(0, 0, 64) == FALSE)
		{
			printf("ERROR DISPLAYING VERTICAL LINE\n");
		}
		if(oled_display_drawVerticalLine(127, 0, 64) == FALSE)
		{
			printf("ERROR DISPLAYING VERTICAL LINE\n");
		}
		if(oled_display_drawVerticalLine(126, 0, 4) == FALSE)
		{
			printf("ERROR DISPLAYING VERTICAL LINE\n");
		}
		if(oled_display_drawVerticalLine(125, 0, 3) == FALSE)
		{
			printf("ERROR DISPLAYING VERTICAL LINE\n");
		}
		if(oled_display_drawVerticalLine(124, 0, 2) == FALSE)
		{
			printf("ERROR DISPLAYING VERTICAL LINE\n");
		}
		oled_display_setTextAlignment(eText_align_center_both);
		char caDisplayStringBuffer[30];
		for(BYTE i=0; i<=100; i++)
		{
			oled_display_clearBuffer();
			sprintf(caDisplayStringBuffer, "Uploading process: %3d%%", i);
			oled_display_drawString(OLED_DISPLAY_WIDTH/2, OLED_DISPLAY_HEIGHT/2, caDisplayStringBuffer);
			oled_display_drawProgressBar(4, 40, 120, 15, i);
			oled_display_send_buffer();
			//vTaskDelay(25/portTICK_PERIOD_MS);
			ets_delay_us(25*1000);
		}
		oled_display_send_buffer();
		oled_display_setTextAlignment(eText_align_left);
		//vTaskDelay(1000/portTICK_PERIOD_MS);
		ets_delay_us(1000*1000);

		oled_display_clearBuffer();
		oled_display_drawCircle(30, 30, 20);
		oled_display_fillCircle(30, 30, 10);
		oled_display_drawCircleQuads(30, 30, 25, 0x1);
		oled_display_drawCircleQuads(30, 30, 25, 0x4);
		oled_display_drawRect(40,40,20,20);
		oled_display_fillRect(45,45,10,10);
		for(BYTE i=0; i < OLED_DISPLAY_HEIGHT; i++)
		{
			oled_display_setPixel((OLED_DISPLAY_WIDTH - OLED_DISPLAY_HEIGHT) + i, i);
		}
		for(BYTE i=0; i < OLED_DISPLAY_HEIGHT; i++)
		{
			oled_display_setPixel((OLED_DISPLAY_WIDTH - OLED_DISPLAY_HEIGHT) + i, OLED_DISPLAY_HEIGHT - i);
		}
		oled_display_send_buffer();
		//vTaskDelay(2000/portTICK_PERIOD_MS);
		ets_delay_us(2000*1000);
		
		oled_display_clearBuffer();
		oled_display_drawStringMaxWidth(0, 0, 127, "Testiram kr neki, da vidim maxWidth() funkcijo :D");
		oled_display_setFont(ArialMT_Plain_16);
		oled_display_drawStringMaxWidth(0, 22, 127, "ArialMT_Plain_16");
		oled_display_setFont(ArialMT_Plain_24);
		oled_display_drawStringMaxWidth(0, 38, 127, "ArialMT_Plain_24");
		oled_display_send_buffer();
		//vTaskDelay(2000/portTICK_PERIOD_MS);
		ets_delay_us(2000*1000);
		oled_display_setFont(ArialMT_Plain_10);

		oled_display_sendCommand(OLED_CMD_DISPLAY_OFF);
		ets_delay_us(500*1000);
		oled_display_sendCommand(OLED_CMD_DISPLAY_ON);
		ets_delay_us(500*1000);
		oled_display_sendCommand(OLED_CMD_DISPLAY_INVERTED);
		ets_delay_us(500*1000);
		oled_display_sendCommand(OLED_CMD_DISPLAY_NORMAL);
		ets_delay_us(500*1000);
		oled_display_sendCommand(OLED_CMD_DISPLAY_ALLON);  // after this command, if you want to display normal OLED_CMD_DISPLAY_RAM must be sent to display (for ssd1306)
		ets_delay_us(500*1000);
		oled_display_sendCommand(OLED_CMD_DISPLAY_RAM);
		ets_delay_us(500*1000);
		
		oled_display_clearBuffer();
		oled_display_send_buffer();
		ets_delay_us(1000*1000);
  }
}
