#pragma once

#define FW_VERSION 2

#define DEFAULTS_SETTINGS_FILE  "/defaults.json"
#define USER_SETTINGS_FILE      "/settings.json"

// TODO: CHANGE ME! Defaults for dev device vs costume
//#define BUTTON_PIN        21
#define BUTTON_PIN          0 // for testing; use 21 above for the costume

#define PULSE_BPM 20

#define ALT_MODE_MS 60000

#define BRIGHTNESS_LIMIT_AT_WHITE 200 // 200

// actual costume values in comment; use those, or lower for testing
#define RACETRACK_NUM_LEDS              300      // 300

#define BOTTOM_V_NUM_LEDS               70      // 70

#define FRONT_U_NUM_LEDS                155     // 155

#define BACK_U_NUM_LEDS                 120     // 120

#define BACK_SCREEN_NUM_LEDS            155      // 155
#define BACK_SCREEN_NUM_BACKLIGHT_LEDS  73      // 73 // just the backlight bit, which precedes the rest (0-82: backlight; 83-154: perimeter)

#define PEDESTAL_NUM_LEDS               300       // 300

#define LED_D1          13
#define LED_D2          12
#define LED_D3          4
#define LED_D4          15
#define LED_D5          16
#define LED_D6          17
#define LED_D7          2

#define I2S_DOUT        26
#define I2S_BCLK        27
#define I2S_LRC         14

// These are organized to try to share the load on the V2 PCB's separate rails:
// - rail 1: LED1, LED4, LED7 (this rail also powers the ESP32)
#define BOTTOM_V_PIN        LED_D1
#define FRONT_U_PIN         LED_D4
#define BACK_U_PIN          LED_D7
// - rail 2: LED2, LED5
#define RACETRACK_STRIP_PIN LED_D2
#define BACK_SCREEN_PIN     LED_D5
// - rail 3: LED3, LED6 (this rail also powers the Amp, mic, SPI Flash and GPIO expander)
#define FRONT_TEXT_PIN      LED_D3
#define PEDESTAL_PIN        LED_D6

// =================================================================
// Bluetooth Low Energy settings
// =================================================================
// 
// BLE UUIDs. For characteristics, since we need two (main and alt. mode for when the button is pressed),
// those are stored as arrays where 0: main and 1: alt.

// Main Costume Control service; this is the advertised service, allowing clients to find us
#define MW4_BLE_COSTUME_CONTROL_SERVICE_UUID                        "47191881-ebb3-4a9f-9645-3a5c6dae4900"
#define MW4_BLE_COSTUME_CONTROL_FW_VERSION_CHARACTERISTIC_UUID                  "55cf24c7-7a28-4df4-9b53-356b336bab71"
#define MW4_BLE_COSTUME_CONTROL_OTA_DATA_CHARACTERISTIC_UUID                    "1083b9a4-fdc0-4aa6-b027-a2600c8837c4"
#define MW4_BLE_COSTUME_CONTROL_OTA_CONTROL_CHARACTERISTIC_UUID                 "d1627dbe-b6ae-421f-b2eb-5878576410c0"
#define MW4_BLE_COSTUME_CONTROL_DANGER_ZONE_CHARACTERISTIC_UUID                 "c193c0df-7eff-4261-943d-0672b85871b5"

// OTA update control messages
#define OTA_CONTROL_NOP   0x00
#define OTA_CONTROL_ACK   0x01
#define OTA_CONTROL_NACK  0x02
#define OTA_CONTROL_START 0x04
#define OTA_CONTROL_END   0x08
#define OTA_CONTROL_ERR   0xFF


