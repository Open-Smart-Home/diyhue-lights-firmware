#ifndef DEVICE_H
#define DEVICE_H

//#if defined(ARDUINO) && ARDUINO >= 100
//#include "Arduino.h"
//#else
//#include "WProgram.h"
//#endif
//
//#include <ESP8266WebServer.h>
//#include <EEPROM.h>
//#include <ArduinoJson.h>
//#include "internal_cfg.h"
//#include "models/DeviceConfig.h"
//#include "modules/SaveRestoreDataModule.h"
//
//#define DEVICE_LIGHTS_COUNT 1
//
//class Device
//{
//public:
//    void init(String* lightName);
//    void setNetworkCfg(IPAddress deviceIp, IPAddress gatewayIp, IPAddress subnetMask);
//    virtual void restoreLastState();
//    virtual void saveState();
//    virtual void notifyNotConnected();
//    void run(bool isConnected);
//
//
//    virtual void srvOnBaseReq(ESP8266WebServer* srvPtr);
//    virtual void srvOnSwitch(ESP8266WebServer* srvPtr);
//    virtual void srvOnSet(ESP8266WebServer* srvPtr);
//    virtual void srvOnGet(ESP8266WebServer* srvPtr);
//    virtual void srvOnDetect(ESP8266WebServer* srvPtr, byte* macPtr);
//
//    virtual void srvOnStateGet(ESP8266WebServer* srvPtr);
//    virtual void srvOnStatePut(ESP8266WebServer* srvPtr);
//    virtual void srvOnConfigGet(ESP8266WebServer* srvPtr);
//
//protected:
//	DeviceConfig deviceConfig;
//	LightData lightsDataList[DEVICE_LIGHTS_COUNT];
//
//private:
//
//};
#endif

