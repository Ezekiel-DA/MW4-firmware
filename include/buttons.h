#pragma once

#include <AceButton.h>
using namespace ace_button;

ButtonConfig rfidButtonConfig;
AceButton rfidButton(&rfidButtonConfig);

bool rfidWrite = false;

void rfidButtonEventHandler(AceButton* button, uint8_t eventType, uint8_t buttonState)
{
  switch (eventType)
  {
    case AceButton::kEventPressed:
      rfidWrite = true;
      digitalWrite(STATUS_LED_PIN, HIGH);
      break;
    case AceButton::kEventReleased:
      rfidWrite = false;
      digitalWrite(STATUS_LED_PIN, LOW);
      break;
  }
}

void setupButtons()
{
  pinMode(0, INPUT_PULLUP);
  rfidButton.init((uint8_t)0);
  rfidButtonConfig.setEventHandler(rfidButtonEventHandler);
}

void checkButtons()
{
  static uint16_t prev = millis();

  uint16_t now = millis();
  if ((uint16_t)(now - prev) >= 5)
  {
    rfidButton.check();
    prev = now;
  }
}
