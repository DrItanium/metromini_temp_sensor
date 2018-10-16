/*
metromini_temp_sensor
Copyright (c) 2018, Joshua Scoggins 
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR 
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/* sensor suite */
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <dht11.h>
#include <Adafruit_Sensor.h>


// comment next line to enable serial console output
#define NO_SERIAL

// If using software SPI (the default case):
#define OLED_MOSI   9
#define OLED_CLK   10
#define OLED_DC    11
#define OLED_CS    12
#define OLED_RESET 13
#define BUTTON_PIN 2

/* Uncomment this block to use hardware SPI
  #define OLED_DC     6
  #define OLED_CS     7
  #define OLED_RESET  8
  Adafruit_SSD1306 display(OLED_DC, OLED_RESET, OLED_CS);
*/

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2

#define LOGO16_GLCD_HEIGHT 16
#define LOGO16_GLCD_WIDTH  16


#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif
#define DS18B20_PIN 7
OneWire oneWire(DS18B20_PIN);
DallasTemperature sensors(&oneWire);
Adafruit_SSD1306 display(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

dht11 DHT;
#define POTENT_PIN A0
bool useCelsius = true;

const char humidityFmt[] PROGMEM = "Humidity: %d%%";
const char temperatureFmt[] PROGMEM = "Temp: %s C";
const char temperatureFmtF[] PROGMEM = "Temp: %s F";
const char potentiometerValue[] PROGMEM = "Sampling Rate: %dms";
const char* const lines[] PROGMEM = { humidityFmt, temperatureFmt, temperatureFmtF, potentiometerValue };

char floatTmp[32];
char line[32];
char fmt[32];
int prevHum = 0;
float prevTemp = 0.0f;
int prevPotent = 100;


void delayThenClear(int delayAmount = 2000) {
  delay(delayAmount);
  display.clearDisplay();
}

void printScreen(const char* string, int delayAmount = 2000) {
  display.setCursor(0, 0);
  display.println(string);
  display.display();
  delayThenClear(delayAmount);
}

int getDHT11Pin() {
  return 8;
}
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
#ifndef NO_SERIAL
  Serial.begin(9600);
  Serial.println("Bringing up display");
#endif
  display.begin(SSD1306_SWITCHCAPVCC);
  display.setTextSize(1);
  display.setTextColor(WHITE);

  printScreen("");
  sensors.begin();  
  pinMode(BUTTON_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), changeTempFormat, FALLING);
}
void waitForStable() {
  int chk = 0;
  do {
    chk = DHT.read(getDHT11Pin());
  } while(chk != DHTLIB_OK);
}

void outputLine(const char* line) {
#ifndef NO_SERIAL
  Serial.println(line);
#endif
  display.println(line);
}


void generateLineEntry(int index, int value)  {
  strcpy_P(fmt, (char*)pgm_read_word(&(lines[index])));  
  sprintf(line, fmt, value);
  outputLine(line);
}

void generateLineEntry(int index, float value) {
  strcpy_P(fmt, (char*)pgm_read_word(&(lines[index])));  
  sprintf(line, fmt, dtostrf(value, 8, 2, floatTmp));
  outputLine(line);
}
void changeTempFormat() {
  useCelsius = !useCelsius;
}

int computeSamplingRate(int value) {
  return value + 499;
}

// the loop function runs over and over again forever
void loop() {  
  waitForStable();  
  sensors.requestTemperatures();
  auto newHum = DHT.humidity;
  auto newTemp = useCelsius ? sensors.getTempCByIndex(0) : sensors.getTempFByIndex(0);
  auto newPotent = analogRead(POTENT_PIN);
  if (newHum != prevHum || newTemp != prevTemp || newPotent != prevPotent) {
    display.setTextSize(1);
    display.setTextColor(WHITE);

    prevHum = newHum;
    prevTemp = newTemp;
    prevPotent = newPotent;
    display.clearDisplay();
    display.setCursor(0, 0);
    generateLineEntry(0, newHum);
    generateLineEntry(useCelsius ? 1 : 2, newTemp);
    generateLineEntry(3, computeSamplingRate(newPotent));
    display.display();
  }   
  delay(computeSamplingRate(prevPotent)); 
}
