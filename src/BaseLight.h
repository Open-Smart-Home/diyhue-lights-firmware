
#ifndef BASE_LIGHT_H
#define BASE_LIGHT_H

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <ArduinoJson.h>

#include "models/AdvancedSerial.h"
#include "internal_cfg.h"
#include "models/LightData.h"
#include "models/DeviceConfig.h"
#include "modules/SaveRestoreDataModule.h"


// #ifdef LIGHT_TYPE_RGB || LIGHT_TYPE_RGBW
// #define COLOR_INDEX_RED         0
// #define COLOR_INDEX_GREEN       1
// #define COLOR_INDEX_BLUE        2
// #define COLOR_INDEX_WARM_WHITE  3
// #endif
// #ifdef LIGHT_TYPE_CCT
// #define COLOR_INDEX_WARM_WHITE  0
// #define COLOR_INDEX_COLD_WHITE  1
// #endif


//#define CHANNELS_COUNT	4

#define LIGHT_VERSION 2.1

#define LIGHT_MODEL_RGB_STRIP_PLUS  "LST002"
#define LIGHT_MODEL_RGB_AND_WHITE   "LCT015"


#define EEPROM_ADDR_CONTROL_VAL		0
//#define EEPROM_ADDR_IS_ON			1
//#define EEPROM_ADDR_STARTUP_MODE	2
//#define EEPROM_ADDR_SCENE			3
//#define EEPROM_ADDR_COLOR_MODE		4
//#define EEPROM_ADDR_HUE				5	// int = 2 byte
//#define EEPROM_ADDR_CT				7	// int = 2 byte
//#define EEPROM_ADDR_BRI				9	// int = 2 byte
//#define EEPROM_ADDR_X				11	// float = 4 byte
//#define EEPROM_ADDR_Y				15	// float = 4 byte

//#define EEPROM_ADDR_COLORS_START	30

#define EEPROM_CTRL_VAL				23
#define EEPROM_SAVE_STATE_DELAY_MS	2000

// Define values for 'alert' field that has info to displaying alerts by flashing the bulb either once or multiple times. 
// Valid values are: NONE, SELECT and LSELECT.
#define ALERT_NONE              "none"
#define ALERT_SINGLE_FLASH      "select"
#define ALERT_MULTIPLE_FLASH    "lselect"

// the single flash is a rapid ON/OFF flash
#define ALERT_SINGLE_FLASH_FLASHES_COUNT            1
#define ALERT_SINGLE_FLASH_FLASHES_FREQUENCY_MS     300      
// In the multiple flash, the light turn ON/OFF for 10 seconds
#define ALERT_MULTIPLE_FLASH_FLASHES_COUNT          10
#define ALERT_MULTIPLE_FLASH_FLASHES_FREQUENCY_MS   1000

#define COLOR_MODE_RGBW	0
#define COLOR_MODE_XY	1
#define COLOR_MODE_CT	2
#define COLOR_MODE_HUE	3

#define MAX_BRIGHTNESS      255
#define FLASH_INTENSITY     100 // value from 0 to MAX_BRIGHTNESS (lower value = lower intensity)

#define COLOR_TEMP_COLD	153
#define COLOR_TEMP_NEUTRAL	320
#define COLOR_TEMP_WARM	500

#if (defined LIGHT_TYPE_RGB)
#define COLOR_INDEX_WARM_WHITE	0
#define COLOR_INDEX_COLD_WHITE	1
#endif
#if (defined LIGHT_TYPE_RGB) || (defined LIGHT_TYPE_RGBW) || (defined LIGHT_TYPE_RGBCCT)
#define COLOR_INDEX_RED			0
#define COLOR_INDEX_GREEN		1
#define COLOR_INDEX_BLUE		2
#endif 
#if (defined LIGHT_TYPE_RGBW)
#define COLOR_INDEX_WHITE		3
#endif
#if (defined LIGHT_TYPE_RGBCCT)
#define COLOR_INDEX_WARM_WHITE	3
#define COLOR_INDEX_COLD_WHITE	4
#endif


#ifndef uint8_t
#define uint8_t unsigned char
#endif


// Define gamut triangle using the information from here:
// https://github.com/PhilipsHue/PhilipsHueSDK-iOS-OSX/blob/00187a3db88dedd640f5ddfa8a474458dff4e1db/ApplicationDesignNotes/RGB%20to%20xy%20Color%20conversion.md

