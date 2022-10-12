#include "settingsManager.h"

#include <Arduino.h>
#include <FS.h>
#include <SPIFFS.h>

#include <ArduinoJson.h>

#include "config.h"

portMUX_TYPE settingsMutex = portMUX_INITIALIZER_UNLOCKED;

LightDeviceSettings chairLightSettings;
LightDeviceSettings chairLightSettingsAlt;

LightDeviceSettings pedestalLightSettings;
LightDeviceSettings pedestalLightSettingsAlt;

TextDisplaySettings textSettings;
TextDisplaySettings textSettingsAlt;

MusicSettings musicSettings;

TaskHandle_t settingsSaverTask = nullptr;

std::string userSettingsJSON;
bool settingsUpdated = false;

void RGBToHexString(const uint8_t rgb[3], char* ioBuffer) {
  snprintf(ioBuffer, 9, "0x%02x%02x%02x", rgb[0], rgb[1], rgb[2]);
};

void serializeSettings(std::string& ioString, const CostumeSettings& settings) {
  StaticJsonDocument<1024> doc;

  JsonObject musicService = doc.createNestedObject("musicService");
  musicService["state"] = settings.musicSettings.state;
  musicService["volume"] = settings.musicSettings.volume;
  musicService["track"] = settings.musicSettings.track;

  JsonObject frontText = doc.createNestedObject("frontText");

  JsonObject frontText_default = frontText.createNestedObject("default");
  frontText_default["state"] = settings.textSettings.state;
  frontText_default["text"] = settings.textSettings.text;
  frontText_default["scrolling"] = settings.textSettings.scrolling;
  frontText_default["scrollSpeed"] = settings.textSettings.scrollSpeed;
  frontText_default["pauseTime"] = settings.textSettings.pauseTime;

  char hexcol[9];
  RGBToHexString(settings.textSettings.fgColor, hexcol);
  frontText_default["fgColor"] = hexcol;
  RGBToHexString(settings.textSettings.bgColor, hexcol);
  frontText_default["bgColor"] = hexcol;

  JsonObject frontText_alt = frontText.createNestedObject("alt");
  frontText_alt["state"] = settings.textSettingsAlt.state;
  frontText_alt["text"] = settings.textSettingsAlt.text;
  frontText_alt["scrolling"] = settings.textSettingsAlt.scrolling;
  frontText_alt["scrollSpeed"] = settings.textSettingsAlt.scrollSpeed;
  frontText_alt["pauseTime"] = settings.textSettingsAlt.pauseTime;

  RGBToHexString(settings.textSettingsAlt.fgColor, hexcol);
  frontText_alt["fgColor"] = hexcol;
  RGBToHexString(settings.textSettingsAlt.bgColor, hexcol);
  frontText_alt["bgColor"] = hexcol;

  JsonObject chairLights = doc.createNestedObject("chairLights");

  JsonObject chairLights_default = chairLights.createNestedObject("default");
  chairLights_default["state"] = settings.chairLightSettings.state;
  chairLights_default["mode"] = settings.chairLightSettings.mode;
  chairLights_default["hue"] = settings.chairLightSettings.hue;
  chairLights_default["saturation"] = settings.chairLightSettings.saturation;
  chairLights_default["value"] = settings.chairLightSettings.value;

  JsonObject chairLights_alt = chairLights.createNestedObject("alt");
  chairLights_alt["state"] = settings.chairLightSettingsAlt.state;
  chairLights_alt["mode"] = settings.chairLightSettingsAlt.mode;
  chairLights_alt["hue"] = settings.chairLightSettingsAlt.hue;
  chairLights_alt["saturation"] = settings.chairLightSettingsAlt.saturation;
  chairLights_alt["value"] = settings.chairLightSettingsAlt.value;

  JsonObject pedestalLights = doc.createNestedObject("pedestalLights");

  JsonObject pedestalLights_default = pedestalLights.createNestedObject("default");
  pedestalLights_default["state"] = settings.pedestalLightSettings.state;
  pedestalLights_default["mode"] = settings.pedestalLightSettings.mode;
  pedestalLights_default["hue"] = settings.pedestalLightSettings.hue;
  pedestalLights_default["saturation"] = settings.pedestalLightSettings.saturation;
  pedestalLights_default["value"] = settings.pedestalLightSettings.value;

  JsonObject pedestalLights_alt = pedestalLights.createNestedObject("alt");
  pedestalLights_alt["state"] = settings.pedestalLightSettingsAlt.state;
  pedestalLights_alt["mode"] = settings.pedestalLightSettingsAlt.mode;
  pedestalLights_alt["hue"] = settings.pedestalLightSettingsAlt.hue;
  pedestalLights_alt["saturation"] = settings.pedestalLightSettingsAlt.saturation;
  pedestalLights_alt["value"] = settings.pedestalLightSettingsAlt.value;

  ioString.clear();
  Serial.println("Serializing settings to JSON in memory");
  serializeJson(doc, ioString);
  settingsUpdated = true;
}

