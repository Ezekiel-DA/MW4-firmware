#include "settingsManager.h"

#include <Arduino.h>
#include <FS.h>
#include <SPIFFS.h>

#include <ArduinoJson.h>

#include "config.h"

LightDeviceSettings chairLightSettings;
LightDeviceSettings chairLightSettingsAlt;

LightDeviceSettings pedestalLightSettings;
LightDeviceSettings pedestalLightSettingsAlt;

TextDisplaySettings textSettings;
TextDisplaySettings textSettingsAlt;

MusicSettings musicSettings;

LightDeviceSettings getLightSettingsForDevice(JsonDocument* doc, const char* device, bool alt) {
  LightDeviceSettings ret;

  ret.state = (*doc)[device][alt ? "alt" : "default"]["state"];
  ret.mode = (*doc)[device][alt ? "alt" : "default"]["mode"];
  ret.hue = (*doc)[device][alt ? "alt" : "default"]["hue"];
  ret.saturation = (*doc)[device][alt ? "alt" : "default"]["saturation"];
  ret.value = (*doc)[device][alt ? "alt" : "default"]["value"];
  
  return ret;
}

TextDisplaySettings getTextSettings(JsonDocument* doc, bool alt) {
  TextDisplaySettings ret;

  const char* device = "frontText";

  ret.state = (*doc)[device][alt ? "alt" : "default"]["state"];
  ret.text = (*doc)[device][alt ? "alt" : "default"]["text"].as<std::string>();
  ret.scrolling = (*doc)[device][alt ? "alt" : "default"]["scrolling"];
  long fg = strtol((*doc)[device][alt ? "alt" : "default"]["fgColor"], NULL, 16);
  long bg = strtol((*doc)[device][alt ? "alt" : "default"]["bgColor"], NULL, 16);
  
  ret.fgColor[0] = ((unsigned char*) &fg)[2];
  ret.fgColor[1] = ((unsigned char*) &fg)[1];
  ret.fgColor[2] = ((unsigned char*) &fg)[0];

  ret.bgColor[0] = ((unsigned char*) &bg)[2];
  ret.bgColor[1] = ((unsigned char*) &bg)[1];
  ret.bgColor[2] = ((unsigned char*) &bg)[0];

  return ret;
}

void autoSaveTask(void* params) {
  while(1) {
    vTaskDelay(5000 / portTICK_PERIOD_MS);

    // if (settingsModified) {

    //   settingsModified = false;
    // }
  }
}

void initSettings() {
  SPIFFS.begin();

  StaticJsonDocument<1024> defaultSettingsDoc;
  StaticJsonDocument<1024> userSettingsDoc;
  
  auto settingsFile = SPIFFS.exists(USER_SETTINGS_FILE) ? SPIFFS.open(USER_SETTINGS_FILE) : SPIFFS.open(DEFAULTS_SETTINGS_FILE);
  assert(settingsFile);

  auto err = deserializeJson(defaultSettingsDoc, settingsFile);
  assert(err.code() == DeserializationError::Ok);

  chairLightSettings = getLightSettingsForDevice(&defaultSettingsDoc, "chairLights", false);
  chairLightSettingsAlt = getLightSettingsForDevice(&defaultSettingsDoc, "chairLights", true);

  pedestalLightSettings = getLightSettingsForDevice(&defaultSettingsDoc, "pedestalLights", false);
  pedestalLightSettingsAlt = getLightSettingsForDevice(&defaultSettingsDoc, "pedestalLights", true);

  textSettings = getTextSettings(&defaultSettingsDoc, false);
  textSettingsAlt = getTextSettings(&defaultSettingsDoc, true);

  musicSettings.state = defaultSettingsDoc["musicService"]["state"];
  musicSettings.volume = defaultSettingsDoc["musicService"]["volume"];
  musicSettings.track = defaultSettingsDoc["musicService"]["track"];

  //xTaskCreate(autoSaveTask, "autosave settings", 8192, nullptr, 1, nullptr);
};

void resetSettings() {
  SPIFFS.remove(USER_SETTINGS_FILE);
  delay(100);
  esp_restart();
}

LightDeviceSettings* getChairLightSettings(bool iAlt) {
  return iAlt ? &chairLightSettingsAlt : &chairLightSettings;
}

LightDeviceSettings* getPedestalLightSettings(bool iAlt) {
  return iAlt ? &pedestalLightSettingsAlt : &pedestalLightSettings;
}

TextDisplaySettings* getTextDisplaySettings(bool iAlt) {
  return iAlt ? &textSettingsAlt : &textSettings;
}

MusicSettings* getMusicSettings() {
  return &musicSettings;
}