/*
    ICE CAKE
    Version: 1.0 2018-05-05
    Author: Jake Scaltreto

    Description: This is a cake made of ice that never melts.

    Flat Earth Theatre Presents "The Nether" by Jennifer Haley
    June 8th - 23rd 2018
    Mosesian Center for the Arts, Watertown MA
    Info at https://www.flatearththeatre.com

    Copyright (C) 2018 Jake Scaltreto

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <Adafruit_NeoPixel.h>
#include <SimpleTimer.h>

#define LED_PIN       6
#define NUM_LEDS      56
#define NUM_ROWS      8
#define LEDS_PER_ROW  7
#define PULSE_NUM_STEPS 250
#define BLINK_NUM_STEPS 50

static const uint8_t pulseMinBright = 10;
static const uint8_t pulseMaxBright = 150;
static const uint8_t pulseSat = 150;
static const uint16_t pulseHue = 500;
static const uint8_t pulseSpeed = 10;
uint8_t pulseSteps[PULSE_NUM_STEPS];

static const uint8_t blinkMinBright = 50;
static const uint8_t blinkMaxBright = 250;
static const uint16_t blinkChance = 100; // Out Of 10000
static const uint16_t blinkHue = 500;
static const uint8_t blinkSpeed = 5;
static const uint16_t blinkRate = 500;
uint8_t blinkSteps[BLINK_NUM_STEPS];

static const uint8_t frameRate = 1;

typedef struct {
  uint8_t r;
  uint8_t g;
  uint8_t b;
} rgb;

uint16_t row[NUM_ROWS];
uint16_t *rowStep;
rgb rowColor[NUM_ROWS];
rgb *rowPtr;

uint8_t ledRow[NUM_LEDS];
uint8_t ledBlink[NUM_LEDS];

uint16_t blinkTrigger;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

SimpleTimer timer;

void setup() {
  // Setup our rows
  uint8_t row_cur = 0;
  for (uint8_t i = 0; i < strip.numPixels() + 1; i++) {
    ledRow[i] = row_cur;
    // Initialize rows to a random step value
    if ((i + 1) % LEDS_PER_ROW == 0) {
      row[row_cur] = random(0, PULSE_NUM_STEPS);
      row_cur++;
    }
  }
  // Precalculate our step values for the pulsing effect
  for (uint16_t i = 0; i < PULSE_NUM_STEPS; i++) {
    pulseSteps[i] = step2bright(i, PULSE_NUM_STEPS, pulseMinBright, pulseMaxBright);
  }

  // Precalculate our step values for the blinking effect
  for (uint16_t i = 0; i < BLINK_NUM_STEPS; i++) {
    // Invert this because step2bright goes BRIGHT->DIM->BRIGHT by default. 
    // Doesn't matter for pulses, but does for blinks!
    blinkSteps[i] = ~step2bright(i, BLINK_NUM_STEPS, blinkMinBright, blinkMaxBright);
  }

  strip.begin();

  timer.setInterval(frameRate, updateLEDs);
  timer.setInterval(pulseSpeed, updateRows);
  timer.setInterval(blinkRate, evalBlink);
  timer.setInterval(blinkSpeed, updateBlink);

  updateRows();
}

void loop() {
  timer.run();
}

void updateLEDs() {
  for (uint8_t i = 0; i < strip.numPixels() + 1; i++) {
    rowPtr = &rowColor[ledRow[i]];
    strip.setPixelColor(i, rowPtr->r, rowPtr->g, rowPtr->b);
    if ( ledBlink[i] > 0 ) {
      strip.setPixelColor(i, blinkSteps[ledBlink[i]], blinkSteps[ledBlink[i]], blinkSteps[ledBlink[i]]);
    }
  }
  strip.show();
}

void updateBlink() {
  for (uint8_t i = 0; i < strip.numPixels() + 1; i++) {
    if ( ledBlink[i] > 0 ) {
      ledBlink[i] = ledBlink[i] < (BLINK_NUM_STEPS - 1) ? ledBlink[i] + 1 : 0;
    }
  }
}

void updateRows() {
  for (uint8_t i = 0; i < NUM_ROWS; i++) {
    row[i] = row[i] < (PULSE_NUM_STEPS - 1) ? row[i] + 1 : 0;
    hsb2rgb(pulseHue, pulseSat, pulseSteps[row[i]], &rowColor[i]);
  }
}

void evalBlink() {
  for (uint8_t i = 0; i < strip.numPixels() + 1; i++) {
    if ( ledBlink[i] == 0 ) {
      blinkTrigger = random(1, 10000);
      if ( blinkTrigger < blinkChance) {
        ledBlink[i] = 1;
      }
    }
  }
}

uint8_t step2bright(int16_t thisStep, uint16_t numSteps, uint8_t minBright, uint8_t maxBright) {
  char buff[50];
  int16_t cycle = thisStep - (numSteps / 2);
  uint8_t brightness = map(abs(cycle), 0 , (numSteps / 2), 1, 250);
  uint8_t realBrightness = map(brightness, 1, 250, minBright, maxBright);
  return realBrightness;
}

void hsb2rgb(uint16_t index, uint8_t sat, uint8_t bright, rgb color[3]) {
  uint8_t temp[5], n = (index >> 8) % 3;
  uint8_t x = ((((index & 255) * sat) >> 8) * bright) >> 8;
  uint8_t s = ((256 - sat) * bright) >> 8;
  temp[0] = temp[3] = s;
  temp[1] = temp[4] = x + s;
  temp[2] = bright - x;
  color->r  = temp[n + 2];
  color->g = temp[n + 1];
  color->b  = temp[n];
}
