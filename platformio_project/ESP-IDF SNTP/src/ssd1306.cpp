/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 by ThingPulse, Daniel Eichhorn
 * Copyright (c) 2018 by Fabrice Weinberg
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * ThingPulse invests considerable time and money to develop these open source libraries.
 * Please support us by buying our products (and not the clones) from
 * https://thingpulse.com
 *
 */

 // Include the correct display library
 // For a connection via I2C using Wire include
 #include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
 #include "SSD1306Wire.h" // legacy include: `#include "SSD1306.h"`
 // or #include "SH1106Wire.h", legacy include: `#include "SH1106.h"`
 // For a connection via I2C using brzo_i2c (must be installed) include
 // #include <brzo_i2c.h> // Only needed for Arduino 1.6.5 and earlier
 // #include "SSD1306Brzo.h"
 // #include "SH1106Brzo.h"
 // For a connection via SPI include
 // #include <SPI.h> // Only needed for Arduino 1.6.5 and earlier
 // #include "SSD1306Spi.h"
 // #include "SH1106SPi.h"

// Include the UI lib
#include "OLEDDisplayUi.h"

// Include custom images

#include "images.h"
#include "formosa.h"

#include <tcpip_adapter.h>
#include <esp_log.h>
//#include <stdio.h>
#include <sys/time.h>
// Use the corresponding display class:

// Initialize the OLED display using SPI
// D5 -> CLK
// D7 -> MOSI (DOUT)
// D0 -> RES
// D2 -> DC
// D8 -> CS
// SSD1306Spi        display(D0, D2, D8);
// or
// SH1106Spi         display(D0, D2);

// Initialize the OLED display using brzo_i2c
// D3 -> SDA
// D5 -> SCL
// SSD1306Brzo display(0x3c, D3, D5);
// or
// SH1106Brzo  display(0x3c, D3, D5);

// Initialize the OLED display using Wire library
//SSD1306Wire  display(0x3c, D3, D5); //SDA, SCL
SSD1306Wire  display(0x3c, GPIO_NUM_5, GPIO_NUM_4);

// SH1106Wire display(0x3c, D3, D5);

OLEDDisplayUi ui  ( &display );

void msOverlay(OLEDDisplay *display, OLEDDisplayUiState* state) {
  display->setTextAlignment(TEXT_ALIGN_RIGHT);
  display->setFont(ArialMT_Plain_10);
  int iSec = millis()/1000;
  char buf[64];
  sprintf(buf, "%02d:%02d:%02d", iSec/(60*60), (iSec/60)%60, iSec%60);
  display->drawString(128, 0, buf);
}

extern RTC_DATA_ATTR tcpip_adapter_ip_info_t ipInfo;

void drawFrame1(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  // draw an xbm image.
  // Please note that everything that should be transitioned
  // needs to be drawn relative to x and y
  display->drawXbm(x, y + 14, WiFi_Logo_width, WiFi_Logo_height, WiFi_Logo_bits);

  if (&ipInfo != NULL) { //&ipInfo got valid data?
    display->setTextAlignment(TEXT_ALIGN_RIGHT);
    display->setFont(ArialMT_Plain_10);
    display->drawString(x+128, y+25, ip4addr_ntoa(&ipInfo.ip));
    }
/*
  display->setTextAlignment(TEXT_ALIGN_RIGHT);
  display->setFont(ArialMT_Plain_16);

  tcpip_adapter_ip_info_t ipInfo; 
  tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_STA, &ipInfo);
  //char buf[64];
  //sprintf(buf, "%X.%X.%X.%X", ip4_addr1_16(&ipInfo.ip.addr), ip4_addr2_16(&ipInfo.ip.addr), ip4_addr3_16(&ipInfo.ip.addr), ip4_addr4_16(&ipInfo.ip.addr));
  //display->drawString(128, y, buf);
  String sIP;
  sIP += IP2STR(&ipInfo.ip);
  display->drawString(64, y, sIP.c_str());
*/
}

void drawFrame2(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  int iX = Formosa_width/2;
  display->drawXbm(x-16, y , Formosa_width, Formosa_height, Formosa_bits);

  time_t t1 = time(NULL);
  struct tm *nPtr = localtime(&t1);
  int t_year = nPtr->tm_year + 1900;
  int t_month= nPtr->tm_mon + 1;
  int t_mday = nPtr->tm_mday;
  int t_hour = nPtr->tm_hour;
  int t_min  = nPtr->tm_min;
  int t_sec  = nPtr->tm_sec;

  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  display->drawString(iX + x, 10 + y, "Taiwan");

  display->setFont(ArialMT_Plain_16);
  char buf[32];
  sprintf(buf,"%4d-%02d-%02d", t_year, t_month, t_mday);
  display->drawString(iX + x, 20 + y, buf);

  display->setFont(ArialMT_Plain_24);
  sprintf(buf,"%02d:%02d:%02d", t_hour, t_min, t_sec);
  display->drawString(iX + x, 34 + y, buf);
}

// This array keeps function pointers to all frames
// frames are the single views that slide in
FrameCallback frames[] = { drawFrame1, drawFrame2 };

// how many frames are there?
int frameCount = 2;

// Overlays are statically drawn on top of a frame eg. a clock
OverlayCallback overlays[] = { msOverlay };
int overlaysCount = 1;

void setupSSD() {
	// The ESP is capable of rendering 60fps in 80Mhz mode
	// but that won't give you much time for anything else
	// run it in 160Mhz mode or just set it to 30 fps
  ui.setTargetFPS(10);
  ui.setTimePerFrame(500);

	// Customize the active and inactive symbol
  ui.setActiveSymbol(activeSymbol);
  ui.setInactiveSymbol(inactiveSymbol);

  // You can change this to
  // TOP, LEFT, BOTTOM, RIGHT
  ui.setIndicatorPosition(BOTTOM);

  // Defines where the first frame is located in the bar.
  ui.setIndicatorDirection(LEFT_RIGHT);

  // You can change the transition that is used
  // SLIDE_LEFT, SLIDE_RIGHT, SLIDE_UP, SLIDE_DOWN
  ui.setFrameAnimation(SLIDE_LEFT);

  // Add frames
  ui.setFrames(frames, frameCount);

  // Add overlays
  ui.setOverlays(overlays, overlaysCount);

  // Initialising the UI will init the display too.
  ui.init();
  display.flipScreenVertically();

  display.drawString(32, 48, "(c)JimmyCraft");
  display.display();
}
