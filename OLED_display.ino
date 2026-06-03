#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h> //library type stuff

#define SCREEN_WIDTH 128 // width
#define SCREEN_HEIGHT 32 // height

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1); //the -1 means that its sucky and doesn't have a reset pin

void setup() {
  // Initialize with the I2C address 0x3C (standard for almost all mini OLEDs)
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.begin(9600);
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); //dope ahh, basically a while (true), can't be broken, stops explosions
  }

  display.clearDisplay(); //apparently you always wanna clear whatever is there first

  display.setTextSize(1);             // point 1 font, it's a small screen
  display.setTextColor(SSD1306_WHITE); // white text
  

  display.setTextSize(2);             // Double size font
  display.setCursor(32, 10);            // wdith, height with top left being 0,0. positive y is down
  display.print("Hello");

  display.display(); //the library doesn't display until you tell it to

}

void loop() {
  // put your main code here, to run repeatedly:

}
