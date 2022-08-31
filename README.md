# BLE protocol

All literal UUIDs can be found in `config.h` and are referenced by name here.

## Costume controller service
This service is advertised as `MW4_BLE_COSTUME_CONTROL_SERVICE_UUID`. Centrals (BLE clients, i.e. your phone) should query for this service, connect, then enumerate other (non advertised) services to discover costume capabilities.

The following characteristics exist:
* `MW4_BLE_COSTUME_CONTROL_FW_VERSION_CHARACTERISTIC_UUID`: 1 byte unsigned int representing the version of the currently installed firmware
* `MW4_BLE_COSTUME_CONTROL_OTA_CHARACTERISTIC_UUID`: 512 byte writable buffer to transfer new firmare; see below

## Text display service
The `MW4_BLE_TEXT_DISPLAY_SERVICE_UUID` service controls the front panel text. The following characteristics exist:
* `MW4_BLE_TEXT_DISPLAY_TEXT_CHARACTERISTIC_UUID`: UTF-8 string to display
* `MW4_BLE_TEXT_DISPLAY_OFFSET_CHARACTERISTIC_UUID` : 2 byte little-endian signed int representing the offset to show the text at (see below)
* `MW4_BLE_TEXT_DISPLAY_SCROLLING_CHARACTERISTIC_UUID`: 1 byte unsigned int (converted to boolean) to enable (1) automatic text scrolling (default), or disable it (1)
* `MW4_BLE_TEXT_DISPLAY_SCROLL_SPEED_CHARACTERISTIC_UUID`: 1 byte unsigned int controlling scrolling speed. For now, this is simply the delay() between changes of the text offset
* `MW4_BLE_TEXT_DISPLAY_PAUSE_TIME_CHARACTERISTIC_UUID`: 1 byte unsigned int controlling the pause time for scrolling text, in seconds
* `MW4_BLE_TEXT_DISPLAY_BRIGHTNESS_CHARACTERISTIC_UUID`: 1 byte unsigned int overall panel brightness, as set by FastLED
* `MW4_BLE_TEXT_DISPLAY_FG_COLOR_CHARACTERISTIC_UUID`:  3 bytes (each an unsigned int), representing the RGB value for the foreground color for the text
* `MW4_BLE_TEXT_DISPLAY_BG_COLOR_CHARACTERISTIC_UUID`: as above but for the background color

Behavior note: if scrolling is disabled, text is displayed at the value set by the corresponding characteristic. If it is enabled, text starts at the right edge of the display and scrolls left until it is gone, in a loop. Additionally, if text fits in one screen (64 pixels / 10 characters), text pauses for a configurable amount of seconds once it reaches the center.

NB: an offset of 0 left align texts. An offset of negative panel length (technically -63 for us, because text starts 1 pixel from the edge when offset by 0) pushes the text to the right and out of the screen. An offset of (num_chars_in_text * 6) (because our font is 6 wide) pushes the text leftwards and out of the screen (allowing it to have scrolled from right to left and out completely).

TODO: as implemented, only one of these can exist, because some underlying variables for the text display and FastLED are shared.

## Light device service
TODO: adopt this from MWNext project.


# OTA updates

## Principle
The costume can receive Over-The-Air updates via BLE from the companion app.

To trigger an update, the process is as follows:
* Look for the device as usual by querying for the advertised Costume Controller service
* Query the FW version characteristic of this service to check the current version
* Check this version against some reference, e.g. the `deployment.json` file stored in S3 at `<REDACTED>`. Format to be nailed down later...
* If version checks passes, download the corresponding firmware file, e.g.: ``http://${manifest.host}${manifest.bin}`` where manifest is the parsed JSON file from above.
* Cut up the firmware file into 512 byte chunks

## Manifest file
Structure of `deployment.json`:
```
{
    "type": "MW4-base-firmware",
    "version": 2,
    "host": "<REDACTED>",
    "port": 80,
    "bin": "/firmware.bin",
    "check_signature": false
}
```

## S3 setup for firmware updates

Firmware is hosted in AWS S3 for OTA. To configure a bucket for this:

Add this policy:
```
{
    "Version": "2012-10-17",
    "Statement": [
        {
            "Sid": "PublicReadGetObject",
            "Effect": "Allow",
            "Principal": "*",
            "Action": "s3:GetObject",
            "Resource": "arn:aws:s3:::<bucket_name_here>/*"
        }
    ]
}
```

Add this CORS configuration:
```
[
    {
        "AllowedHeaders": [
            "*"
        ],
        "AllowedMethods": [
            "GET",
            "HEAD",
            "PUT",
            "POST"
        ],
        "AllowedOrigins": [
            "*"
        ],
        "ExposeHeaders": [],
        "MaxAgeSeconds": 3000
    }
]
```

Enable static website mode in bucket Properties (just use index.html as the index when asked, it doesn't need to exist).

Build URLs for files in the bucket based on the Bucket website endpoint given in the bucket properties.

NB: don't forget to set metadata on objects in the metadata to give them `Cache-Control: no-cache`. There doesn't seem to be a good way to automate this easily, and this is lost on uploads of new versions, unfortunately.
