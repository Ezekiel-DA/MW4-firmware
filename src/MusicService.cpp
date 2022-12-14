#include "MusicService.h"

#include <Arduino.h>
#include <Audio.h>
#include <NimBLEDevice.h>

#include <string>

#include "BLE.h"
#include "config.h"

char* trackMapping[] = {
  "/billy_joel_piano_man.mp3",  // 0
  "/blues_traveler_hook.mp3",
  "/blues_traveler_run_around.mp3",
  "/bobby_boris_picket_monster_mash.mp3",
  "/encanto_surface_pressure.mp3",
  "/israel_kamakawiwo_ole_somewhere_over_the_rainbow.mp3",
  "/journey_dont_stop_believing.mp3",
  "/raffi_lets_play.mp3",
  "/sia_riding_on_my_bike.mp3",
  "/taylor_swift_shake_it_off.mp3",
  "/taylor_swift_the_best_day.mp3",
  "/the_impressions_its_all_right.mp3",  // 11
};

struct audioMessage {
  uint8_t cmd;
  const char* txt;
  uint32_t value;
  uint32_t ret;
} audioTxMessage, audioRxMessage;

enum : uint8_t { SET_VOLUME,
                 GET_VOLUME,
                 CONNECTTOSD };

QueueHandle_t audioSetQueue = NULL;
QueueHandle_t audioGetQueue = NULL;

Audio audio;

void CreateQueues() {
  audioSetQueue = xQueueCreate(10, sizeof(struct audioMessage));
  audioGetQueue = xQueueCreate(10, sizeof(struct audioMessage));
}

void audioTask(void* parameter) {
  CreateQueues();
  if (!audioSetQueue || !audioGetQueue) {
    log_e("queues are not initialized");
    while (true) {
      ;
    }  // endless loop
  }

  struct audioMessage audioRxTaskMessage;
  struct audioMessage audioTxTaskMessage;

  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);

  while (true) {
    if (xQueueReceive(audioSetQueue, &audioRxTaskMessage, 1) == pdPASS) {
      if (audioRxTaskMessage.cmd == SET_VOLUME) {
        audioTxTaskMessage.cmd = SET_VOLUME;
        audio.setVolume(audioRxTaskMessage.value);
        audioTxTaskMessage.ret = 1;
        xQueueSend(audioGetQueue, &audioTxTaskMessage, portMAX_DELAY);
      } else if (audioRxTaskMessage.cmd == CONNECTTOSD) {
        audioTxTaskMessage.cmd = CONNECTTOSD;
        audioTxTaskMessage.ret = audio.connecttoSD(audioRxTaskMessage.txt);
        xQueueSend(audioGetQueue, &audioTxTaskMessage, portMAX_DELAY);
      } else if (audioRxTaskMessage.cmd == GET_VOLUME) {
        audioTxTaskMessage.cmd = GET_VOLUME;
        audioTxTaskMessage.ret = audio.getVolume();
        xQueueSend(audioGetQueue, &audioTxTaskMessage, portMAX_DELAY);
      } else {
        log_i("error");
      }
    }
    audio.loop();
  }
}

void audioInit() {
  xTaskCreatePinnedToCore(
    audioTask,   /* Function to implement the task */
    "audioplay", /* Name of the task */
    5000,        /* Stack size in words */
    NULL,        /* Task input parameter */
    5,           /* Priority of the task */
    NULL,        /* Task handle. */
    1            /* Core where the task should run */
  );
}

audioMessage transmitReceive(audioMessage msg) {
  xQueueSend(audioSetQueue, &msg, portMAX_DELAY);
  if (xQueueReceive(audioGetQueue, &audioRxMessage, portMAX_DELAY) == pdPASS) {
    if (msg.cmd != audioRxMessage.cmd) {
      log_e("wrong reply from message queue");
    }
  }
  return audioRxMessage;
}

void audioSetVolume(uint8_t vol) {
  audioTxMessage.cmd = SET_VOLUME;
  audioTxMessage.value = vol;
  Serial.print("Setting volume to: "); Serial.println(vol);
  audioMessage RX = transmitReceive(audioTxMessage);
}

uint8_t audioGetVolume() {
  audioTxMessage.cmd = GET_VOLUME;
  audioMessage RX = transmitReceive(audioTxMessage);
  return RX.ret;
}

bool audioConnecttoSD(const char* filename) {
  audioTxMessage.cmd = CONNECTTOSD;
  audioTxMessage.txt = filename;
  audioMessage RX = transmitReceive(audioTxMessage);
  return RX.ret;
}

MusicService::MusicService(BLEServer* iServer, MusicSettings* iSettings) : settings(iSettings) {
  audioInit();

  audioSetVolume(this->settings->volume);

  this->service = iServer->createService(MW4_BLE_MUSIC_CONTROL_SERVICE_UUID);

  auto stateCharacteristic = service->createCharacteristic(MW4_BLE_MUSIC_CONTROL_STATE_CHARACTERISTIC, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::INDICATE);
  stateCharacteristic->setCallbacks(this);
  stateCharacteristic->setValue((uint8_t*)&(this->settings->state), 1);
  attachUserDescriptionToCharacteristic(stateCharacteristic, "State");

  auto volumeCharacteristic = service->createCharacteristic(MW4_BLE_MUSIC_CONTROL_VOLUME_CHARACTERISTIC, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::INDICATE);
  volumeCharacteristic->setCallbacks(this);
  volumeCharacteristic->setValue(&(this->settings->volume), 1);
  attachUserDescriptionToCharacteristic(volumeCharacteristic, "Volume");

  auto trackCharacteristic = service->createCharacteristic(MW4_BLE_MUSIC_CONTROL_TRACK_CHARACTERISTIC, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::INDICATE);
  trackCharacteristic->setCallbacks(this);
  trackCharacteristic->setValue(&(this->settings->track), 1);
  attachUserDescriptionToCharacteristic(trackCharacteristic, "Track");
}

void MusicService::onWrite(BLECharacteristic* characteristic) {
  BLEUUID id = characteristic->getUUID();

  std::string safeData = characteristic->getValue();
  uint8_t* data = (uint8_t*)safeData.data();

  portENTER_CRITICAL(&settingsMutex);

  if (id.equals(std::string(MW4_BLE_MUSIC_CONTROL_TRACK_CHARACTERISTIC))) {
    this->settings->track = *data;
  } else if (id.equals(std::string(MW4_BLE_MUSIC_CONTROL_VOLUME_CHARACTERISTIC))) {
    this->settings->volume = *data;
  } else if (id.equals(std::string(MW4_BLE_STATE_CHARACTERISTIC_UUID))) {
    this->settings->state = (*data) != 0;
  }

  portEXIT_CRITICAL(&settingsMutex);

  audioSetVolume(this->settings->volume);

  markSettingsModified();
}

void MusicService::play() {
  srand(millis());
  audioConnecttoSD(trackMapping[rand() % 12]);
}
