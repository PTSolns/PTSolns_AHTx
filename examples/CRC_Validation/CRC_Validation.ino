// Example: CRC Validation
// Last Update: Oct 28, 2025
//
// DESCRIPTION
// 1) Demonstrates enabling CRC validation for AHTx readings
// 2) If the module provides a CRC byte, it will be verified
// 3) If the CRC byte is missing or invalid, the library automatically disables CRC and continues safely
//
// REFERENCES
// Website: http://PTSolns.com
// Documentation Repository: http://docs.PTSolns.com
// Newsletter Signup: http://eepurl.com/hnESjL
// Tinker Thoughts Blog: https://PTSolns.com/Tinker-Thoughts

/* NOTE 1
CRC stands for Cyclic Redundancy Check. It is a method used to detect errors in data that was transmitted or read from a device.

How it works in simple terms
- The sensor collects measurement data (temperature and humidity).
- It runs the data through a math formula and produces one extra number called a CRC.
- The library runs the same formula on the received data.
- If both numbers match, the data is considered valid. If not, the data may be corrupted and should not be trusted.

Why CRC is useful
- Detects data corruption caused by noise on the I2C lines
- Helps ensure temperature or humidity readings are correct before using them
- Important in systems where reliability is critical

Example Output | Meaning
- CRC: Valid or Supported | Sensor provided CRC and it matched
- CRC: Not Supported/Fallback | Sensor did not provide CRC or CRC failed
*/

#include <Wire.h>
#include <PTSolns_AHTx.h>

PTSolns_AHTx aht;   // auto-detect model

void setup() {
  Serial.begin(115200);
  while(!Serial) {}

  if (!aht.begin()) {
    Serial.println("AHT begin failed");
    for(;;){}
  }

  // Enable CRC checking (safe enable)
  aht.enableCRC(true);
  Serial.println("CRC Enabled");
}

void loop() {
  float t, h;
  AHTxStatus st = aht.readTemperatureHumidity(t, h);

  if (st == AHTX_OK) {
    Serial.print("T_C="); Serial.print(t, 2);
    Serial.print(" RH="); Serial.print(h, 2);

    Serial.print(" | CRC: ");
    Serial.println(aht.crcEnabled() ? "Valid or Supported" : "Not Supported/Fallback");

  } else {
    Serial.print("Error="); Serial.println((int)st);
  }

  delay(1000);
}
