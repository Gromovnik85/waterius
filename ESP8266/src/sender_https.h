#include "Logging.h"
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include "setup.h"


bool send_https(WiFiClient &client, const char *hostname, const String &data, String &responce) 
{
    bool connect = false;
    
    HTTPClient http;
    http.setTimeout(SERVER_TIMEOUT);

    if (http.begin(client, "192.168.10.50", 5000, "/")) {  // hostname); 
        http.addHeader("Content-Type", "application/json"); 
        http.addHeader("Connection", "close");

        int httpCode = http.POST(data);   //Send the request
        
        if (httpCode == HTTP_CODE_OK) {
            responce = http.getString();
            LOG_NOTICE("JSN", httpCode);
            http.end(); //Close connection
            return true;
        } 
        LOG_ERROR("JSN", httpCode);

        http.end();  //Close connection

    }
    return false;
}