void writeJSONFile(const char* iFilename, const std::string& iJSONString) {
  auto settingsFile = SPIFFS.open(iFilename, FILE_WRITE);
  assert(settingsFile);

  Serial.println("Writing JSON file");

  settingsFile.write((uint8_t*)iJSONString.c_str(), iJSONString.size());
  settingsFile.close();
}

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
  ret.scrollSpeed = (*doc)[device][alt ? "alt" : "default"]["scrollSpeed"];
  ret.pauseTime = (*doc)[device][alt ? "alt" : "default"]["pauseTime"];
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

    auto notification = ulTaskNotifyTake(pdTRUE, 0);
    if (notification) {
      Serial.println("Serializing settings...");

      portENTER_CRITICAL(&settingsMutex); // ensure no one is writing out settings while we're reading them and serializing them

      CostumeSettings costumeSettingsCopy;
      costumeSettingsCopy.chairLightSettings = chairLightSettings;
      costumeSettingsCopy.chairLightSettingsAlt = chairLightSettingsAlt;
      costumeSettingsCopy.pedestalLightSettings = pedestalLightSettings;
      costumeSettingsCopy.pedestalLightSettingsAlt = pedestalLightSettingsAlt;
      costumeSettingsCopy.textSettings = textSettings;
      costumeSettingsCopy.textSettingsAlt = textSettingsAlt;
      costumeSettingsCopy.musicSettings = musicSettings;

      portEXIT_CRITICAL(&settingsMutex);

      serializeSettings(userSettingsJSON, costumeSettingsCopy);
    }
  }
}

void markSettingsModified() {
  xTaskNotifyGive(settingsSaverTask);
}

void saveSettings() {
  if (settingsUpdated) {
    Serial.println("Settings updated in memory; will write out");
    writeJSONFile(USER_SETTINGS_FILE, userSettingsJSON);
    settingsUpdated = false;
  }
}

void initSettings() {
  SPIFFS.begin();

  StaticJsonDocument<1024> defaultSettingsDoc;
  StaticJsonDocument<1024> userSettingsDoc;

  bool userSettings = SPIFFS.exists(USER_SETTINGS_FILE);

  if (userSettings) {
    Serial.println("user settings exist; loading...");
  } else {
    Serial.println("No user settings found; using defaults...");
  }
  
  auto settingsFile = userSettings ? SPIFFS.open(USER_SETTINGS_FILE) : SPIFFS.open(DEFAULTS_SETTINGS_FILE);
  assert(settingsFile);

  auto err = deserializeJson(defaultSettingsDoc, settingsFile);
  if (userSettings && err.code() != DeserializationError::Ok) {
    Serial.println("User settings did not deserialize correctly; restoring defaults...");
    resetSettings();
  }
  assert(err.code() == DeserializationError::Ok);
  settingsFile.close();

  chairLightSettings = getLightSettingsForDevice(&defaultSettingsDoc, "chairLights", false);
  chairLightSettingsAlt = getLightSettingsForDevice(&defaultSettingsDoc, "chairLights", true);

  pedestalLightSettings = getLightSettingsForDevice(&defaultSettingsDoc, "pedestalLights", false);
  pedestalLightSettingsAlt = getLightSettingsForDevice(&defaultSettingsDoc, "pedestalLights", true);

  textSettings = getTextSettings(&defaultSettingsDoc, false);
  textSettingsAlt = getTextSettings(&defaultSettingsDoc, true);

  musicSettings.state = defaultSettingsDoc["musicService"]["state"];
  musicSettings.volume = defaultSettingsDoc["musicService"]["volume"];
  musicSettings.track = defaultSettingsDoc["musicService"]["track"];

  xTaskCreate(autoSaveTask, "autosave settings", 8192, nullptr, 1, &settingsSaverTask);
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