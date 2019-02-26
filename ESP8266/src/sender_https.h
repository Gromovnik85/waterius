#include "Logging.h"
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include "setup.h"


bool send_https(BearSSL::WiFiClientSecure &client, const char *hostname, const String &data, String &responce) 
{
    bool connect = false;
    
    HTTPClient http;
    http.setTimeout(SERVER_TIMEOUT);

    connect = http.begin(client, hostname); 
        
    if (connect) {
        http.addHeader("Content-Type", "application/json"); 
        http.addHeader("Connection", "close");

        int httpCode = http.POST(data);   //Send the request
        responce = http.getString();  //Get the response payload

        http.end();  //Close connection

        if (httpCode == 200) {
            LOG_NOTICE("JSN", httpCode);
            return true;
        } 
        LOG_ERROR("JSN", httpCode);
    }
    return false;
}