// Light controlling services; multiple instances allowed
#define MW4_BLE_LIGHT_DEVICE_SERVICE_UUID                           "0ba35e90-f55f-4f15-9347-3dc4a0287881"
#define MW4_BLE_ID_CHARACTERISTIC_UUID                                          "63c62656-807f-4db4-97d3-94095962acf8"
#define MW4_BLE_STATE_CHARACTERISTIC_UUID                                       "c2af353b-e5fc-4bdf-b743-5d226f1198a2"
#define MW4_BLE_MODE_CHARACTERISTIC_UUID                                        "b54fc13b-4374-4a6f-861f-dd198f88f299"
#define MW4_BLE_HUE_CHARACTERISTIC_UUID                                         "19dfe175-aa12-404b-843d-b625937cffff" 
#define MW4_BLE_SATURATION_CHARACTERISTIC_UUID                                  "946d22e6-2b2f-49e7-b941-150b023f2261"
#define MW4_BLE_VALUE_CHARACTERISTIC_UUID                                       "6c5df188-2e69-4f2f-b4ab-9d2b76ef7aa9"


// Text display matrix service
#define MW4_BLE_TEXT_DISPLAY_SERVICE_UUID                             "aafca82b-95ae-4f33-9cf3-7ee0ef15ddf4"

#define MW4_BLE_TEXT_DISPLAY_TEXT_CHARACTERISTIC_UUID                           "c5b56d2e-b6e9-49c7-b098-5af9a75f46cd"
#define MW4_BLE_TEXT_DISPLAY_TEXT_ALT_CHARACTERISTIC_UUID                       "5830efb7-819e-42db-8d4e-e9e19f096a20"
static const char* MW4_BLE_TEXT_DISPLAY_TEXT_CHARACTERISTIC_UUIDS[2] =          {MW4_BLE_TEXT_DISPLAY_TEXT_CHARACTERISTIC_UUID, MW4_BLE_TEXT_DISPLAY_TEXT_ALT_CHARACTERISTIC_UUID};

#define MW4_BLE_TEXT_DISPLAY_OFFSET_CHARACTERISTIC_UUID                         "a0f348d3-5c80-420f-bb5f-1732824ac215"
#define MW4_BLE_TEXT_DISPLAY_OFFSET_ALT_CHARACTERISTIC_UUID                     "0ef6a5e1-9f5c-41ba-b1d6-39139cc8e721"
static const char* MW4_BLE_TEXT_DISPLAY_OFFSET_CHARACTERISTIC_UUIDS[2] =        {MW4_BLE_TEXT_DISPLAY_OFFSET_CHARACTERISTIC_UUID, MW4_BLE_TEXT_DISPLAY_OFFSET_ALT_CHARACTERISTIC_UUID};

#define MW4_BLE_TEXT_DISPLAY_SCROLLING_CHARACTERISTIC_UUID                      "966c3d0d-493c-4b59-ae1d-d3768ff9ada8"
#define MW4_BLE_TEXT_DISPLAY_SCROLLING_ALT_CHARACTERISTIC_UUID                  "15b01f47-fe52-4851-8835-e97b09906443"
static const char* MW4_BLE_TEXT_DISPLAY_SCROLLING_CHARACTERISTIC_UUIDS[2] =     {MW4_BLE_TEXT_DISPLAY_SCROLLING_CHARACTERISTIC_UUID, MW4_BLE_TEXT_DISPLAY_SCROLLING_ALT_CHARACTERISTIC_UUID};

#define MW4_BLE_TEXT_DISPLAY_SCROLL_SPEED_CHARACTERISTIC_UUID                   "1fc1cf59-0e88-4dda-81fc-818a14da216b"
#define MW4_BLE_TEXT_DISPLAY_SCROLL_SPEED_ALT_CHARACTERISTIC_UUID               "ee361bc6-09cc-47af-b5b8-7e035b92e339"
static const char* MW4_BLE_TEXT_DISPLAY_SCROLL_SPEED_CHARACTERISTIC_UUIDS[2] =  {MW4_BLE_TEXT_DISPLAY_SCROLL_SPEED_CHARACTERISTIC_UUID, MW4_BLE_TEXT_DISPLAY_SCROLL_SPEED_ALT_CHARACTERISTIC_UUID};

