
#include "Logging.h"
#include <user_interface.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include "ESP8266httpUpdate.h"

#include "wifi_settings.h"
#include "master_i2c.h"
#include "setup_ap.h"
#include "sender_blynk.h"
#include <ArduinoJson.h>
#include "sender_https.h"
#include "utils.h"

MasterI2C masterI2C; // Для общения с Attiny85 по i2c

SlaveData data; // Данные от Attiny85
Settings sett;  // Настройки соединения и предыдущие показания из EEPROM


static const char digicert[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIID/TCCAuWgAwIBAgIJAJ3fU5UnunjmMA0GCSqGSIb3DQEBBQUAMFwxCzAJBgNV
BAYTAkFVMRMwEQYDVQQIEwpTb21lLVN0YXRlMSEwHwYDVQQKExhJbnRlcm5ldCBX
aWRnaXRzIFB0eSBMdGQxFTATBgNVBAMTDDE5Mi4xNjguMS40MjAeFw0xOTAyMjQw
MTEzMDhaFw0zMDA1MTMwMTEzMDhaMFwxCzAJBgNVBAYTAkFVMRMwEQYDVQQIEwpT
b21lLVN0YXRlMSEwHwYDVQQKExhJbnRlcm5ldCBXaWRnaXRzIFB0eSBMdGQxFTAT
BgNVBAMTDDE5Mi4xNjguMS40MjCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoC
ggEBAL1wPb/g+A+TkzS9KiHi5rMv81jMaFERqhTp3yThVoKTmB0+2oUdttdTvWqH
zuXg30dH5E8tR2oPRmhkaDoiev6n8feAH3Nweu6R9w7EO9DR42ETMK2YXxVIKi8t
VjxvGhEy1cTX8Z23gwP0PWFkW7vutSLrU1GDd+vtASRT4AcWfuggDWb61dj/dSaT
Hr4X8wNk3fMGHt+MgZteE86JACc9GNFOGJlfIhPLD9hPs22PhK6Q7rgWtT2UGqhn
H17IBr9ItmqhM9NtlelG8O8UuHVMqkr/AFr/HU4zxEUAB0+FNCVa7riPeWET7ugA
kL7IqgKKzW2QUZUq1j4bEyzzlLcCAwEAAaOBwTCBvjAdBgNVHQ4EFgQUKJ6eyeip
kIdvoSOmLSVq+9NCTkkwgY4GA1UdIwSBhjCBg4AUKJ6eyeipkIdvoSOmLSVq+9NC
TkmhYKReMFwxCzAJBgNVBAYTAkFVMRMwEQYDVQQIEwpTb21lLVN0YXRlMSEwHwYD
VQQKExhJbnRlcm5ldCBXaWRnaXRzIFB0eSBMdGQxFTATBgNVBAMTDDE5Mi4xNjgu
MS40MoIJAJ3fU5UnunjmMAwGA1UdEwQFMAMBAf8wDQYJKoZIhvcNAQEFBQADggEB
AK/Uqhn6Y2qR0onLLUO2bU8TI/9CY/UbWKOknrih8Pq1a5r145Pi/7y5sXJc+IhM
LTV/6YbILO6nQ1MiQTvd2jXCJwgclInk59KwHFw3s30Mh34IUrpmvpSH1W0bKlQS
TFElKzVdy76bXyVnStgp6142g+hbyqgM5DVzrV+km0YmfBvUeM4DSkixY09LQbV2
0EaXGB3TjIAP+taYJf/sOKfTq+q26TZDAnbW2bFge0LLrslvWEYUBgyACs1jqjUd
ihlEy/Dq+KYDBllDV0fmPCJA2Dny9Rnn+VIW0lKgFwIXe0kGyvRvs5A2SLhB02Ze
K9SgRnixMN8XORtmus7juGI=
-----END CERTIFICATE-----
)EOF";

/*
Выполняется однократно при включении
*/
void setup()
{
    memset(&data, 0, sizeof(data)); // На всякий случай
    LOG_BEGIN(115200);    //Включаем логгирование на пине TX, 115200 8N1
    LOG_NOTICE("ESP", "Booted");
    masterI2C.begin();    //Включаем i2c master
}

