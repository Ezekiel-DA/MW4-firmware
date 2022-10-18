#pragma once

#define FASTLED_RMT_MAX_CHANNELS 1 // required to work around a bug in FastLED's ESP32 SMT driver?
#include <FastLED.h>

class LEDChannel {
public:
  LEDChannel(size_t iNumLEDs, bool iBackScreenHack = false);

  void off();
  void fillSolid(const CHSV& iColor);

  void fillRainbow();

private:
  CRGB* leds;
  size_t numLEDs;

  bool _backScreenHack = false;

  template<uint8_t DATA_PIN> friend void addLEDsToChannel(LEDChannel* iChannel);
};

template<uint8_t DATA_PIN> void addLEDsToChannel(LEDChannel* iChannel) {
  iChannel->leds = (CRGB*) malloc(sizeof(CRGB) * iChannel->numLEDs);
  auto controller = &FastLED.addLeds<WS2812B, DATA_PIN, GRB>(iChannel->leds, iChannel->numLEDs);
  controller->setDither(0);
};
