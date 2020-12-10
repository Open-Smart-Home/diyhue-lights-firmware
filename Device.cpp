#include "Device.h"

//void Device::init(String* lightName)
//{
//	//this->lightName = lightName;
//	deviceConfig.setLightName(lightName);
//
//	for (uint8_t i = 0; i < DEVICE_LIGHTS_COUNT) {
//		this->lightsDataList[i] = Light();
//	}
//
//	this->lightData.mode = LightMode::Normal;
//	this->lightData.toDoFlashesLeft = 0;
//	this->lightData.lastFlash = 0;
//	this->lightData.flashFreqMs = 0;
//
//	// initialize HW (eg. led driver, etc)
//	this->initPlatformSpecific();
//
//	// if it's the first startup then initialize data structure and turn lamp on to bright white
//	if (this->isFirstPowerUp() == true) {
//		Serial.println("[BaseLight::init] First lamp startup, initialize data structure");
//		saveStartupMode(STARTUP_MODE_POWERFAIL); // aka: restore last state
//		this->setFirstTimeMode();
//		saveState();
//	}
//
//	// the perform common operations
//	this->handleLightStartup();
//}
//
//void Device::setNetworkCfg(IPAddress deviceIp, IPAddress gatewayIp, IPAddress subnetMask)
//{
//	/*this->device_ip = deviceIp;
//	this->gateway_ip = gatewayIp;
//	this->subnet_mask = subnetMask;*/
//	this->deviceConfig.saveNetworkData(&deviceIp, &gatewayIp, &subnetMask);
//}