/*
Берем начальные показания и кол-во импульсов, 
вычисляем текущие показания по новому кол-ву импульсов
*/
void calculate_values(Settings &sett, SlaveData &data, float *channel0, float *channel1)
{

    LOG_NOTICE("ESP", "new impulses=" << data.impulses0 << " " << data.impulses1);

    if (sett.liters_per_impuls > 0) {
        *channel0 = sett.channel0_start + (data.impulses0 - sett.impules0_start) / 1000.0 * sett.liters_per_impuls;
        *channel1 = sett.channel1_start + (data.impulses1 - sett.impules1_start) / 1000.0 * sett.liters_per_impuls;
        LOG_NOTICE("ESP", "new values=" << *channel0 << " " << *channel1);
    }
}

void loop()
{
    float channel0, channel1;
    uint8_t mode = TRANSMIT_MODE;

	// спрашиваем у Attiny85 повод пробуждения и данные
    //if (masterI2C.getMode(mode) && masterI2C.getSlaveData(data)) {
    if (true) {
        if (mode == SETUP_MODE) {
            //Режим настройки - запускаем точку доступа на 192.168.4.1

            //Загружаем конфигурацию из EEPROM
            loadConfig(sett);

            //Вычисляем текущие показания
            calculate_values(sett, data, &channel0, &channel1);

            //Запускаем точку доступа с вебсервером
            setup_ap(sett, data, channel0, channel1);
        }
        else {   
            // Режим передачи новых показаний
            if (!loadConfig(sett)) {
                LOG_ERROR("ESP", "error loading config");
            }
            else {
                //Вычисляем текущие показания
                calculate_values(sett, data, &channel0, &channel1);

                LOG_NOTICE("WIF", "Starting Wifi");
                //WiFi.mode(WIFI_STA);
                
                //WifiManager уже записал ssid & pass в Wifi, поэтому не надо самому заполнять
                WiFi.begin(); 

                //Ожидаем подключения к точке доступа
                uint32_t start = millis();
                while (WiFi.status() != WL_CONNECTED && millis() - start < ESP_CONNECT_TIMEOUT) {

                    LOG_NOTICE("WIF", "Wifi status: " << WiFi.status());
                    delay(200);
                }

                if (WiFi.status() == WL_CONNECTED) {

                    LOG_NOTICE("WIF", "Connected, got IP address: " << WiFi.localIP().toString());

#ifdef SEND_BLYNK
                    if (send_blynk(sett, data, channel0, channel1)) {
                        LOG_NOTICE("BLK", "send ok");
                    }
#endif
                    StaticJsonBuffer<2000> jsonBuffer;
                    JsonObject& root = jsonBuffer.createObject();
                    String output;

#ifdef SEND_HTTPS
                    if (setClock()) {
                        root["key"] = sett.key;
                        root["version"] = data.version;
                        root["version_esp"] = FIRMWARE_VERSION;
                        root["boot"] = data.service;  // 2 - reset pin, 3 - power
                        root["resets"] = data.resets;
                        root["voltage"] = (float)(data.voltage / 1000.0);
                        root["good"] = data.diagnostic;
                        root["ch0"] = channel0;
                        root["ch1"] = channel1;
                        root["delta0"] = (channel0 - sett.channel0_previous)*1000;  // litres
                        root["delta1"] = (channel1 - sett.channel1_previous)*1000;
                        root["ca_crc"] = CRC16(0, (char*)digicert, strlen(digicert));
                        root["ca2_crc"] = CRC16(0, sett.ca, strlen(sett.ca));

                        root.printTo(output);
                        LOG_NOTICE("JSN", output);

                        BearSSL::WiFiClientSecure client;
                        BearSSL::X509List cert;
                        cert.append(digicert);
                        //client.setTrustAnchors(&cert);
                        client.setInsecure();
                        
                        String responce;
                        if (send_https(client, sett.hostname_json, output, responce)) {
                            JsonObject& root = jsonBuffer.parseObject(responce);
                            if (!root.success()) {
                                if (root.containsKey("ca2")) {
                                    strncpy0(sett.ca, root["ca2"], CERT_LEN);
                                    LOG_NOTICE("JSN", "Key CA2 updated");
                                } else {
                                    LOG_NOTICE("JSN", "No CA2 in response");
                                    LOG_NOTICE("JSN", responce);
                                }
                            } else {
                                LOG_ERROR("JSN", "parse response error");
                            }
                        }
                    }
#endif

                    //Сохраним текущие значения в памяти.
                    sett.channel0_previous = channel0;
                    sett.channel1_previous = channel1;
                    storeConfig(sett);
                }
            }
        }
    }

    LOG_NOTICE("ESP", "Going to sleep");
    masterI2C.sendCmd('Z');        // "Можешь идти спать, attiny"
    LOG_END();
    
    twi_stop();
    ESP.deepSleep(0, RF_DEFAULT);  // Спим до следущего включения EN
}
