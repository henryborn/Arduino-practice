//I am doing this project with no help from AI or anything. This is to test what I have learned and what I can figure out

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <LiquidCrystal_I2C.h>

const int trig = 9;
const int echo = 10;
const int led = 3;
double duration;
double distance;
double speedOfSound = .0343;
double circleRadius;
double LEDBlink;
int timer = 0;
bool on = false;

#define SCREEN_WIDTH 128 // width
#define SCREEN_HEIGHT 64 // height

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1); //the -1 means that its sucky and doesn't have a reset pin
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.begin(9600);
    Serial.println("Double check wiring to OLED display, Arduino can't see it");
    for(;;);
  }
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(2);
  display.print("Starting");
  display.display();

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Starting");

  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);
  pinMode(led, OUTPUT); //gotta remember it's from the arduino's perspective
}

void loop() {
  display.clearDisplay();
  //display.drawRect(display.width() / 2, display.height() / 2, display.width(), display.height(), SSD1306_WHITE);

  digitalWrite(trig, LOW);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);

  duration = pulseIn(echo, HIGH);
  distance = duration * speedOfSound / 2;

  if (distance < 10) {
      display.invertDisplay(true);
      if (circleRadius != 0) {
      display.fillCircle(display.width() / 2, display.height() / 2, circleRadius, SSD1306_WHITE);
      }
  }
  if (distance < 50) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(distance);
    lcd.setCursor(0, 1);
    lcd.print("cm away");
    if (distance > 10) {
      display.invertDisplay(false);
      if (circleRadius != 0) {
      display.fillCircle(display.width() / 2, display.height() / 2, circleRadius, SSD1306_BLACK);
      }
    }
    circleRadius = map(distance, 0, 50,  32, 0);
    display.fillCircle(display.width() / 2, display.height() / 2, circleRadius, SSD1306_WHITE);
  }
  else {
    lcd.clear();
    lcd.print("Out of Range");
    display.invertDisplay(false);
    display.clearDisplay();
  }
  LEDBlink = map(distance, 0, 50, 100, 1000);
  if (timer < LEDBlink) {
    if (on) {
      digitalWrite(led, HIGH);
    }
    else {
      digitalWrite(led, LOW);
    }
  }
  else if (timer > LEDBlink) {
    timer = 0;
    if (on) {
      digitalWrite(led, LOW);
      on = false;
    }
    else {
      digitalWrite(led, HIGH);
      on = true;
    }
  }
  delay(50);
  timer += 50;
  display.display();
}

//https://github.com/adafruit/Adafruit_SSD1306/blob/master/examples/ssd1306_128x32_i2c/ssd1306_128x32_i2c.ino is the repo I used to help write this
