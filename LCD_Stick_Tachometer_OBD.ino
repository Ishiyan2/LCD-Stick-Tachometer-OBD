//MCP2515
#include <SPI.h>
#include <mcp_can.h>
#define CAN0_INT 3                                                    // Set INT to pin 3
MCP_CAN CAN0(10);                                                     // Set CS to pin 10
long unsigned int rxId;
unsigned char len = 0;
unsigned char rxBuf[8];                                               // Array to store serial string
byte data1[8] = {0x02, 0x01, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00};     // Request ENGINE Speed

#define LED 6                                                         // LED Stick Control 
#include <Adafruit_NeoPixel.h>
Adafruit_NeoPixel strip = Adafruit_NeoPixel(8, LED, NEO_GRB + NEO_KHZ800);
#define MAX_VAL 15                                                    // 0 to 255 for brightness
#define WAN_VAL 40                                                    // Warnig for brightness

unsigned long timeA = 0;
unsigned long rev = 0;
unsigned long rev_old = 0;
unsigned long hysteresis = 100;

void setup(){
  strip.begin();
  rainbowCycle();
  colorWipe(strip.Color(0, 0, 0)); // Black
  
  pinMode(CAN0_INT, INPUT);                        // Configuring pin for /INT input
  // Initialize MCP2515 running at 16MHz with a baudrate of 500kb/s and the masks and filters disabled.
  if(CAN0.begin(MCP_STDEXT, CAN_500KBPS, MCP_8MHZ) == CAN_OK) {
    //pass
  }
  CAN0.init_Mask(0,0,0x1F800000);                                       // Init first mask
  CAN0.init_Mask(1,0,0x1F800000);                                       // Init second mask
  for ( int i=0 ; i < 6 ; i++ ) {
    CAN0.init_Filt(i,0,0x1FA00000);                                     // Init filters
  }
  CAN0.setMode(MCP_NORMAL);                                             // Set operation mode to normal so the MCP2515 sends acks to received data.
}

void loop() {
  while(millis() < timeA + 200) { }
  timeA=millis();
  byte sndStat = CAN0.sendMsgBuf(0x7E0, 0, 8, data1);
  while (millis() <= timeA+50 ) {
    if(!digitalRead(CAN0_INT)) {
      CAN0.readMsgBuf(&rxId, &len, rxBuf);   
      if ( rxId==0x7E8 && rxBuf[2]==0x0C ) {
        rev_old = rev;
        rev=int((rxBuf[3]*256+rxBuf[4])/4);
      }
    }
  }
  if ( rev > rev_old ) {
    rev_old = rev;
  } else if (rev  > rev_old - hysteresis ) {
    rev = rev_old;
  } else {
    rev_old = rev; 
  }
  flush_LED();
}

void flush_LED() {
    if (rev > 7000 ) {
      //pass
    } else if (rev > 5000)  {
      colorWipe(strip.Color(WAN_VAL, 0, 0));        // Red Warning
      delay(100);
      colorWipe(strip.Color(0, 0, 0));              // Black
      delay(100);
    } else {
      for ( int i = 0 ; i<8 ; i++ ) {
        strip.setPixelColor(i, strip.Color(0, 0, 0));
      }
      if (rev>=   0) { strip.setPixelColor(0, strip.Color(0, MAX_VAL, 0));  }
      if (rev>=1000) { strip.setPixelColor(1, strip.Color(0, MAX_VAL, 0));  }
      if (rev>=1500) { strip.setPixelColor(2, strip.Color(MAX_VAL, MAX_VAL, 0)); }
      if (rev>=2000) { strip.setPixelColor(3, strip.Color(MAX_VAL, MAX_VAL, 0)); }
      if (rev>=2500) { strip.setPixelColor(4, strip.Color(MAX_VAL, MAX_VAL/2, 0));  }
      if (rev>=3000) { strip.setPixelColor(5, strip.Color(MAX_VAL, MAX_VAL/2, 0));  }
      if (rev>=3500) { strip.setPixelColor(6, strip.Color(WAN_VAL, 0, 0));  }
      if (rev>=4000) { strip.setPixelColor(7, strip.Color(WAN_VAL, 0, 0));  }
      strip.show();
    }
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
      strip.show();
  }
}

void rainbow() {
  uint16_t i, j;
  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle() {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color((255 - WheelPos * 3) * MAX_VAL/255, 0, (WheelPos * 3) * MAX_VAL/255);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3 * MAX_VAL/255, (255 - WheelPos * 3) * MAX_VAL/255);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3 * MAX_VAL/255, (255 - WheelPos * 3) * MAX_VAL/255 , 0);
}
