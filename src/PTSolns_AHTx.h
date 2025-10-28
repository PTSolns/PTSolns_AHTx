#pragma once
#include <Arduino.h>
#include <Wire.h>

enum AHTxType : uint8_t { AHT10 = 10, AHT20 = 20, AHT21 = 21 };

enum AHTxStatus : uint8_t {
  AHTX_OK = 0,
  AHTX_I2CError,
  AHTX_NotInitialized,
  AHTX_BusyTimeout,
  AHTX_CrcMismatch,
  AHTX_InvalidParam,
  AHTX_NotSupported
};

struct AHTxReading {
  float temperatureC;
  float humidityRH;
};

class PTSolns_AHTx {
public:
  explicit PTSolns_AHTx(AHTxType typeHint = AHT20) : _type(typeHint) {}
  bool begin(TwoWire& w = Wire, uint8_t i2cAddr = 0x38, uint32_t i2cClockHz = 100000);
  void end();
  AHTxStatus readTemperatureHumidity(float& tC, float& rh, uint32_t timeoutMs = 120);
  AHTxStatus startMeasurement();
  bool       pollReady();
  AHTxStatus readResult(float& tC, float& rh);
  AHTxStatus softReset();
  AHTxStatus reinitialize();
  AHTxStatus setHeater(bool on);
  void setTemperatureOffsetC(float offC) { _tOff = offC; }
  void setHumidityOffsetPct(float offPct) { _hOff = offPct; }
  static float toFahrenheit(float c) { return c * 9.0f / 5.0f + 32.0f; }
  static float toKelvin(float c)     { return c + 273.15f; }
  AHTxStatus lastError() const { return _last; }
  uint8_t    lastStatusByte() const { return _status; }
  uint8_t    address() const { return _addr; }
  AHTxType   sensorType() const { return _type; }
  void  enableCRC(bool en) { _useCRC = en; }
  bool  crcEnabled() const { return _useCRC; }
private:
  bool       autoDetect_();
  AHTxStatus triggerMeasurement_();
  AHTxStatus readRaw_(uint8_t* buf, size_t n);
  bool       busy_() const { return (_status & 0x80) != 0; }
  static uint8_t crc8_(const uint8_t* data, size_t len);
  static void parse_(const uint8_t* buf, float& tC, float& rh);
  TwoWire*   _wire = nullptr;
  uint8_t    _addr = 0x38;
  AHTxType   _type = AHT20;
  bool       _inited = false;
  bool       _measuring = false;
  bool       _useCRC = false;
  uint8_t    _status = 0;
  uint32_t   _lastStartMs = 0;
  float      _tOff = 0.0f;
  float      _hOff = 0.0f;
  AHTxStatus _last = AHTX_OK;
};
