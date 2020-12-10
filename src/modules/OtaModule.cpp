#include "OtaModule.h"


void OtaModule::init(WiFiClient* wiFiClient, char hostName[])
{
    this->_wiFiClient = wiFiClient;

    ArduinoOTA.setPort(OTA_PORT);   // Port defaults to 8266

    SerialPrint("[OTA] Hostname:"); SerialPrintln(hostName);
    SerialPrint("[OTA] Port:"); SerialPrintlnWithFormat(OTA_PORT, DEC);
    ArduinoOTA.setHostname(hostName);

    // No authentication by default?
    ArduinoOTA.setPassword((const char *)"123");

    ArduinoOTA.onStart([]() {
        SerialPrint("Start OTA");
        if (ArduinoOTA.getCommand() == U_FLASH) {
            SerialPrintln(" (Type: sketch)");
        }
        else {
            // U_SPIFFS
            // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
            SPIFFS.end();
            SerialPrintln(" (Type: filesystem)");
        }
    });
    ArduinoOTA.onEnd([]() {
        SerialPrintln("\nEnd OTA");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        SerialPrintf("OTA Progress: %u%%", (progress / (total / 100)));
        SerialPrintln();
    });
    ArduinoOTA.onError([](ota_error_t error) {
        SerialPrintf("OTA Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) SerialPrintln("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) SerialPrintln("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) SerialPrintln("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) SerialPrintln("Receive Failed");
        else if (error == OTA_END_ERROR) SerialPrintln("End Failed");
    });
    ArduinoOTA.begin();
}


void OtaModule::run() {
    ArduinoOTA.handle();
}








