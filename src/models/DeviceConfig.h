
#ifndef DIYHUE_LIGHTS_FIRMWARE_DEVICECONFIG_H
#define DIYHUE_LIGHTS_FIRMWARE_DEVICECONFIG_H

#include <Arduino.h>
#include <ESP8266WiFi.h>

#define STARTUP_MODE_POWERFAIL	0
#define STARTUP_MODE_SAFETY		1
#define STARTUP_MODE_OFF		2

#define DEVICE_NAME_MAX_LENGTH  63 + 1 // +1 = NULL TERMINATOR
#define IP_AS_STR_MAX_LENGTH    15 + 1

class DeviceConfig {
public:
    uint8_t device_ip[4];
    uint8_t gateway_ip[4];
    uint8_t subnet_mask[4];
    uint8_t scene;
#ifdef LIGHT_TYPE_RGB
    uint8_t rgbMultiplier[CHANNELS_COUNT]; // light multiplier in percentage /R, G, B/
#endif // LIGHT_TYPE_RGB


    void saveNetworkData(IPAddress* deviceIp, IPAddress* gatewayIp, IPAddress* subnetMask);
    void saveNetworkData(uint8_t deviceIp[4], uint8_t gatewayIp[4], uint8_t subnetMask[4]);
    void networdDataToStr(char ipBuf[IP_AS_STR_MAX_LENGTH], char gatwBuf[IP_AS_STR_MAX_LENGTH], char sbnmskBuf[IP_AS_STR_MAX_LENGTH]);
    void setStartMode(uint8_t mode);
    bool isStartupModePowerFail();
    bool isStartupModeSafety();
    bool isStartupModeOff();
    uint8_t getStartupMode();
    void setLightName(String* name);
    char* getLightName();

    void debugPrint();

private:
    char _lightName[DEVICE_NAME_MAX_LENGTH];
    uint8_t _startupMode;
};


#endif //DIYHUE_LIGHTS_FIRMWARE_DEVICECONFIG_H
