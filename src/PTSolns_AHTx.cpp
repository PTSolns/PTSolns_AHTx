#include "PTSolns_AHTx.h"
static const uint8_t CMD_SOFT_RESET = 0xBA;
static const uint8_t CMD_STATUS     = 0x71;
static const uint8_t CMD_TRIGGER    = 0xAC;
static const uint8_t CMD_INIT_AHT20 = 0xBE;
static const uint8_t CMD_INIT_AHT10 = 0xE1;
static const uint8_t TRIG_P1 = 0x33;
static const uint8_t TRIG_P2 = 0x00;
bool PTSolns_AHTx::begin(TwoWire& w, uint8_t i2cAddr, uint32_t i2cClockHz) {
  _wire = &w; _addr = i2cAddr; _wire->begin();
#if defined(TWBR) || ARDUINO >= 100
  if (i2cClockHz >= 100000UL && i2cClockHz <= 400000UL) { _wire->setClock(i2cClockHz); }
#endif
  _inited = false;
  _last = softReset(); if (_last != AHTX_OK) return false; delay(40);
  if (!autoDetect_()) return false;
  _wire->beginTransmission(_addr); _wire->write(CMD_STATUS);
  if (_wire->endTransmission(false) != 0) return false;
  if (_wire->requestFrom((int)_addr, 1) != 1) return false;
  _status = _wire->read(); if (((_status >> 3) & 0x01) == 0) return false;
  _inited = true; return true;
}
bool PTSolns_AHTx::autoDetect_() {
  _wire->beginTransmission(_addr); _wire->write(CMD_INIT_AHT20); _wire->write(0x08); _wire->write(0x00);
  if (_wire->endTransmission() == 0) { _type = AHT20; delay(10); return true; }
  _wire->beginTransmission(_addr); _wire->write(CMD_INIT_AHT10); _wire->write(0x08); _wire->write(0x00);
  if (_wire->endTransmission() == 0) { _type = AHT10; delay(10); return true; }
  return false;
}
void PTSolns_AHTx::end() { _inited = false; _measuring = false; }
AHTxStatus PTSolns_AHTx::softReset() {
  _wire->beginTransmission(_addr); _wire->write(CMD_SOFT_RESET);
  if (_wire->endTransmission() != 0) return _last = AHTX_I2CError;
  delay(20); return _last = AHTX_OK;
}
AHTxStatus PTSolns_AHTx::reinitialize() {
  uint8_t cmd = (_type == AHT10) ? CMD_INIT_AHT10 : CMD_INIT_AHT20;
  _wire->beginTransmission(_addr); _wire->write(cmd); _wire->write(0x08); _wire->write(0x00);
  if (_wire->endTransmission() != 0) return _last = AHTX_I2CError;
  delay(10); return _last = AHTX_OK;
}
AHTxStatus PTSolns_AHTx::triggerMeasurement_() {
  _wire->beginTransmission(_addr); _wire->write(CMD_TRIGGER); _wire->write(TRIG_P1); _wire->write(TRIG_P2);
  if (_wire->endTransmission() != 0) return _last = AHTX_I2CError;
  _measuring = true; _lastStartMs = millis(); return _last = AHTX_OK;
}
AHTxStatus PTSolns_AHTx::readTemperatureHumidity(float& tC, float& rh, uint32_t timeoutMs) {
  if (!_inited) return _last = AHTX_NotInitialized;
  _wire->beginTransmission(_addr); _wire->write(CMD_TRIGGER); _wire->write(TRIG_P1); _wire->write(TRIG_P2);
  if (_wire->endTransmission() != 0) return _last = AHTX_I2CError;
  uint32_t waitMs = timeoutMs > 90 ? timeoutMs : 90; delay(waitMs);
  uint8_t need = _useCRC ? 7 : 6; uint8_t buf[7] = {0};
  int got = _wire->requestFrom((int)_addr, (int)need);
  if (got != need) {
    if (_useCRC && got == 6) { for (int i=0;i<6;i++) buf[i] = _wire->read(); _useCRC = false; }
    else return _last = AHTX_I2CError;
  } else { for (int i=0;i<need;i++) buf[i] = _wire->read(); }
  if (_useCRC) { uint8_t calc = crc8_(buf, 6); if (calc != buf[6]) { _useCRC = false; } }
  _status = buf[0];
  parse_(buf, tC, rh); tC += _tOff; rh += _hOff; if (rh < 0) rh = 0; if (rh > 100) rh = 100;
  _measuring = false; return _last = AHTX_OK;
}
AHTxStatus PTSolns_AHTx::startMeasurement() { if (!_inited) return _last = AHTX_NotInitialized; return triggerMeasurement_(); }
bool PTSolns_AHTx::pollReady() {
  if (!_inited || !_measuring) return false;
  _wire->beginTransmission(_addr); _wire->write(CMD_STATUS);
  if (_wire->endTransmission(false) != 0) return false;
  if (_wire->requestFrom((int)_addr, 1) != 1) return false;
  _status = _wire->read(); return !busy_();
}
AHTxStatus PTSolns_AHTx::readResult(float& tC, float& rh) {
  if (!_inited) return _last = AHTX_NotInitialized;
  if (_measuring && busy_()) return _last = AHTX_BusyTimeout;
  uint8_t need = _useCRC ? 7 : 6; uint8_t buf[7] = {0};
  int got = _wire->requestFrom((int)_addr, (int)need);
  if (got != need) {
    if (_useCRC && got == 6) { for (int i=0;i<6;i++) buf[i]=_wire->read(); _useCRC = false; }
    else return _last = AHTX_I2CError;
  } else { for (int i=0;i<need;i++) buf[i]=_wire->read(); }
  _status = buf[0];
  if (_useCRC) { uint8_t calc = crc8_(buf, 6); if (calc != buf[6]) { _useCRC = false; } }
  parse_(buf, tC, rh); tC += _tOff; rh += _hOff; if (rh < 0) rh = 0; if (rh > 100) rh = 100;
  _measuring = false; return _last = AHTX_OK;
}
AHTxStatus PTSolns_AHTx::setHeater(bool /*on*/) { return _last = AHTX_NotSupported; }
AHTxStatus PTSolns_AHTx::readRaw_(uint8_t* buf, size_t n) {
  if (_wire->requestFrom((int)_addr, (int)n) != (int)n) return _last = AHTX_I2CError;
  for (size_t i=0;i<n;i++) buf[i] = _wire->read(); _status = buf[0]; return _last = AHTX_OK;
}
void PTSolns_AHTx::parse_(const uint8_t* b, float& tC, float& rh) {
  uint32_t rawHum = ((uint32_t)b[1] << 12) | ((uint32_t)b[2] << 4) | ((uint32_t)b[3] >> 4);
  uint32_t rawTemp = (((uint32_t)b[3] & 0x0F) << 16) | ((uint32_t)b[4] << 8) | (uint32_t)b[5];
  rh = (rawHum * 100.0f) / 1048576.0f; tC = (rawTemp * 200.0f) / 1048576.0f - 50.0f;
}
uint8_t PTSolns_AHTx::crc8_(const uint8_t* data, size_t len) {
  uint8_t crc = 0xFF;
  for (size_t i = 0; i < len; ++i) { crc ^= data[i];
    for (uint8_t b = 0; b < 8; ++b) { if (crc & 0x80) crc = (crc << 1) ^ 0x31; else crc <<= 1; } }
  return crc;
}
