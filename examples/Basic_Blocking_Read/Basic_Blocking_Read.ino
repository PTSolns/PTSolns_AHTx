// Example: Basic Blocking Read
// Last Update: Oct 28, 2025
//
// DESCRIPTION
// 1) Reads temperature and humidity once per second using the blocking API
// 2) Shows Celsius and RH on Serial
// 3) begin uses Wire at 100 kHz and address 0x38 and auto detects the sensor model
//
// REFERENCES
// Website: http://PTSolns.com
// Documentation Repository: http://docs.PTSolns.com
// Newsletter Signup: http://eepurl.com/hnESjL
// Tinker Thoughts Blog: https://PTSolns.com/Tinker-Thoughts

/* NOTE 1
A blocking read means the function waits for the sensor to finish measuring before returning. 
When you call readTemperatureHumidity(), the program pauses for about 90 to 120 ms until the measurement is complete. 
During this time, no other code runs. This method is simple and easy to use, 
but if you need the microcontroller to continue doing other work while waiting for the sensor, use the non blocking example instead.
*/

/* NOTE 2
AHTxStatus is a status code returned by the library after calling functions such as readTemperatureHumidity(), startMeasurement(), or readResult().
It tells you if the operation succeeded or why it failed.

This allows your sketch to detect and diagnose issues such as I2C communication problems, incorrect usage, or data errors.

Value | Meaning

AHTX_OK
The function completed successfully and the temperature and humidity values are valid.

AHTX_I2CError
The sensor did not acknowledge a command or did not return the expected number of bytes. Most often caused by wiring issues, wrong I2C address, or bus speed problems.

AHTX_NotInitialized
You tried to read from the sensor before calling begin().

AHTX_BusyTimeout
The sensor did not finish the measurement in time. This only applies to non blocking mode when using startMeasurement() and pollReady().

AHTX_CrcMismatch
The received data failed the CRC (checksum) validation. The reading is discarded to avoid using corrupted data.

AHTX_InvalidParam
A function was called with an invalid argument.

AHTX_NotSupported
The requested action is not supported for this device for example heater control.
*/

#include <Wire.h>
#include <PTSolns_AHTx.h>

PTSolns_AHTx aht;

void setup() {
  Serial.begin(115200);
  while(!Serial) {}
  if (!aht.begin()) { Serial.println("AHT begin failed"); for(;;){} }
}

void loop() {
  float t, h;
  AHTxStatus st = aht.readTemperatureHumidity(t, h, 120);
  if (st == AHTX_OK) {
    Serial.print("T_C="); Serial.print(t, 2);
    Serial.print(" RH_="); Serial.println(h, 2);
  } else {
    Serial.print("Error="); Serial.println((int)st);
  }
  delay(1000);
}
