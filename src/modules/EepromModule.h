#ifndef FIRMWARE_EEPROMMODULE_H
#define FIRMWARE_EEPROMMODULE_H

#include <Arduino.h>
#include <EEPROM.h>

#include "../../Config.h"
#include "../models/DeviceConfig.h"



#define USED_EEPROM_SIZE        256
#define EEPROM_DEFAULT_VALUE    0xFF

//-----------------------------------------------------------------------------------------------
// EEPROM ADDRESSES CONFIGURATION
//-----------------------------------------------------------------------------------------------
#define EEPROM_ADDR_CONTROL_VAL		0
//#define EEPROM_ADDR_IS_ON			1   // not used anymore ? (due to LightData)
//#define EEPROM_ADDR_STARTUP_MODE	2   // not used anymore ? (due to DeviceConfig)
//#define EEPROM_ADDR_SCENE			3   // not used anymore ? (due to DeviceConfig)

// addr and size of 'DeviceConfig' class
#define ADDR_CONFIG_INIT            16  
#define CONFIG_SIZE                 sizeof(DeviceConfig)
#define ADDR_CONFIG_END             ADDR_CONFIG_INIT + CONFIG_SIZE - 1

// the light data must be left as last data to save due to it may contain multiple lights
#define ADDR_LIGHT_DATA_INIT        ADDR_CONFIG_END + 2
#define LIGHT_DATA_SIZE             sizeof(LightDataEeprom)
#define ADDR_LIGHT_DATA_END         ADDR_LIGHT_DATA_INIT + (LIGHT_DATA_SIZE * DEVICE_LIGHTS_COUNT) - 1

#define EEPROM_ADDR_COLOR_MODE		4   
#define EEPROM_ADDR_HUE				5	// int = 2 byte
#define EEPROM_ADDR_CT				7	// int = 2 byte
#define EEPROM_ADDR_BRI				9	// int = 2 byte
#define EEPROM_ADDR_X				11	// float = 4 byte
#define EEPROM_ADDR_Y				15	// float = 4 byte



//#define EEPROM_ADDR_COLORS_START	30

//-----------------------------------------------------------------------------------------------
// OTHER EEPROM PARAMS
//-----------------------------------------------------------------------------------------------
#define EEPROM_CTRL_VAL				23
#define EEPROM_SAVE_STATE_DELAY_MS	2000



class EepromModule {
public:
    static void eraseAll();
    static void printEepromDump();


    static bool saveConfig(DeviceConfig* config);
    static void restoreConfig(DeviceConfig* config);

    static void testEeprom();
    static void writeEepromDataControlIfNotSet();
    static bool isEpromDataCompliant();

    static void eepromBegin();
    static void eepromCommitAndEnd(); // wrapper for EEPROM.commit() with additional operation
    static void eepromEnd();

private:
    static void debugPrintWriteAddress(uint16_t addr, bool addEol = true);
    static void debugPrintAddressAndByteUse(const char* fieldName, uint16_t addr, uint16_t usedByte);
};


#endif //FIRMWARE_EEPROMMODULE_H
