#include "LEDChannel.h"

#include "config.h"

LEDChannel::LEDChannel(size_t iNumLEDs, bool iBackScreenHack): numLEDs(iNumLEDs), _backScreenHack(iBackScreenHack) {

};

void LEDChannel::off() {
  fill_solid(this->leds, this->numLEDs, CRGB::Black);
}

void LEDChannel::fillSolid(const CHSV& iColor) {
  if (this->_backScreenHack) {
    fill_solid(this->leds, BACK_SCREEN_NUM_BACKLIGHT_LEDS, CRGB::White);
    fill_solid(&(this->leds[BACK_SCREEN_NUM_BACKLIGHT_LEDS]), this->numLEDs - BACK_SCREEN_NUM_BACKLIGHT_LEDS, iColor);
  } else {
    fill_solid(this->leds, this->numLEDs, iColor);
  }
}

void LEDChannel::fillRainbow() {
  if (this->_backScreenHack) {
    fill_solid(this->leds, BACK_SCREEN_NUM_BACKLIGHT_LEDS, CRGB::White);
    fill_rainbow(&(this->leds[BACK_SCREEN_NUM_BACKLIGHT_LEDS]), this->numLEDs - BACK_SCREEN_NUM_BACKLIGHT_LEDS, beat8(5), 5);
  } else {
    fill_rainbow(this->leds, this->numLEDs, beat8(5), 5);
  }
}
