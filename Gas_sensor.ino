#include <math.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <LiquidCrystal_I2C.h>

const int gas = A1;
const float RL = 1.0;
const float Ro = 2.5;
const float a = 574.25;
const float b = -2.222;

#define SCREEN_WIDTH 128 // width
#define SCREEN_HEIGHT 64 // height

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.begin(9600);
    Serial.println("Double check wiring to OLED display, Arduino can't see it");
    for(;;);
  }
  Serial.begin(9600);
  Serial.println("Warming up"); //needs to get to like 400 celsius

  lcd.init();
  lcd.backlight();
  lcd.clear();

  display.clearDisplay();
  display.display();
}

void loop() {
  float rawValue = analogRead(gas);
  
  float voltage = (rawValue / 1023.0) * 5.0;
  
  float Rs = RL * ((1023.0 - rawValue) / rawValue); //sensor resistance which can be derived from ohm's law

  float Rt = Rs / Ro;

  float ppm = a * pow(Rt, b); //derived using basic algebra from a previously defined and solved for equation by other researchers where most of these values came from

  lcd.setCursor(0, 0);

  if (ppm < 5) {
    lcd.clear();
    lcd.print(ppm);
    lcd.setCursor(0, 1);
    lcd.print("Safe");
    display.clearDisplay();
    display.display();
    delay(500);
  }
  else{
    lcd.clear();
    display.clearDisplay();
    lcd.setCursor(0, 1);
    lcd.print("GAS PRESENT");
    display.display();
    display.invertDisplay(true);
    display.drawLine(display.width()-1, display.height()-1, 0, 0, SSD1306_WHITE);
    display.drawLine(display.width()-1, 0, 0, display.height()-1, SSD1306_WHITE);
    display.display();
    delay(500);
  }
  display.invertDisplay(false);
  delay(500);
}