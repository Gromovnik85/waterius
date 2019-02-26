#ifndef _WATERIUS_WIFI_SETTINGS_h
#define _WATERIUS_WIFI_SETTINGS_h

#include "setup.h"

#include <Arduino.h>
#include <WiFiClient.h>
#include "master_i2c.h"

#define FAKE_CRC 0412

/*
Сохраняем конфигурацию в EEPROM
*/
void storeConfig(const Settings &sett);

/*
Читаем конфигурацию из EEPROM
*/
bool loadConfig(Settings &sett);

#endif

