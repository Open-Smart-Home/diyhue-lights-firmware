

#ifndef LIGHT_H
#define LIGHT_H

//#define WS2812B_RGB_LED_STRIP

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <EEPROM.h>
#include <ArduinoJson.h>
#include <WiFiUdp.h>
#include "../BaseLight.h"
#include "../internal_cfg.h"

#include <NeoPixelBus.h>
#include <NeoPixelBrightnessBus.h>
#include <NeoPixelAnimator.h>


// Define base setup if not defined by user
#ifndef TOTAL_LED_COUNT
#define TOTAL_LED_COUNT 36
#endif
#ifndef LIGHTS_IN_STRIP
#define LIGHTS_IN_STRIP 3
#endif
#ifndef TRANSITION_LEDS
#define TRANSITION_LEDS 6
#endif

#define ENTERTAINMENT_TIMEOUT 1500 // millis


class Light : public BaseLight
{
public:
	// Override
	void init(String* lightName);
	void restoreLastState();
	void saveState();
	void notifyNotConnected();
	void srvOnSwitch(ESP8266WebServer* srvPtr);
	void srvOnSet(ESP8266WebServer* srvPtr);
	void srvOnGet(ESP8266WebServer* srvPtr);
	void srvOnDetect(ESP8266WebServer* srvPtr, byte* macPtr);
	void srvOnBaseReq(ESP8266WebServer* srvPtr);

	void srvOnStateGet(ESP8266WebServer* srvPtr);
	void srvOnStatePut(ESP8266WebServer* srvPtr);
	void srvOnConfigGet(ESP8266WebServer* srvPtr);
	void run(bool isConnected);

	// Extra
	void setUdpForEntertainment(WiFiUDP* wifiUdp_ptr);

protected:
	// Override
	void initPlatformSpecific();
	void setWhiteMaxBrightness();
	void lightEngine();
	String getModelId();
	String getType();
	void applyColorsImmediatly();
	void applyScene(uint8_t scene, uint8_t light);
	virtual uint8_t correctBrightness(uint8_t brightness);
	void handleTurnOnOrOff(uint8_t lightIndex, boolean isOn);
	void handleFlashMode(uint8_t lightIndex, LightData* lightData_p);
	void printDebug();


	// Extra
	void processLightData(uint8_t light, float transitiontime);
	RgbColor blending(float left[CHANNELS_COUNT], float right[CHANNELS_COUNT], uint8_t pixel);
	RgbColor convInt(uint8_t color[CHANNELS_COUNT]);
	RgbColor convFloat(float color[CHANNELS_COUNT]);
	/*void cache();
	void restoreFromCache();*/
	bool isLightOn(uint8_t light);
	void setLightOn(uint8_t light);
	void setLightOff(uint8_t light);
	void setLightState(uint8_t light, bool isOn);

	bool isColorModeXY(uint8_t light);
	bool isColorModeCT(uint8_t light);
	bool isColorModeHUE(uint8_t light);

	bool isAnyLightInFlashMode();
	void entertainment();
	RgbColor blendingEntert(float left[CHANNELS_COUNT], float right[CHANNELS_COUNT], float pixel);

	LightData lightsData[LIGHTS_IN_STRIP];

	bool isInTransition;
	bool entertainmentRun;
	unsigned long lastEPMillis;

	//uint16_t lightLedsCount;
	uint16_t ledsPerLight[LIGHTS_IN_STRIP + 1];
	//uint16_t dividedLightsArray[LIGHTS_IN_STRIP + 1]; // TODO: without the +1 there's a runtime error, check why

private:
	WiFiUDP* wifiUdp_ptr;
	byte packetBuffer[46];
	//NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod>* strip;

};




#endif
