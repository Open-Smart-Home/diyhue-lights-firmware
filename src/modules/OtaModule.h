
#ifndef FIRMWARE_OTAMODULE_H
#define FIRMWARE_OTAMODULE_H

#include <Arduino.h>
#include <ArduinoOTA.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

#include "../models/AdvancedSerial.h"

#define OTA_PORT    8266

class OtaModule {
public:
    //void init(BearSSL::WiFiClientSecure* wiFiClient);
    void init(WiFiClient* wiFiClient, char hostName[]);
    void run();

private:
//    BearSSL::WiFiClientSecure* _wiFiClient;
    WiFiClient* _wiFiClient;

};


#endif //FIRMWARE_OTAMODULE_H
