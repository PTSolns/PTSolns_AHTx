// Example: Non-Blocking Periodic Read
// Last Update: Oct 28, 2025
//
// DESCRIPTION
// 1) Demonstrates the non blocking API with a periodic scheduler
// 2) startMeasurement begins a conversion then loop polls pollReady
// 3) When ready readResult returns temperature and humidity
//
// REFERENCES
// Website: http://PTSolns.com
// Documentation Repository: http://docs.PTSolns.com
// Newsletter Signup: http://eepurl.com/hnESjL
// Tinker Thoughts Blog: https://PTSolns.com/Tinker-Thoughts

#include <Wire.h>
#include <PTSolns_AHTx.h>

PTSolns_AHTx aht;

enum { PERIOD_MS = 2000 };
uint32_t tNext = 0;
bool measuring = false;

void setup() {
  Serial.begin(115200);
  while(!Serial) {}
  if (!aht.begin()) { Serial.println("AHT begin failed"); for(;;){} }
}

void loop() {
  const uint32_t now = millis();

  if (!measuring && now >= tNext) {
    if (aht.startMeasurement() == AHTX_OK) { measuring = true; }
    else { Serial.println("startMeasurement failed"); tNext = now + PERIOD_MS; }
  }

  if (measuring && aht.pollReady()) {
    float t, h;
    AHTxStatus st = aht.readResult(t, h);
    if (st == AHTX_OK) {
      Serial.print("T="); Serial.print(t, 2);
      Serial.print(" RH="); Serial.println(h, 2);
    } else {
      Serial.print("Error="); Serial.println((int)st);
    }
    measuring = false;
    tNext = now + PERIOD_MS;
  }
}