#define MW4_BLE_TEXT_DISPLAY_PAUSE_TIME_CHARACTERISTIC_UUID                     "7a991117-a277-46ae-8d73-0a7e7e6ae7e8"
#define MW4_BLE_TEXT_DISPLAY_PAUSE_TIME_ALT_CHARACTERISTIC_UUID                 "920879e1-b132-4326-ad22-7daf4bd4f037"
static const char* MW4_BLE_TEXT_DISPLAY_PAUSE_TIME_CHARACTERISTIC_UUIDS[2] =    {MW4_BLE_TEXT_DISPLAY_PAUSE_TIME_CHARACTERISTIC_UUID, MW4_BLE_TEXT_DISPLAY_PAUSE_TIME_ALT_CHARACTERISTIC_UUID};

#define MW4_BLE_TEXT_DISPLAY_BRIGHTNESS_CHARACTERISTIC_UUID                     "48387eca-eedf-40ee-ab37-b4fb3a18cdf1"
#define MW4_BLE_TEXT_DISPLAY_BRIGHTNESS_ALT_CHARACTERISTIC_UUID                 "a682dc43-8039-402e-a5cb-4c1b41075bf4"
static const char* MW4_BLE_TEXT_DISPLAY_BRIGHTNESS_CHARACTERISTIC_UUIDS[2] =    {MW4_BLE_TEXT_DISPLAY_BRIGHTNESS_CHARACTERISTIC_UUID, MW4_BLE_TEXT_DISPLAY_BRIGHTNESS_ALT_CHARACTERISTIC_UUID};

#define MW4_BLE_TEXT_DISPLAY_FG_COLOR_CHARACTERISTIC_UUID                       "4119bf67-6295-4ec9-b596-5b32a3f2fda5"
#define MW4_BLE_TEXT_DISPLAY_FG_COLOR_ALT_CHARACTERISTIC_UUID                   "de7cc85a-7df9-416b-be56-76c5ac8faa89"
static const char* MW4_BLE_TEXT_DISPLAY_FG_COLOR_CHARACTERISTIC_UUIDS[2] =      {MW4_BLE_TEXT_DISPLAY_FG_COLOR_CHARACTERISTIC_UUID, MW4_BLE_TEXT_DISPLAY_FG_COLOR_ALT_CHARACTERISTIC_UUID};

#define MW4_BLE_TEXT_DISPLAY_BG_COLOR_CHARACTERISTIC_UUID                       "1b56faa0-376a-432a-a79a-e1a4f12dd493"
#define MW4_BLE_TEXT_DISPLAY_BG_COLOR_ALT_CHARACTERISTIC_UUID                   "fe344d11-3bc2-4511-841b-05f69f80b17f"
static const char* MW4_BLE_TEXT_DISPLAY_BG_COLOR_CHARACTERISTIC_UUIDS[2] =      {MW4_BLE_TEXT_DISPLAY_BG_COLOR_CHARACTERISTIC_UUID, MW4_BLE_TEXT_DISPLAY_BG_COLOR_ALT_CHARACTERISTIC_UUID};


// Music control service
#define MW4_BLE_MUSIC_CONTROL_SERVICE_UUID                              "884411c5-0445-4a87-bd8e-e08311026227"

#define MW4_BLE_MUSIC_CONTROL_VOLUME_CHARACTERISTIC                             "2a561d00-1072-42a4-acac-e06a7509b4ca"
#define MW4_BLE_MUSIC_CONTROL_TRACK_CHARACTERISTIC                              "9e085f00-02ef-4869-b703-e31f015485d2"
#define MW4_BLE_MUSIC_CONTROL_STATE_CHARACTERISTIC                              "ef1df999-7b78-40ea-b5dc-205f700dff75"


// "Little Snack" audio recording / playback service
#define MW4_BLE_LITTLE_SNACK_SERVICE_UUID                                   "d043678c-e20f-4bad-9b3d-889568b6c383"

#define MW4_BLE_LITTLE_SNACK_STATE_CHARACTERISTIC                                   "6f56efd2-667c-41b2-8a8a-a2d228f80ae7"
