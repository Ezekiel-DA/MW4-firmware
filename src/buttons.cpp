#include "buttons.h"

#include <AceButton.h>
using namespace ace_button;

#include "config.h"
#include "MusicService.h"

ButtonConfig mainButtonConfig;
AceButton mainButton(&mainButtonConfig);

static MusicService** musicService = nullptr;

bool altMode = false;

void timerCB(TimerHandle_t xTimer) {
  altMode = false;
  Serial.println("alt mode off");
}

TimerHandle_t altModeResetTimerHandle = xTimerCreate("altModeResetTimer", pdMS_TO_TICKS(ALT_MODE_MS), /*make it one shot*/pdFALSE, 0, timerCB);

void mainButtonEventHandler(AceButton* button, uint8_t eventType, uint8_t buttonState)
{
  switch (eventType)
  {
    case AceButton::kEventPressed:
      Serial.println("pressed");
      (*musicService)->play();
      break;
    case AceButton::kEventReleased:
      altMode = true;
      Serial.println("alt mode on");
      xTimerStart(altModeResetTimerHandle, 0); // this will conveniently reset the timer if it was running
      break;
  }
}

void setupButtons(MusicService** iMusicService)
{
  musicService = iMusicService;
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
