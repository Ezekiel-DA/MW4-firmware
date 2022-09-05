# pragma once

#include <FastLED.h>

void setAllLEDs(CRGB c, CRGB* strip, uint16_t numLeds);
void setAllLEDs(CHSV c, CRGB* strip, uint16_t numLeds);
