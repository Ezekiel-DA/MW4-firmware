#pragma once

#define STATUS_LED_PIN 5

// the advertised service, allowing clients to find us
#define MW4_BLE_COSTUME_CONTROL_SERVICE_UUID                      "47191881-ebb3-4a9f-9645-3a5c6dae4900"
#define MW4_BLE_COSTUME_CONTROL_FW_VERSION_CHARACTERISTIC_UUID    "55cf24c7-7a28-4df4-9b53-356b336bab71"
#define MW4_BLE_COSTUME_CONTROL_OTA_CHARACTERISTIC_UUID         	"1083b9a4-fdc0-4aa6-b027-a2600c8837c4"

// Light controlling services; multiple instances allowed
#define MW4_BLE_LIGHT_DEVICE_SERVICE_UUID                        "0ba35e90-f55f-4f15-9347-3dc4a0287881"
#define MW4_BLE_CAPABILITIES_CHARACTERISTIC_UUID                 "ed8128c1-5bfa-418c-9a4d-af2596f92952"
#define MW4_BLE_ID_CHARACTERISTIC_UUID                           "63c62656-807f-4db4-97d3-94095962acf8"
#define MW4_BLE_STATE_CHARACTERISTIC_UUID                        "c2af353b-e5fc-4bdf-b743-5d226f1198a2"
#define MW4_BLE_MODE_CHARACTERISTIC_UUID                         "b54fc13b-4374-4a6f-861f-dd198f88f299"
#define MW4_BLE_CYCLE_COLOR_CHARACTERISTIC_UUID                  "dfe34849-2d42-4222-b6b1-617a4f4d0869"
#define MW4_BLE_HUE_CHARACTERISTIC_UUID                          "19dfe175-aa12-404b-843d-b625937cffff"
#define MW4_BLE_SATURATION_CHARACTERISTIC_UUID                   "946d22e6-2b2f-49e7-b941-150b023f2261"
#define MW4_BLE_VALUE_CHARACTERISTIC_UUID                        "6c5df188-2e69-4f2f-b4ab-9d2b76ef7aa9"
#define MW4_BLE_ADDRESSABLE_DATA_CHARACTERISTIC_UUID             "8d4510e9-8565-49a4-8813-f45f49061833"

// Text display matrix service
#define MW4_BLE_TEXT_DISPLAY_SERVICE_UUID                             "aafca82b-95ae-4f33-9cf3-7ee0ef15ddf4"
#define MW4_BLE_TEXT_DISPLAY_TEXT_CHARACTERISTIC_UUID                 "c5b56d2e-b6e9-49c7-b098-5af9a75f46cd"
#define MW4_BLE_TEXT_DISPLAY_OFFSET_CHARACTERISTIC_UUID               "a0f348d3-5c80-420f-bb5f-1732824ac215"
#define MW4_BLE_TEXT_DISPLAY_SCROLLING_CHARACTERISTIC_UUID            "966c3d0d-493c-4b59-ae1d-d3768ff9ada8"
#define MW4_BLE_TEXT_DISPLAY_SCROLL_SPEED_CHARACTERISTIC_UUID         "1fc1cf59-0e88-4dda-81fc-818a14da216b"
#define MW4_BLE_TEXT_DISPLAY_PAUSE_TIME_CHARACTERISTIC_UUID           "7a991117-a277-46ae-8d73-0a7e7e6ae7e8"
#define MW4_BLE_TEXT_DISPLAY_BRIGHTNESS_CHARACTERISTIC_UUID           "48387eca-eedf-40ee-ab37-b4fb3a18cdf1"
#define MW4_BLE_TEXT_DISPLAY_FG_COLOR_CHARACTERISTIC_UUID             "4119bf67-6295-4ec9-b596-5b32a3f2fda5"
#define MW4_BLE_TEXT_DISPLAY_BG_COLOR_CHARACTERISTIC_UUID             "1b56faa0-376a-432a-a79a-e1a4f12dd493"
