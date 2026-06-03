#include <Wire.h>                 // Built-in library that handles I2C communication
#include <LiquidCrystal_I2C.h>    //installed library to make this easier

LiquidCrystal_I2C lcd(0x27, 16, 2); //adress, columns, rows, max 32 characters

int counter = 10000000;
int length = 0;
int start = 8;

void setup() {
  lcd.init(); //you'll never believe it
  lcd.backlight(); //make text legible

  lcd.setCursor(0, 0); //column, row
  lcd.print("PENIS");

}

void loop() {
  lcd.clear();
  length = log10(counter) + 1;
  if (length%2 > 0) {
    start = (length - 1) / 2;
  }
  else {
    start = length / 2;
  }
  lcd.setCursor((8 - start), 0);
  lcd.print(counter);
  counter++;
  delay(100);
}
