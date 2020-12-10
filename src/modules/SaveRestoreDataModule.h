
#ifndef DIYHUE_LIGHTS_FIRMWARE_SAVERESTOREDATAMODULE_H
#define DIYHUE_LIGHTS_FIRMWARE_SAVERESTOREDATAMODULE_H

//#include <Arduino>
#include <ArduinoJson.h>
#include <FS.h>

#include "EepromModule.h"
#include "../internal_cfg.h"
#include "../models/AdvancedSerial.h"
#include "../models/LightData.h"
#include "../models/DeviceConfig.h"

//#define USED_EEPROM_SIZE    256

#define SAVED_JSON_KEY_DATA_VERSION           "dv"
#define SAVED_JSON_KEY_DEVICE_NAME            "n"
#define SAVED_JSON_KEY_STARTUP_MODE           "m"
#define SAVED_JSON_KEY_SCENE                  "s"
#define SAVED_JSON_KEY_DEVICE_IP              "ip"
#define SAVED_JSON_KEY_DEVICE_GATEWAY         "g"
#define SAVED_JSON_KEY_DEVICE_SUBNET_MASK     "sm"

#define SAVED_JSON_SIZE_CONFIG      400


class SaveRestoreDataModule {
public:
    static bool init();
    static bool getFileContent(const char* fileName, char buffer[], uint16_t bufferSize);
    static bool saveConfig(DeviceConfig* config);
    static void restoreConfig(DeviceConfig* config);

    static bool saveAllLightsData(LightData lightsData[DEVICE_LIGHTS_COUNT]);
    static bool saveLightData(LightData* lightData, uint8_t lightIndex);

    static bool restoreAllLightsData(LightData lightsData[DEVICE_LIGHTS_COUNT]);
    static bool restoreLightData(LightData* lightData, uint8_t lightIndex);


private:
    static bool _isSpiffsInit;

    static bool saveJsonToSpiffs(const char* filename, DynamicJsonDocument* json);
};


#endif //DIYHUE_LIGHTS_FIRMWARE_SAVERESTOREDATAMODULE_H
