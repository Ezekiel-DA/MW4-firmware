# pragma once

#include "config.h"

void pulseStatusLED() {
  for (uint8_t i = 0; i < 4; ++i)
      {
        digitalWrite(STATUS_LED_PIN, LOW);
        delay(50);
        digitalWrite(STATUS_LED_PIN, HIGH);
        delay(50);
      }
      digitalWrite(STATUS_LED_PIN, LOW);
}