//// HUE Bulb
//#define GAMUT_TRIANGLE_RED_X	0.675
//#define GAMUT_TRIANGLE_RED_Y	0.322
//#define GAMUT_TRIANGLE_GREEN_X	0.4091
//#define GAMUT_TRIANGLE_GREEN_Y	0.518
//#define GAMUT_TRIANGLE_BLUE_X	0.167
//#define GAMUT_TRIANGLE_BLUE_Y	0.04
//
//// LivingColors Bloom, Aura and Iris
//#define GAMUT_TRIANGLE_RED_X	0.704
//#define GAMUT_TRIANGLE_RED_Y	0.296
//#define GAMUT_TRIANGLE_GREEN_X	0.2151
//#define GAMUT_TRIANGLE_GREEN_Y	0.7106
//#define GAMUT_TRIANGLE_BLUE_X	0.138
//#define GAMUT_TRIANGLE_BLUE_Y	0.08

// Any other light
#define GAMUT_TRIANGLE_RED_X	1.0
#define GAMUT_TRIANGLE_RED_Y	0.0
#define GAMUT_TRIANGLE_GREEN_X	0.0
#define GAMUT_TRIANGLE_GREEN_Y	1.0
#define GAMUT_TRIANGLE_BLUE_X	0.0
#define GAMUT_TRIANGLE_BLUE_Y	0.0


// Check version of ArduinoJson
#if ARDUINOJSON_VERSION_MAJOR < 6
#error ">>> Please update ArduinoJson library to a version >= 6.0 <<<"
#endif



class BaseLight{
public:
    //BaseLight();
    void init(String* lightName);
    void setNetworkCfg(IPAddress deviceIp, IPAddress gatewayIp, IPAddress subnetMask);
    virtual void restoreLastState();
    virtual void saveState();
    virtual void notifyNotConnected();
    void run(bool isConnected);


    virtual void srvOnBaseReq(ESP8266WebServer* srvPtr);
    virtual void srvOnSwitch(ESP8266WebServer* srvPtr);
    virtual void srvOnSet(ESP8266WebServer* srvPtr);
    virtual void srvOnGet(ESP8266WebServer* srvPtr);
    virtual void srvOnDetect(ESP8266WebServer* srvPtr, byte* macPtr);

    virtual void srvOnStateGet(ESP8266WebServer* srvPtr);
    virtual void srvOnStatePut(ESP8266WebServer* srvPtr);
    virtual void srvOnConfigGet(ESP8266WebServer* srvPtr);



protected:
    static void initLightData(LightData* lightData_p);
    virtual void initPlatformSpecific() = 0;
    void setWhiteMaxBrightness();
    //virtual void _infoLight();
    //virtual void _infoLight(RgbColor color);
    uint8_t getColorChannelsCount();
    virtual String getModelId()=0;
    virtual String getType() = 0;
    virtual void lightEngine() = 0;
    void convertHue(LightData* lightData_p);
    void convertCt(LightData* lightData_p);
    void convertXy(LightData* lightData_p);
    virtual uint8_t calculateOptimalBrightness(uint8_t brightness);
    virtual float applyGammaCorrection(float val);
    void convertCtForRGBW(LightData* lightData_p);
    void convertCtForRGB(LightData* lightData_p);

    void applyScene(uint8_t new_scene);
    virtual void applyColorsImmediatly() = 0;
    virtual void saveStartupMode(uint8_t mode);
    virtual uint8_t getStartupMode();
    virtual bool isOnStartupRestoreLastState();
    virtual bool isOnStartupWhiteMaxBrightness();
    void handleLightStartup();
    void copyLightData(LightData* in, LightData* out);
    void raiseSaveStateRequest();
    void normalizeFloatRgb(float* r_ptrr, float* g_ptr, float* b_ptr);
    void setLightInFlashMode(LightData* lightData_p, uint8_t lampsCount, uint16_t frequencyMs);
    void setLightInNormalMode(LightData* lightData_p);
    void handleFlashMode(LightData* lightData_p);
    void handleAlert();
    void handleTurnOnOrOff(boolean isOn);
    virtual uint8_t correctBrightness(uint8_t brightness);
    virtual void processLightData(float transitionTime);
    virtual void setFirstTimeMode();
    virtual bool hasStartupModeBeenSet();
//		bool isLightOn(uint8_t lightIndex);
//		void setLightOn(uint8_t lightIndex);
//		void setLightOff(uint8_t lightIndex);
    bool isLightOn();
    void setLightOn();
    void setLightOff();
    void setLightState(bool isOn);

    virtual void printDebug();

    // Use 'DeviceConfig' class
//		IPAddress device_ip;
//		IPAddress gateway_ip;
//		IPAddress subnet_mask;
//		String* lightName;
//        uint8_t scene;
    DeviceConfig deviceConfig;
    LightData lightData;

    

    bool in_transition;
    bool entertainment_run;
    bool requireSavingNewState;
    int transitiontime;
    unsigned long latestStateSave;
    bool hasBeenNoConnectionNotifyFired;

    //bool isEpromDataCompliant();
    bool isFirstPowerUp();
    uint8_t checkRgbInterval(int val);
    uint8_t rgbMultiplier[CHANNELS_COUNT]; // light multiplier in percentage /R, G, B/

    

private:
    

};

#endif
