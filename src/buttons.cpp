#include "buttons.h"

#include <AceButton.h>
using namespace ace_button;

#include "config.h"

ButtonConfig mainButtonConfig;
AceButton mainButton(&mainButtonConfig);

bool altMode = false;
bool pressed = false;
bool resetButton = false;

void timerCB(TimerHandle_t xTimer) {
  altMode = false;
  Serial.println("alt mode off");
}

TimerHandle_t altModeResetTimerHandle = xTimerCreate("altModeResetTimer", pdMS_TO_TICKS(ALT_MODE_MS), /*make it one shot*/pdFALSE, 0, timerCB);

void mainButtonEventHandler(AceButton* button, uint8_t eventType, uint8_t buttonState)
{
  static long pressedAt = 0;
  switch (eventType)
  {
    case AceButton::kEventPressed:
      pressedAt = millis();
      pressed = true;
      break;
    case AceButton::kEventReleased:
      auto elapsed = millis() - pressedAt;
      altMode = true;
      xTimerStart(altModeResetTimerHandle, 0); // this will conveniently reset the timer if it was running

      if (elapsed > 10 * 1000) {
        resetButton = true;
      }
      break;
  }
}

void setupButtons()
{
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  mainButton.init((uint8_t)BUTTON_PIN);
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

bool getAltMode() {
    return altMode;
}

bool getAndResetPressed() {
  auto ret = pressed;
  pressed = false;
  return ret;
}

bool getReset() {
  return resetButton;
}