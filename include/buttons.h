#pragma once

#include <AceButton.h>
using namespace ace_button;

#include "config.h"
#include "MusicService.h"

ButtonConfig mainButtonConfig;
AceButton mainButton(&mainButtonConfig);

// TODO move this sort of global costume state somewhere logical...
static bool altMode = false;

void timerCB(TimerHandle_t xTimer) {
  altMode = false;
}

TimerHandle_t altModeResetTimerHandle = xTimerCreate("altModeResetTimer", pdMS_TO_TICKS(5000), /*make it one shot*/pdFALSE, 0, timerCB);

void mainButtonEventHandler(AceButton* button, uint8_t eventType, uint8_t buttonState)
{
  switch (eventType)
  {
    case AceButton::kEventPressed:
      audioConnecttoSD("/taylor_swift_shake_it_off.mp3");
      break;
    case AceButton::kEventReleased:
      altMode = true;
      xTimerStart(altModeResetTimerHandle, 0); // this will conveniently reset the timer if it was running
      break;
  }
}

void setupButtons()
{
  pinMode(SX1509_BUTTON_PIN, INPUT_PULLUP);
  mainButton.init((uint8_t)0);
  mainButtonConfig.setEventHandler(mainButtonEventHandler);
}

void checkButtons()
{
  static uint16_t prev = millis();

  uint16_t now = millis();
  if ((uint16_t)(now - prev) >= 5)
  {
    mainButton.check();
    prev = now;
  }
}
