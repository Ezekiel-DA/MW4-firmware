#include "SX1509.h"

#include "config.h"

SX1509 io;

void setupSX1509() {
  Wire.begin();

  if (io.begin(SX1509_ADDRESS) == false) {
      Serial.println("Failed to communicate. Check wiring and address of SX1509.");
      while (1)
          ;  // If we fail to communicate, loop forever.
  }

  pinMode(SX1509_STATUS_LED_PIN, OUTPUT);
  digitalWrite(SX1509_STATUS_LED_PIN, LOW);
}