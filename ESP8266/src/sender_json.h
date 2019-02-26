#ifndef _SENDERJSON_h
#define _SENDERJSON_h

#include "setup.h"

#include "master_i2c.h"
#include "Logging.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#ifdef SEND_JSON


/*
Функция отправляющая данные в JSON на TCP сервер.
URL HTTP сервера: sett.hostname_json
*/
bool send_json(const Settings &sett, const String &data)
{
    HTTPClient http;
    WiFiClient client;
    
    bool connect = false;

    if (strnlen(sett.hostname_json, HOSTNAME_JSON_LEN))
    {
        LOG_NOTICE("JSN", "Making HTTP connection to: " << sett.hostname_json);

        http.setTimeout(SERVER_TIMEOUT);
        connect = http.begin(client, sett.hostname_json);      //Specify request destination
        
        if (connect) {

            http.addHeader("Content-Type", "application/json"); 
            http.addHeader("Connection", "close");


            int httpCode = http.POST(data);   //Send the request
            String payload = http.getString();  //Get the response payload

            if (httpCode == 200) {
                LOG_NOTICE("JSN", httpCode);
            } else {
                LOG_ERROR("JSN", httpCode);
            }
            
            LOG_NOTICE("JSN", payload);
            http.end();  //Close connection
            
        } else {
            LOG_ERROR("JSN", "Connection to server failed");
        }
    }
      
    return false;
}

#endif  // SEND_JSON
#endif
