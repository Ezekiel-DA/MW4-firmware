#pragma once

#define MW_RFID_DATA_BLOCK_ADDR 4

#define STATUS_LED_PIN 5

// the advertised service, allowing clients to find us
#define MWNEXT_BLE_COSTUME_CONTROL_SERVICE_UUID           "47191881-ebb3-4a9f-9645-3a5c6dae4900"

// characteristics for the main service, giving access to Peripheral-wide functions
#define MWNEXT_BLE_TAG_PRESENT_CHARACTERISTIC_UUID        "68344255-0632-4635-93f0-178bd0d7e577"
#define MWNEXT_BLE_TAG_WRITE_REQUEST_CHARACTERISTIC_UUID  "64957168-79ba-469b-96e9-ccf53086c4c3"
#define MWNEXT_BLE_TAG_WRITE_ERROR_CHARACTERISTIC_UUID    "acd33ab1-eda9-4790-8ce2-f9fe951b3e25"

// non advertised, light controlling services
#define MWNEXT_BLE_WINDOWS_SERVICE_UUID                   "c3f48cdf-18bb-4947-bef2-f804d59d74da"
#define MWNEXT_BLE_CLOUDS_SERVICE_UUID                    "f248fd08-0326-482e-b023-db6e3f6bb250"
#define MWNEXT_BLE_WALLS_SERVICE_UUID                     "b3701981-4955-49d5-bd2d-e2ea57e8e64c"
#define MWNEXT_BLE_MOAT_SERVICE_UUID                      "5180f077-a430-4a6d-b6ea-cdf1075a0dd9"
#define MWNEXT_BLE_STARS_SERVICE_UUID                     "c634d6bf-0f7b-4580-b342-7aa2d42dedab"

// characteristics for our light controlling services 
#define MWNEXT_BLE_DEVICE_TYPE_CHARACTERISTIC_UUID        "8106f98f-fb24-4b97-a995-47a1695cea75"
#define MWNEXT_BLE_MODE_CHARACTERISTIC_UUID               "b54fc13b-4374-4a6f-861f-dd198f88f299"
#define MWNEXT_BLE_HUE_CHARACTERISTIC_UUID                "19dfe175-aa12-404b-843d-b625937cffff"
#define MWNEXT_BLE_CYCLE_COLOR_CHARACTERISTIC_UUID        "dfe34849-2d42-4222-b6b1-617a4f4d0869"
#define MWNEXT_BLE_SATURATION_CHARACTERISTIC_UUID         "946d22e6-2b2f-49e7-b941-150b023f2261"

// characteristics for our text display matrix service
#define MWNEXT_BLE_TEXT_DISPLAY_TEXT_CHARACTERISTIC_UUID  "c5b56d2e-b6e9-49c7-b098-5af9a75f46cd"
#define MWNEXT_BLE_TEXT_DISPLAY_OFFSET_UUID               "a0f348d3-5c80-420f-bb5f-1732824ac215"
#define MWNEXT_BLE_TEXT_DISPLAY_CUSTOMIZE_OFFSET_UUID     "966c3d0d-493c-4b59-ae1d-d3768ff9ada8"

// TODO: rethink this whole thing. This probably needs to be some sort of bitfield of capabilities?
// Right now, though:
// 1: RGB LED; can do patterns and colors
// 2: MONO LED: can do patterns, cannot do colors
// 3: ONOFF_PORT: cannot do patterns, cannot do colors; style uses the MODE characteristic to indicate a "Pattern ID", but the only two legal values are 0: off and 1: on
enum struct MWNEXT_DEVICE_TYPE {
  RGB_LED = 1,
  MONO_LED,
  ONOFF_PORT
};

enum struct MWNEXT_WRITE_REQUEST_ERROR {
  MFRC522_ERROR = 1,
  NO_TAG,
};