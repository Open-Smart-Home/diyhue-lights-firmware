#include "Ws2812bRgbLedStrip.h"

#ifdef WS2812B_RGB_LED_STRIP

//byte packetBuffer[64];
//long lastEPMillis;



NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(TOTAL_LED_COUNT);
RgbColor red = RgbColor(255, 0, 0);
RgbColor green = RgbColor(0, 255, 0);
RgbColor blue = RgbColor(0, 0, 255);
RgbColor white = RgbColor(255);
RgbColor black = RgbColor(0);


void Light::init(String* lightName)
{
	//this->lightName = lightName;
	deviceConfig.setLightName(lightName);

	this->rgbMultiplier[COLOR_INDEX_RED] = 100;
	this->rgbMultiplier[COLOR_INDEX_GREEN] = 100;
	this->rgbMultiplier[COLOR_INDEX_BLUE] = 100;

	hasBeenNoConnectionNotifyFired = false;
	for (uint8_t i = 0; i < LIGHTS_IN_STRIP; i++) {
		BaseLight::initLightData( &(this->lightsData[i]) );
	}
	

	// initialize HW (eg. led driver, etc)
	this->initPlatformSpecific();

	// TEST STRIP
	/*strip.ClearTo(red); strip.Show(); delay(1000);
	strip.ClearTo(green); strip.Show(); delay(1000);
	strip.ClearTo(blue); strip.Show(); delay(1000);
	strip.ClearTo(black); strip.Show();*/

	// if it's the first startup then initialize data structure and turn lamp on to bright white
	if (this->isFirstPowerUp() == true) {
		SerialPrintln("[BaseLight::init] First lamp startup, initialize data structure");
		saveStartupMode(STARTUP_MODE_POWERFAIL); // aka: restore last state
		this->setFirstTimeMode();
		saveState();
	}

	// the perform common operations
	this->handleLightStartup();
}


void Light::saveState() {
	SaveRestoreDataModule::saveAllLightsData( this->lightsData );
}


void Light::restoreLastState() {
	if (SaveRestoreDataModule::restoreAllLightsData(this->lightsData)) {

		for (uint8_t i = 0; i < DEVICE_LIGHTS_COUNT; i++)
		{
			if (this->lightsData[i].colorMode == COLOR_MODE_XY && this->isLightOn(i)) {
				convertXy(&(lightsData[i]));
			}
			else if (this->lightsData[i].colorMode == COLOR_MODE_CT && this->isLightOn(i)) {
				convertCt(&(lightsData[i]));
			}
			else if (this->lightsData[i].colorMode == COLOR_MODE_HUE && this->isLightOn(i)) {
				convertHue(&(lightsData[i]));
			}
		}

		applyColorsImmediatly();
	}
}


void Light::initPlatformSpecific()
{
	uint16_t ledsPerLightCalculated = (uint16_t)ceil(TOTAL_LED_COUNT / LIGHTS_IN_STRIP);
	uint16_t unusedLeds = TOTAL_LED_COUNT;

	// fill the 'ledsPerLight' with the pixel count for each light
	//this->ledsPerLight[LIGHTS_IN_STRIP];
	for (uint8_t i = 0; i < LIGHTS_IN_STRIP; i++) {
		if(unusedLeds >= ledsPerLightCalculated){
			this->ledsPerLight[i] = ledsPerLightCalculated;
		}
		else if (unusedLeds > TRANSITION_LEDS){
			this->ledsPerLight[i] = unusedLeds;
		}
		else {
			SerialPrint("[Light::initPlatformSpecific] Light "); SerialPrintWithFormat(i, DEC);
			SerialPrint(" cannot be set, no LEDs left"); SerialPrintWithFormat(this->ledsPerLight[i], DEC);
			break;
		}
		unusedLeds -= this->ledsPerLight[i];
	}

	// DEBUG
	for (uint8_t i = 0; i < LIGHTS_IN_STRIP; i++) {
		SerialPrint("[Light::initPlatformSpecific] Light "); SerialPrintWithFormat(i, DEC);
		SerialPrint(" --> LED used: "); SerialPrintWithFormat(this->ledsPerLight[i], DEC);
		SerialPrintln();
	}

	SerialPrint("Free HEAP: "); SerialPrintln(ESP.getFreeHeap());

    strip.Begin();
	strip.ClearTo(black);
    strip.Show();

	if (this->wifiUdp_ptr != NULL) {
		this->wifiUdp_ptr->begin(2100);
	}
	else {
		SerialPrintln("[Light::initPlatformSpecific] ERROR, WIFI UDP POINTER IS NULL");
	}
}

void Light::setUdpForEntertainment(WiFiUDP* wifiUdp_ptr) {
	this->wifiUdp_ptr = wifiUdp_ptr;
}


String Light::getModelId()
{
	return LIGHT_MODEL_RGB_STRIP_PLUS;
}

String Light::getType() {
	return "ws2812_strip";
}


bool Light::isLightOn(uint8_t light){ return lightsData[light].lightState == true; }
void Light::setLightOn(uint8_t light){ setLightState(light, true); }
void Light::setLightOff(uint8_t light){ setLightState(light, false); }
void Light::setLightState(uint8_t light, bool isOn){ lightsData[light].lightState = isOn; }

bool Light::isColorModeXY(uint8_t light){ return lightsData[light].colorMode == COLOR_MODE_XY; }
bool Light::isColorModeCT(uint8_t light){return lightsData[light].colorMode == COLOR_MODE_CT;}
bool Light::isColorModeHUE(uint8_t light){return lightsData[light].colorMode == COLOR_MODE_HUE;}


// Set the light to a white (warm or cold) at full brightnes.
// This is may change according with light hardware
void Light::setWhiteMaxBrightness()
{
	for (uint8_t i = 0; i < LIGHTS_IN_STRIP; i++) {
		setLightOn(i);
		lightsData[i].bri = MAX_BRIGHTNESS;

#ifdef HAS_COLOR_MODE_CT
		lightData.colorMode = COLOR_MODE_CT;
		lightData.ct = COLOR_TEMP_NEUTRAL + 20; // neutral/slight warm white
		convertCt(&lightData);
#endif
#ifndef HAS_COLOR_MODE_CT
#ifdef HAS_COLOR_MODE_XY
		lightsData[i].colorMode = COLOR_MODE_XY;
		// set xy color value to slight warm white
		lightsData[i].x = 0.47f; //? ? ? check
		lightsData[i].y = 0.45f; //? ? ? check
		convertXy( &(lightsData[i]) );
#endif
#endif // !HAS_COLOR_MODE_CT
	}

	applyColorsImmediatly();
}



uint8_t Light::correctBrightness(uint8_t brightness)
{
	if (brightness < 5) {
		brightness = 5;
	}
	return brightness;
}

void Light::applyScene(uint8_t new_scene, uint8_t light) 
{
	if (new_scene == 0) {
		lightsData[light].bri = 144; lightsData[light].ct = 447; lightsData[light].colorMode = 2; convertCt( &(lightsData[light]));
	}
	else if (new_scene == 1) {
		lightsData[light].bri = 254; lightsData[light].ct = 346; lightsData[light].colorMode = 2; convertCt(&(lightsData[light]));
	}
	else if (new_scene == 2) {
		lightsData[light].bri = 254; lightsData[light].ct = 233; lightsData[light].colorMode = 2; convertCt(&(lightsData[light]));
	}
	else if (new_scene == 3) {
		lightsData[light].bri = 254; lightsData[light].ct = 156; lightsData[light].colorMode = 2; convertCt(&(lightsData[light]));
	}
	else if (new_scene == 4) {
		lightsData[light].bri = 77; lightsData[light].ct = 367; lightsData[light].colorMode = 2; convertCt(&(lightsData[light]));
	}
	else if (new_scene == 5) {
		lightsData[light].bri = 254; lightsData[light].ct = 447; lightsData[light].colorMode = 2; convertCt(&(lightsData[light]));
	}
	else if (new_scene == 6) {
		lightsData[light].bri = 1; lightsData[light].x = 0.561; lightsData[light].y = 0.4042; lightsData[light].colorMode = 1; convertXy( &(lightsData[light]));
	}
	else if (new_scene == 7) {
		lightsData[light].bri = 203; lightsData[light].x = 0.380328; lightsData[light].y = 0.39986; lightsData[light].colorMode = 1; convertXy( &(lightsData[light]));
	}
	else if (new_scene == 8) {
		lightsData[light].bri = 112; lightsData[light].x = 0.359168; lightsData[light].y = 0.28807; lightsData[light].colorMode = 1; convertXy( &(lightsData[light]));
	}
	else if (new_scene == 9) {
		lightsData[light].bri = 142; lightsData[light].x = 0.267102; lightsData[light].y = 0.23755; lightsData[light].colorMode = 1; convertXy( &(lightsData[light]));
	}
	else if (new_scene == 10) {
		lightsData[light].bri = 216; lightsData[light].x = 0.393209; lightsData[light].y = 0.29961; lightsData[light].colorMode = 1; convertXy( &(lightsData[light]));
	}
}

void  Light::processLightData(uint8_t light, float transitiontime) 
{
	transitiontime *= 17 - (TOTAL_LED_COUNT / 40); //every extra led add a small delay that need to be counted
	if (isColorModeXY(light) && isLightOn(light)) {
		convertXy( &(lightsData[light]));
	}
	else if (isColorModeCT(light) && isLightOn(light) ) {
		convertCt( &(lightsData[light]) );
	}
	else if (isColorModeHUE(light) && isLightOn(light) ) {
		convertHue(&(lightsData[light]));
	}
	for (uint8_t color = 0; color < CHANNELS_COUNT; color++)
	{
		if ( isLightOn(light) ) {
			lightsData[light].stepLevel[color] = ((float)lightsData[light].colors[color] - lightsData[light].currentColors[color]) / transitiontime;
		}
		else {
			lightsData[light].stepLevel[color] = lightsData[light].currentColors[color] / transitiontime;
		}
	}
}


RgbColor Light::blending(float left[CHANNELS_COUNT], float right[CHANNELS_COUNT], uint8_t pixel)
{
	uint8_t result[CHANNELS_COUNT];
	for (uint8_t i = 0; i < CHANNELS_COUNT; i++) {
		float percent = 0;
		if (TRANSITION_LEDS > 0) {
			float percent = (float)pixel / (float)(TRANSITION_LEDS + 1);
		}
		result[i] = (left[i] * (1.0f - percent) + right[i] * percent) / 2;
	}
	return RgbColor(result[COLOR_INDEX_RED], result[COLOR_INDEX_GREEN], result[COLOR_INDEX_BLUE]);
}


RgbColor Light::convInt(uint8_t color[CHANNELS_COUNT]){
	return RgbColor((uint8_t)color[0], (uint8_t)color[1], (uint8_t)color[2]);
}

RgbColor Light::convFloat(float color[3]) {
	return RgbColor((int)color[0], (int)color[1], (int)color[2]);
}

//void Light::cache() 
//{
//	for (int light = 0; light < LIGHTS_IN_STRIP; light++) 
//	{
//		_light_state[light] = light_state[light];
//		for (int component = 0; component < 3; component++) 
//		{
//			_rgb[light][component] = rgb[light][component];
//			_current_rgb[light][component] = current_rgb[light][component];
//		}
//		_color_mode[light] = color_mode[light];
//		_x[light] = x[light];
//		_y[light] = y[light];
//		_bri[light] = bri[light];
//		_ct[light] = ct[light];
//		_sat[light] = sat[light];
//		_hue[light] = hue[light];
//	}
//}

//void Light::restoreFromCache() 
//{
//	for (int light = 0; light < LIGHTS_IN_STRIP; light++) 
//	{
//		light_state[light] = _light_state[light];
//		for (int component = 0; component < 3; component++) {
//			current_rgb[light][component] = rgb[light][component];
//			rgb[light][component] = _rgb[light][component];
//		}
//		color_mode[light] = _color_mode[light];
//		x[light] = _x[light];
//		y[light] = _y[light];
//		bri[light] = _bri[light];
//		ct[light] = _ct[light];
//		sat[light] = _sat[light];
//		hue[light] = _hue[light];
//		processLightData(light, 4);
//	}
//}


void Light::lightEngine() 
{
	for (int light = 0; light < LIGHTS_IN_STRIP; light++) 
	{
		if (isLightOn(light) )
		{
			// The light 'N-th' is ON
			if (lightsData[light].colors[COLOR_INDEX_RED] != lightsData[light].currentColors[COLOR_INDEX_RED]
				|| lightsData[light].colors[COLOR_INDEX_GREEN] != lightsData[light].currentColors[COLOR_INDEX_GREEN]
				|| lightsData[light].colors[COLOR_INDEX_BLUE] != lightsData[light].currentColors[COLOR_INDEX_BLUE])
			{
				// any of the R, G or B value is still different from the target value
				in_transition = true;

				for (uint8_t k = 0; k < 3; k++) {
					if (lightsData[light].colors[k] != lightsData[light].currentColors[k]) 
						lightsData[light].currentColors[k] += lightsData[light].stepLevel[k];
					
					if ((lightsData[light].stepLevel[k] > 0.0 && lightsData[light].currentColors[k] > lightsData[light].colors[k]) || (lightsData[light].stepLevel[k] < 0.0 && lightsData[light].currentColors[k] < lightsData[light].colors[k]))
						lightsData[light].currentColors[k] = lightsData[light].colors[k];
				}
				if (LIGHTS_IN_STRIP > 1) 
				{
					if (light == 0) {
						for (int pixel = 0; pixel < ledsPerLight[0]; pixel++)
						{
							if (pixel < ledsPerLight[0] - TRANSITION_LEDS / 2) {
								strip.SetPixelColor(pixel, convFloat(lightsData[light].currentColors));
							}
							else {
								strip.SetPixelColor(
									pixel, 
									blending(lightsData[0].currentColors, 
										lightsData[1].currentColors, 
										pixel + 1 - (ledsPerLight[0] - TRANSITION_LEDS / 2)
									)
								);
							}
						}
					}
					else {
						for (int pixel = 0; pixel < ledsPerLight[light]; pixel++)
						{
							long pixelSum;
							for (int value = 0; value < light; value++)
							{
								if (value + 1 == light) {
									pixelSum += ledsPerLight[value] - TRANSITION_LEDS;
								}
								else {
									pixelSum += ledsPerLight[value];
								}
							}

							if (pixel < TRANSITION_LEDS / 2) {
								strip.SetPixelColor(pixel + pixelSum + TRANSITION_LEDS, blending(lightsData[light - 1].currentColors, lightsData[light].currentColors, pixel + 1));
							}
							else if (pixel > ledsPerLight[light] - TRANSITION_LEDS / 2 - 1) {
								//SerialPrintln(String(pixel));
								strip.SetPixelColor(pixel + pixelSum + TRANSITION_LEDS, blending(lightsData[light].currentColors, lightsData[light + 1].currentColors, pixel + TRANSITION_LEDS / 2 - ledsPerLight[light]));
							}
							else {
								strip.SetPixelColor(pixel + pixelSum + TRANSITION_LEDS, convFloat(lightsData[light].currentColors));
							}
							pixelSum = 0;
						}
					}
				}
				else {
					strip.ClearTo(convFloat(lightsData[light].currentColors), 0, TOTAL_LED_COUNT - 1);
				}

				/*for (int pixel = 0; pixel < TOTAL_LED_COUNT; pixel++) {
					Serial.print("[Light::lightEngine] ");
					Serial.print("Pixel ["); Serial.print(pixel, DEC); Serial.print("] color ");
					Serial.print(strip.GetPixelColor(pixel).R, DEC); Serial.print(",");
					Serial.print(strip.GetPixelColor(pixel).G, DEC); Serial.print(",");
					Serial.print(strip.GetPixelColor(pixel).B, DEC);
					Serial.println();

				}*/

				strip.Show();
			}
		} // end of 'the N-th light is ON'
		else 
		{
			// The light 'N-th' is OFF
			if (lightsData[light].currentColors[0] != 0 || lightsData[light].currentColors[1] != 0 || lightsData[light].currentColors[2] != 0) {
				in_transition = true;
				for (uint8_t k = 0; k < 3; k++) {
					if (lightsData[light].currentColors[k] != 0) lightsData[light].currentColors[k] -= lightsData[light].stepLevel[k];
					if (lightsData[light].currentColors[k] < 0) lightsData[light].currentColors[k] = 0;
				}

				if (LIGHTS_IN_STRIP > 1) 
				{
					if (light == 0) {
						for (int pixel = 0; pixel < ledsPerLight[0]; pixel++)
						{
							if (pixel < ledsPerLight[0] - TRANSITION_LEDS / 2) {
								strip.SetPixelColor(pixel, convFloat(lightsData[light].currentColors));
							}
							else {
								strip.SetPixelColor(pixel, blending(lightsData[0].currentColors, lightsData[1].currentColors, pixel + 1 - (ledsPerLight[0] - TRANSITION_LEDS / 2)));
							}
						}
					}
					else {
						for (int pixel = 0; pixel < ledsPerLight[light]; pixel++)
						{
							long pixelSum;
							for (int value = 0; value < light; value++)
							{
								if (value + 1 == light) {
									pixelSum += ledsPerLight[value] - TRANSITION_LEDS;
								}
								else {
									pixelSum += ledsPerLight[value];
								}
							}

							if (pixel < TRANSITION_LEDS / 2) {
								strip.SetPixelColor(pixel + pixelSum + TRANSITION_LEDS, blending(lightsData[light - 1].currentColors, lightsData[light].currentColors, pixel + 1));
							}
							else if (pixel > ledsPerLight[light] - TRANSITION_LEDS / 2 - 1) {
								//Serial.println(String(pixel));
								strip.SetPixelColor(pixel + pixelSum + TRANSITION_LEDS, blending(lightsData[light].currentColors, lightsData[light + 1].currentColors, pixel + TRANSITION_LEDS / 2 - ledsPerLight[light]));
							}
							else {
								strip.SetPixelColor(pixel + pixelSum + TRANSITION_LEDS, convFloat(lightsData[light].currentColors));
							}
							pixelSum = 0;
						}
					}
				}
				else {
					strip.ClearTo(convFloat(lightsData[light].currentColors), 0, TOTAL_LED_COUNT - 1);
				}

				/*for (int pixel = 0; pixel < TOTAL_LED_COUNT; pixel++) {
					Serial.print("[Light::lightEngine] ");
					Serial.print("Pixel ["); Serial.print(pixel, DEC); Serial.print("] color ");
					Serial.print(strip.GetPixelColor(pixel).R, DEC); Serial.print(",");
					Serial.print(strip.GetPixelColor(pixel).G, DEC); Serial.print(",");
					Serial.print(strip.GetPixelColor(pixel).B, DEC);
					Serial.println();

				}*/

				strip.Show();
			}
		}
	}

	// when calculus for every light is completed
	// then if is in transition wait a little bit
	if (in_transition) 
	{
		delay(6);
		in_transition = false;
	}
	// HARDWARE SWTCHES IF ANY?
}



void Light::notifyNotConnected()
{

}

void Light::applyColorsImmediatly()
{
	for (uint8_t lightIndex = 0; lightIndex < LIGHTS_IN_STRIP; lightIndex++) {

		// If the lamp is ON
		if (this->isLightOn(lightIndex))
		{			
			for (uint8_t color = 0; color < CHANNELS_COUNT; color++)
			{
				/*Serial.print("[Light::applyColorsImmediatly] ");
				Serial.print("Set channel ["); Serial.print(color,DEC); Serial.print("] to "); Serial.print(this->lightsData[lightIndex].colors[color], DEC);
				Serial.println();*/
				this->lightsData[lightIndex].currentColors[color] = this->lightsData[lightIndex].colors[color];
			}
			
			
			if (LIGHTS_IN_STRIP > 1)
			{
				if (lightIndex == 0) {
					for (int pixel = 0; pixel < ledsPerLight[0]; pixel++)
					{
						if (pixel < ledsPerLight[0] - TRANSITION_LEDS / 2) {
							strip.SetPixelColor(pixel, convFloat(lightsData[lightIndex].currentColors));
						}
						else {
							strip.SetPixelColor(
								pixel,
								blending(lightsData[0].currentColors,
									lightsData[1].currentColors,
									pixel + 1 - (ledsPerLight[0] - TRANSITION_LEDS / 2)
								)
							);
						}
					}
				}
				else {
					for (int pixel = 0; pixel < ledsPerLight[lightIndex]; pixel++)
					{
						long pixelSum;
						for (int value = 0; value < lightIndex; value++)
						{
							if (value + 1 == lightIndex) {
								pixelSum += ledsPerLight[value] - TRANSITION_LEDS;
							}
							else {
								pixelSum += ledsPerLight[value];
							}
						}

						if (pixel < TRANSITION_LEDS / 2) {
							strip.SetPixelColor(pixel + pixelSum + TRANSITION_LEDS, blending(lightsData[lightIndex - 1].currentColors, lightsData[lightIndex].currentColors, pixel + 1));
						}
						else if (pixel > ledsPerLight[lightIndex] - TRANSITION_LEDS / 2 - 1) {
							//Serial.println(String(pixel));
							strip.SetPixelColor(pixel + pixelSum + TRANSITION_LEDS, blending(lightsData[lightIndex].currentColors, lightsData[lightIndex + 1].currentColors, pixel + TRANSITION_LEDS / 2 - ledsPerLight[lightIndex]));
						}
						else {
							strip.SetPixelColor(pixel + pixelSum + TRANSITION_LEDS, convFloat(lightsData[lightIndex].currentColors));
						}
						pixelSum = 0;
					}
				}
			}
			else {
				strip.ClearTo(convFloat(lightsData[lightIndex].currentColors), 0, TOTAL_LED_COUNT - 1);
			}

			/*for (int pixel = 0; pixel < TOTAL_LED_COUNT; pixel++) {
				Serial.print("[Light::applyColorsImmediatly] ");
				Serial.print("Pixel ["); Serial.print(pixel, DEC); Serial.print("] color "); 
				Serial.print(strip.GetPixelColor(pixel).R, DEC); Serial.print(",");
				Serial.print(strip.GetPixelColor(pixel).G, DEC); Serial.print(",");
				Serial.print(strip.GetPixelColor(pixel).B, DEC);
				Serial.println();
				
			}*/

			strip.Show();
		}
	}
}




void Light::handleTurnOnOrOff(uint8_t lightIndex, boolean isOn)
{
	if (isOn) {
		setLightOn(lightIndex);
	}
	else {
		setLightOff(lightIndex);
	}
	SaveRestoreDataModule::saveLightData( &(this->lightsData[lightIndex]), lightIndex);
}


void Light::handleFlashMode(uint8_t lightIndex, LightData* lightData_p) {
	bool isFirstFlash = lightData_p->lastFlash == 0;
	if (isFirstFlash) {
		// before lamp start flashing for the first time
		// save lamp state so it can be restored when flashing ends
		saveState();
	}

	// if there're still to do flashes left (or if finished but light is still ON from the last flash)
	if (lightData_p->toDoFlashesLeft > 0 || this->isLightOn(lightIndex) == true) {
		// check last execution 
		// NOTE: lamp flash is obtained turn it ON and OFF. So if lamp frequency is 1Hz the code below
		// will be execute 2 times per seconds (2Hs): one for turning ON and one for turn OFF

		uint16_t lampFreqDividedMs = lightData_p->flashFreqMs / 2; // eg. 1 sec --> lamp freq is 2 seconds
		if ((millis() - lightData_p->lastFlash) > lampFreqDividedMs) {

			// turn the lamp ON if it is OFF or this is the first flash
			if (this->isLightOn(lightIndex) == false || isFirstFlash == true) {
				// lamp is OFF --> turn ON and decrement the flashes left
				lightData_p->toDoFlashesLeft--;
				SerialPrint("Turn light ON (flashes left:"); SerialPrintWithFormat(lightData_p->toDoFlashesLeft, DEC); SerialPrintln(")");

#if defined(LIGHT_TYPE_CCT) || defined(LIGHT_TYPE_RGBCCT)
				lightData_p->colors[COLOR_INDEX_COLD_WHITE] = FLASH_INTENSITY;
#endif
#if defined(LIGHT_TYPE_RGB)
				lightData_p->colors[COLOR_INDEX_RED] = FLASH_INTENSITY;
				lightData_p->colors[COLOR_INDEX_GREEN] = FLASH_INTENSITY;
				lightData_p->colors[COLOR_INDEX_BLUE] = FLASH_INTENSITY;
#endif
#if defined(LIGHT_TYPE_RGBW)
				lightData_p->colors[COLOR_INDEX_WHITE] = FLASH_INTENSITY;
#endif
				setLightOn(lightIndex);
				applyColorsImmediatly();

			}
			else {
				// lamp is ON --> turn it OFF
				SerialPrintln("Turn light OFF");
				for (uint8_t i = 0; i < CHANNELS_COUNT; i++)
				{
					lightData_p->colors[i] = 0;
				}

				applyColorsImmediatly();
				setLightOff(lightIndex);
			}

			lightData_p->lastFlash = millis();
		}
	}
	else {
		// light has finished the number of flash set before, return to normal mode
		SerialPrintln("Lamp ends flashing, restore previous state");
		this->setLightInNormalMode(lightData_p);
		restoreLastState();
	}
}


void Light::srvOnBaseReq(ESP8266WebServer* srvPtr) {
	//if (server.arg("section").toInt() == 1) {
	//	server.arg("name").toCharArray(lightName, LIGHT_NAME_MAX_LENGTH);
	//	startup = server.arg("startup").toInt();
	//	scene = server.arg("scene").toInt();
	//	lightsCount = server.arg("lightscount").toInt();
	//	pixelCount = server.arg("pixelcount").toInt();
	//	transitionLeds = server.arg("transitionleds").toInt();
	//	rgb_multiplier[0] = server.arg("rpct").toInt();
	//	rgb_multiplier[1] = server.arg("gpct").toInt();
	//	rgb_multiplier[2] = server.arg("bpct").toInt();
	//	for (uint16_t i = 0; i < lightsCount; i++) {
	//		ledsPerLight[i] = server.arg("dividedLight_" + String(i)).toInt();
	//	}
	//	hwSwitch = server.hasArg("hwswitch") ? server.arg("hwswitch").toInt() : 0;
	//	if (server.hasArg("hwswitch")) {
	//		onPin = server.arg("on").toInt();
	//		offPin = server.arg("off").toInt();
	//	}
	//	//saveConfig();
	//}
	//else if (server.arg("section").toInt() == 2) {
	//	useDhcp = (!server.hasArg("disdhcp")) ? 1 : server.arg("disdhcp").toInt();
	//	if (server.hasArg("disdhcp")) {
	//		address.fromString(server.arg("addr"));
	//		gateway.fromString(server.arg("gw"));
	//		submask.fromString(server.arg("sm"));
	//	}
	//	//saveConfig();
	//}

	String htmlContent = "<!DOCTYPE html> <html> <head> <meta charset=\"UTF-8\"> <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"> <title>" + String(this->deviceConfig.getLightName()) + " - DiyHue</title> <link rel=\"icon\" type=\"image/png\" href=\"https://diyhue.org/wp-content/uploads/2019/11/cropped-Zeichenfl%C3%A4che-4-1-32x32.png\" sizes=\"32x32\"> <link href=\"https://fonts.googleapis.com/icon?family=Material+Icons\" rel=\"stylesheet\"> <link rel=\"stylesheet\" href=\"https://cdnjs.cloudflare.com/ajax/libs/materialize/1.0.0/css/materialize.min.css\"> <link rel=\"stylesheet\" href=\"https://diyhue.org/cdn/nouislider.css\" /> </head> <body> <div class=\"wrapper\"> <nav class=\"nav-extended row\" style=\"background-color: #26a69a !important;\"> <div class=\"nav-wrapper col s12\"> <a href=\"#\" class=\"brand-logo\">DiyHue</a> <ul id=\"nav-mobile\" class=\"right hide-on-med-and-down\" style=\"position: relative;z-index: 10;\"> <li><a target=\"_blank\" href=\"https://github.com/diyhue\"><i class=\"material-icons left\">language</i>GitHub</a></li> <li><a target=\"_blank\" href=\"https://diyhue.readthedocs.io/en/latest/\"><i class=\"material-icons left\">description</i>Documentation</a></li> <li><a target=\"_blank\" href=\"https://diyhue.slack.com/\"><i class=\"material-icons left\">question_answer</i>Slack channel</a></li> </ul> </div> <div class=\"nav-content\"> <ul class=\"tabs tabs-transparent\"> <li class=\"tab\" title=\"#home\"><a class=\"active\" href=\"#home\">Home</a></li> <li class=\"tab\" title=\"#preferences\"><a href=\"#preferences\">Preferences</a></li> <li class=\"tab\" title=\"#network\"><a href=\"#network\">Network settings</a></li> <li class=\"tab\" title=\"/update\"><a href=\"/update\">Updater</a></li> </ul> </div> </nav> <ul class=\"sidenav\" id=\"mobile-demo\"> <li><a target=\"_blank\" href=\"https://github.com/diyhue\">GitHub</a></li> <li><a target=\"_blank\" href=\"https://diyhue.readthedocs.io/en/latest/\">Documentation</a></li> <li><a target=\"_blank\" href=\"https://diyhue.slack.com/\">Slack channel</a></li> </ul> <div class=\"container\"> <div class=\"section\"> <div id=\"home\" class=\"col s12\"> <form> <input type=\"hidden\" name=\"section\" value=\"1\"> <div class=\"row\"> <div class=\"col s10\"> <label for=\"power\">Power</label> <div id=\"power\" class=\"switch section\"> <label> Off <input type=\"checkbox\" name=\"pow\" id=\"pow\" value=\"1\"> <span class=\"lever\"></span> On </label> </div> </div> </div> <div class=\"row\"> <div class=\"col s12 m10\"> <label for=\"bri\">Brightness</label> <input type=\"text\" id=\"bri\" class=\"js-range-slider\" name=\"bri\" value=\"\" /> </div> </div> <div class=\"row\"> <div class=\"col s12\"> <label for=\"hue\">Color</label> <div> <canvas id=\"hue\" width=\"320px\" height=\"320px\" style=\"border:1px solid #d3d3d3;\"></canvas> </div> </div> </div> <div class=\"row\"> <div class=\"col s12\"> <label for=\"ct\">Color Temp</label> <div> <canvas id=\"ct\" width=\"320px\" height=\"50px\" style=\"border:1px solid #d3d3d3;\"></canvas> </div> </div> </div> </form> </div> <div id=\"preferences\" class=\"col s12\"> <form method=\"POST\" action=\"/\"> <input type=\"hidden\" name=\"section\" value=\"1\"> <div class=\"row\"> <div class=\"col s12\"> <label for=\"name\">Light Name</label> <input type=\"text\" id=\"name\" name=\"name\"> </div> </div> <div class=\"row\"> <div class=\"col s12 m6\"> <label for=\"startup\">Default Power:</label> <select name=\"startup\" id=\"startup\"> <option value=\"0\">Last State</option> <option value=\"1\">On</option> <option value=\"2\">Off</option> </select> </div> </div> <div class=\"row\"> <div class=\"col s12 m6\"> <label for=\"scene\">Default Scene:</label> <select name=\"scene\" id=\"scene\"> <option value=\"0\">Relax</option> <option value=\"1\">Read</option> <option value=\"2\">Concentrate</option> <option value=\"3\">Energize</option> <option value=\"4\">Bright</option> <option value=\"5\">Dimmed</option> <option value=\"6\">Nightlight</option> <option value=\"7\">Savanna sunset</option> <option value=\"8\">Tropical twilight</option> <option value=\"9\">Arctic aurora</option> <option value=\"10\">Spring blossom</option> </select> </div> </div> <div class=\"row\"> <div class=\"col s4 m3\"> <label for=\"pixelcount\" class=\"col-form-label\">Pixel count</label> <input type=\"number\" id=\"pixelcount\" name=\"pixelcount\"> </div> </div> <div class=\"row\"> <div class=\"col s4 m3\"> <label for=\"lightscount\" class=\"col-form-label\">Lights count</label> <input type=\"number\" id=\"lightscount\" name=\"lightscount\"> </div> </div> <label class=\"form-label\">Light division</label> </br> <label>Available Pixels:</label> <label class=\"availablepixels\"><b>null</b></label> <div class=\"row dividedLights\"> </div> <div class=\"row\"> <div class=\"col s4 m3\"> <label for=\"transitionleds\">Transition leds:</label> <select name=\"transitionleds\" id=\"transitionleds\"> <option value=\"0\">0</option> <option value=\"2\">2</option> <option value=\"4\">4</option> <option value=\"6\">6</option> <option value=\"8\">8</option> <option value=\"10\">10</option> </select> </div> </div> <div class=\"row\"> <div class=\"col s4 m3\"> <label for=\"rpct\" class=\"form-label\">Red multiplier</label> <input type=\"number\" id=\"rpct\" class=\"js-range-slider\" data-skin=\"round\" name=\"rpct\" value=\"\" /> </div> <div class=\"col s4 m3\"> <label for=\"gpct\" class=\"form-label\">Green multiplier</label> <input type=\"number\" id=\"gpct\" class=\"js-range-slider\" data-skin=\"round\" name=\"gpct\" value=\"\" /> </div> <div class=\"col s4 m3\"> <label for=\"bpct\" class=\"form-label\">Blue multiplier</label> <input type=\"number\" id=\"bpct\" class=\"js-range-slider\" data-skin=\"round\" name=\"bpct\" value=\"\" /> </div> </div> <div class=\"row\"> <label class=\"control-label col s10\">HW buttons:</label> <div class=\"col s10\"> <div class=\"switch section\"> <label> Disable <input type=\"checkbox\" name=\"hwswitch\" id=\"hwswitch\" value=\"1\"> <span class=\"lever\"></span> Enable </label> </div> </div> </div> <div class=\"switchable\"> <div class=\"row\"> <div class=\"col s4 m3\"> <label for=\"on\">On Pin</label> <input type=\"number\" id=\"on\" name=\"on\"> </div> <div class=\"col s4 m3\"> <label for=\"off\">Off Pin</label> <input type=\"number\" id=\"off\" name=\"off\"> </div> </div> </div> <div class=\"row\"> <div class=\"col s10\"> <button type=\"submit\" class=\"waves-effect waves-light btn teal\">Save</button> <!--<button type=\"submit\" name=\"reboot\" class=\"waves-effect waves-light btn grey lighten-1\">Reboot</button>--> </div> </div> </form> </div> <div id=\"network\" class=\"col s12\"> <form method=\"POST\" action=\"/\"> <input type=\"hidden\" name=\"section\" value=\"2\"> <div class=\"row\"> <div class=\"col s12\"> <label class=\"control-label\">Manual IP assignment:</label> <div class=\"switch section\"> <label> Disable <input type=\"checkbox\" name=\"disdhcp\" id=\"disdhcp\" value=\"0\"> <span class=\"lever\"></span> Enable </label> </div> </div> </div> <div class=\"switchable\"> <div class=\"row\"> <div class=\"col s12 m3\"> <label for=\"addr\">Ip</label> <input type=\"text\" id=\"addr\" name=\"addr\"> </div> <div class=\"col s12 m3\"> <label for=\"sm\">Submask</label> <input type=\"text\" id=\"sm\" name=\"sm\"> </div> <div class=\"col s12 m3\"> <label for=\"gw\">Gateway</label> <input type=\"text\" id=\"gw\" name=\"gw\"> </div> </div> </div> <div class=\"row\"> <div class=\"col s10\"> <button type=\"submit\" class=\"waves-effect waves-light btn teal\">Save</button> <!--<button type=\"submit\" name=\"reboot\" class=\"waves-effect waves-light btn grey lighten-1\">Reboot</button>--> <!--<button type=\"submit\" name=\"reboot\" class=\"waves-effect waves-light btn grey lighten-1\">Reboot</button>--> </div> </div> </form> </div> </div> </div> </div> <script src=\"https://cdnjs.cloudflare.com/ajax/libs/jquery/3.5.1/jquery.min.js\"></script> <script src=\"https://cdnjs.cloudflare.com/ajax/libs/materialize/1.0.0/js/materialize.min.js\"></script> <script src=\"https://diyhue.org/cdn/nouislider.js\"></script> <script src=\"https://diyhue.org/cdn/diyhue.js\"></script> </body> </html>";

	srvPtr->send(200, "text/html", htmlContent);
	//if (server.args()) {
	//	delay(1000); // needs to wait until response is received by browser. If ESP restarts too soon, browser will think there was an error.
	//	ESP.restart();
	//}
}

void Light::srvOnSwitch(ESP8266WebServer* srvPtr)
{
	srvPtr->send(200, "text/plain", "OK");
}

// Handle request to set hue/brightnes/etc..
void Light::srvOnSet(ESP8266WebServer* srvPtr)
{
	/*uint8_t light;
	float transitiontime = 4;
	for (uint8_t i = 0; i < srvPtr->args(); i++) {
		if (srvPtr->argName(i) == "light") {
			light = srvPtr->arg(i).toInt() - 1;
		}
		else if (srvPtr->argName(i) == "on") {
			if (srvPtr->arg(i) == "True" || srvPtr->arg(i) == "true") {
				light_state[light] = true;
				if (EEPROM.read(1) == 0 && EEPROM.read(0) == 0) {
					EEPROM.write(0, 1);
				}
			}
			else {
				light_state[light] = false;
				if (EEPROM.read(1) == 0 && EEPROM.read(0) == 1) {
					EEPROM.write(0, 0);
				}
			}
			EEPROM.commit();
		}
		else if (srvPtr->argName(i) == "r") {
			lightsData[light].colors[0] = srvPtr->arg(i).toInt();
			color_mode[light] = 0;
		}
		else if (srvPtr->argName(i) == "g") {
			lightsData[light].colors[1] = srvPtr->arg(i).toInt();
			color_mode[light] = 0;
		}
		else if (srvPtr->argName(i) == "b") {
			lightsData[light].colors[2] = srvPtr->arg(i).toInt();
			color_mode[light] = 0;
		}
		else if (srvPtr->argName(i) == "x") {
			lightsData[light].x = srvPtr->arg(i).toFloat();
			color_mode[light] = 1;
		}
		else if (srvPtr->argName(i) == "y") {
			lightsData[light].y = srvPtr->arg(i).toFloat();
			color_mode[light] = 1;
		}
		else if (srvPtr->argName(i) == "bri") {
			lightsData[light].bri = srvPtr->arg(i).toInt();
		}
		else if (srvPtr->argName(i) == "bri_inc") {
			lightsData[light].bri += srvPtr->arg(i).toInt();
			if (lightsData[light].bri > 255) lightsData[light].bri = 255;
			else if (lightsData[light].bri < 0) lightsData[light].bri = 0;
		}
		else if (srvPtr->argName(i) == "ct") {
			lightsData[light].ct = srvPtr->arg(i).toInt();
			color_mode[light] = 2;
		}
		else if (srvPtr->argName(i) == "sat") {
			sat[light] = srvPtr->arg(i).toInt();
			color_mode[light] = 3;
		}
		else if (srvPtr->argName(i) == "hue") {
			hue[light] = srvPtr->arg(i).toInt();
			color_mode[light] = 3;
		}
		else if (srvPtr->argName(i) == "alert" && srvPtr->arg(i) == "select") {
			if (light_state[light]) {
				lightsData[light].currentColors[0] = 0; lightsData[light].currentColors[1] = 0; lightsData[light].currentColors[2] = 0;
			}
			else {
				lightsData[light].currentColors[0] = 255; lightsData[light].currentColors[1] = 255; lightsData[light].currentColors[2] = 255;
			}
		}
		else if (srvPtr->argName(i) == "transitiontime") {
			transitiontime = srvPtr->arg(i).toInt();
		}
	}
	srvPtr->send(200, "text/plain", "OK, x: " + (String)lightsData[light].x + ", y:" + (String)lightsData[light].y + ", bri:" + (String)lightsData[light].bri + ", ct:" + lightsData[light].ct + ", colormode:" + color_mode[light] + ", state:" + light_state[light]);


	if (entertainment_run == false) 
	{
		processLightData(light, transitiontime);
	}
	cache();*/
}

void Light::srvOnGet(ESP8266WebServer* srvPtr)
{

}

void Light::srvOnDetect(ESP8266WebServer* srvPtr, byte* macPtr)
{
	char macString[32] = { 0 };
	sprintf(macString, "%02X:%02X:%02X:%02X:%02X:%02X", *(macPtr), *(macPtr + 1), *(macPtr + 2), *(macPtr + 3), *(macPtr + 4), *(macPtr + 5));

#if ARDUINOJSON_VERSION_MAJOR >= 6
	DynamicJsonDocument  root(300); // calculated with https://arduinojson.org/v6/assistant/
#else
	DynamicJsonBuffer newBuffer;
	JsonObject& root = newBuffer.createObject();
#endif

#ifdef USE_LEGACY_PROTOCOL
	// {"hue": "bulb","lights": 1,"modelid": "LCT015","mac": "cc:50:e3:50:38:d1"}
	root["hue"] = "bulb";
	root["lights"] = LIGHTS_IN_STRIP;
	root["modelid"] = this->getModelId();
	root["mac"] = String(macString);
#else
	root["name"] = deviceConfig.getLightName();
	root["lights"] = LIGHTS_IN_STRIP;
	root["protocol"] = "native_multi";
	root["modelid"] = this->getModelId();
	root["type"] = this->getType();
	root["mac"] = String(macString);
	root["version"] = LIGHT_VERSION;
#endif // USE_LEGACY_PROTOCOL

	String output;
	serializeJson(root, output);
	srvPtr->send(200, "text/plain", output);
}


void Light::srvOnConfigGet(ESP8266WebServer* srvPtr)
{
	DynamicJsonDocument  jsonDoc(1024); // calculated with https://arduinojson.org/v6/assistant/
	jsonDoc["name"] = deviceConfig.getLightName();
	jsonDoc["scene"] = deviceConfig.scene;
	jsonDoc["startup"] = getStartupMode();
	//jsonDoc["hw"] = hwSwitch;
	//jsonDoc["on"] = onPin;
	//jsonDoc["off"] = offPin;
	//jsonDoc["hwswitch"] = (int)hwSwitch;
	jsonDoc["lightscount"] = LIGHTS_IN_STRIP;
	for (uint8_t i = 0; i < LIGHTS_IN_STRIP; i++) {
		jsonDoc["dividedLight_" + String(i)] = (int)ledsPerLight[i];
	}
	jsonDoc["pixelcount"] = TOTAL_LED_COUNT;
	jsonDoc["transitionleds"] = TRANSITION_LEDS;
#ifdef LIGHT_TYPE_RGB
	jsonDoc["rpct"] = rgbMultiplier[COLOR_INDEX_RED];
	jsonDoc["gpct"] = rgbMultiplier[COLOR_INDEX_GREEN];
	jsonDoc["bpct"] = rgbMultiplier[COLOR_INDEX_BLUE];
#endif
#ifdef USE_STATIC_IP
	jsonDoc["dhcp"] = 0;
#else
	jsonDoc["dhcp"] = 1;
#endif // USE_STATIC_IP
	char ipBuff[IP_AS_STR_MAX_LENGTH] = { '\0' };
	char gatwBuff[IP_AS_STR_MAX_LENGTH] = { '\0' };
	char sbnmskBuf[IP_AS_STR_MAX_LENGTH] = { '\0' };
	deviceConfig.networdDataToStr(ipBuff, gatwBuff, sbnmskBuf);
	//jsonDoc["addr"] = (String)device_ip[0] + "." + (String)device_ip[1] + "." + (String)device_ip[2] + "." + (String)device_ip[3];
	//jsonDoc["gw"] = (String)gateway_ip[0] + "." + (String)gateway_ip[1] + "." + (String)gateway_ip[2] + "." + (String)gateway_ip[3];
	//jsonDoc["sm"] = (String)subnet_mask[0] + "." + (String)subnet_mask[1] + "." + (String)subnet_mask[2] + "." + (String)subnet_mask[3];
	jsonDoc["addr"] = ipBuff;
	jsonDoc["gw"] = gatwBuff;
	jsonDoc["sm"] = sbnmskBuf;


	String output;
	serializeJson(jsonDoc, output);
	srvPtr->send(200, "text/plain", output);
}

void Light::srvOnStateGet(ESP8266WebServer* srvPtr) {
	uint8_t lightIndex = srvPtr->arg("light").toInt() - 1;

	DynamicJsonDocument  jsonDoc(300); // calculated with https://arduinojson.org/v6/assistant/
	jsonDoc["on"] = isLightOn(lightIndex);
	jsonDoc["bri"] = lightsData[lightIndex].bri;

	JsonArray xy = jsonDoc.createNestedArray("xy");
	xy.add(lightsData[lightIndex].x);
	xy.add(lightsData[lightIndex].y);
	jsonDoc["ct"] = lightsData[lightIndex].ct;
	jsonDoc["hue"] = lightsData[lightIndex].hue;
	jsonDoc["sat"] = lightsData[lightIndex].sat;
	if (lightsData[lightIndex].colorMode == COLOR_MODE_XY)
		jsonDoc["colormode"] = "xy";
	else if (lightsData[lightIndex].colorMode == COLOR_MODE_CT)
		jsonDoc["colormode"] = "ct";
	else if (lightsData[lightIndex].colorMode == COLOR_MODE_HUE)
		jsonDoc["colormode"] = "hs";
	String output;
	serializeJson(jsonDoc, output);

	srvPtr->send(200, "text/plain", output);
}

void Light::srvOnStatePut(ESP8266WebServer* srvPtr) {
	bool stateSave = false;
	DynamicJsonDocument root(1024);
	DeserializationError error = deserializeJson(root, srvPtr->arg("plain"));

	if (error) {
		srvPtr->send(404, "text/plain", "FAIL. " + srvPtr->arg("plain"));
	}
	else {
		for (JsonPair state : root.as<JsonObject>()) 
		{
			const char* key = state.key().c_str();
			int lightIndex = atoi(key) - 1;
			JsonObject values = state.value();
			int transitiontime = 4;

			if (values.containsKey("xy")) {
				JsonArray xy = values["xy"];
				lightsData[lightIndex].x = xy[0];
				lightsData[lightIndex].y = xy[1];
				lightsData[lightIndex].colorMode = COLOR_MODE_XY;
			}
			else if (values.containsKey("ct")) {
				lightsData[lightIndex].ct = values["ct"];
				lightsData[lightIndex].colorMode = COLOR_MODE_CT;
			}
			else {
				if (values.containsKey("hue")) {
					SerialPrintln("Set HUE");
					lightsData[lightIndex].hue = values["hue"];
					lightsData[lightIndex].colorMode = COLOR_MODE_HUE;
				}
				if (values.containsKey("sat")) {
					lightsData[lightIndex].sat = values["sat"];
					lightsData[lightIndex].colorMode = COLOR_MODE_HUE;
				}
			}

			if (values.containsKey("on")) {
				handleTurnOnOrOff(lightIndex, values["on"]);
				if (isOnStartupRestoreLastState() == true) {
					stateSave = true;
				}
			}

			if (values.containsKey("bri")) {
				lightsData[lightIndex].bri = values["bri"];
			}

			if (values.containsKey("bri_inc")) {
				lightsData[lightIndex].bri += (int)values["bri_inc"];
				if (lightsData[lightIndex].bri > 255) lightsData[lightIndex].bri = 255;
				else if (lightsData[lightIndex].bri < 1) lightsData[lightIndex].bri = 1;
			}

			if (values.containsKey("transitiontime")) {
				// Transition Time is given in 100ms units (eg 50 equal to 5 seconds)
				transitiontime = values["transitiontime"];
				SerialPrint("[BaseLight::srvOnStatePut] Set transitiontime="); SerialPrintWithFormat(transitiontime, DEC);
				SerialPrint("-->"); SerialPrintWithFormat(transitiontime / 10, DEC); SerialPrint("s");
				SerialPrintln();
				SerialPrint("Transition time has been set at: "); SerialPrintWithFormat(millis(), DEC); SerialPrint("ms");
				SerialPrintln();
			}

			if (values.containsKey("alert") && values["alert"] == "select") {
				if (lightsData[lightIndex].lightState) {
					lightsData[lightIndex].currentColors[0] = 0; lightsData[lightIndex].currentColors[1] = 0; lightsData[lightIndex].currentColors[2] = 0;
				}
				else {
					lightsData[lightIndex].currentColors[1] = 126; lightsData[lightIndex].currentColors[2] = 126;
				}
			}

			// request contains info about alerts (flashing light single or multiple times)
			if (values.containsKey("alert")) {
				if (values["alert"] == ALERT_SINGLE_FLASH) {
					this->setLightInFlashMode( &(this->lightsData[lightIndex]), ALERT_SINGLE_FLASH_FLASHES_COUNT, ALERT_SINGLE_FLASH_FLASHES_FREQUENCY_MS );
				}
				else if (values["alert"] == ALERT_MULTIPLE_FLASH) {
					this->setLightInFlashMode( &(this->lightsData[lightIndex]), ALERT_MULTIPLE_FLASH_FLASHES_COUNT, ALERT_MULTIPLE_FLASH_FLASHES_FREQUENCY_MS );
				}
				else {
					this->setLightInNormalMode(&(this->lightsData[lightIndex]));
				}
			}


			processLightData(lightIndex, transitiontime);

			//printDebug();
		}
		String output;
		serializeJson(root, output);
		srvPtr->send(200, "text/plain", output);
		/*if (stateSave) {
			saveState();
		}*/
		raiseSaveStateRequest();
	}
}


void Light::printDebug()
{
#ifdef USE_SERIAL_AS_DEBUG
	for (uint8_t i = 0; i < LIGHTS_IN_STRIP; i++) {
		Serial.print("--- LIGHT "); Serial.print(i, DEC); Serial.println(" ---");
		Serial.print("hue:"); Serial.println(this->lightsData[i].hue, DEC);
		Serial.print("bri:"); Serial.println(this->lightsData[i].bri, DEC);
		Serial.print("sat:"); Serial.println(this->lightsData[i].sat, DEC);
		Serial.print("x:"); Serial.println(lightsData[i].x, DEC);
		Serial.print("y:"); Serial.println(lightsData[i].y, DEC);
		Serial.print("colorMode:"); Serial.println(this->lightsData[i].colorMode, DEC);
		Serial.print("light_state:"); Serial.println(isLightOn(i), DEC);
		
		Serial.print("Current color:"); Serial.print(lightsData[i].currentColors[COLOR_INDEX_RED], DEC);
		Serial.print(","); Serial.print(lightsData[i].currentColors[COLOR_INDEX_GREEN], DEC);
		Serial.print(","); Serial.print(lightsData[i].currentColors[COLOR_INDEX_BLUE], DEC);
		Serial.println("]");
		
		Serial.print("Target color:"); Serial.print(lightsData[i].colors[COLOR_INDEX_RED], DEC);
		Serial.print(","); Serial.print(lightsData[i].colors[COLOR_INDEX_GREEN], DEC);
		Serial.print(","); Serial.print(lightsData[i].colors[COLOR_INDEX_BLUE], DEC);
		Serial.println("]");
		
		Serial.print("Colors steps: ["); Serial.print(lightsData[i].stepLevel[COLOR_INDEX_RED], DEC);
		Serial.print(","); Serial.print(lightsData[i].stepLevel[COLOR_INDEX_GREEN], DEC);
		Serial.print(","); Serial.print(lightsData[i].stepLevel[COLOR_INDEX_BLUE], DEC);
		Serial.println("]");

	}
#endif // USE_SERIAL_AS_DEBUG
}



bool Light::isAnyLightInFlashMode() {
	for (uint8_t i = 0; i < LIGHTS_IN_STRIP; i++) {
		if (lightsData[i].mode == LightMode::Flashing && this->lightsData[i].flashFreqMs > 0) {
			return true;
		}
	}
	return false;
}


RgbColor Light::blendingEntert(float left[3], float right[3], float pixel) {
	uint8_t result[3];
	for (uint8_t i = 0; i < 3; i++) {
		float percent = (float)pixel / (float)(TRANSITION_LEDS + 1);
		result[i] = (left[i] * (1.0f - percent) + right[i] * percent) / 2;
	}
	return RgbColor((uint8_t)result[0], (uint8_t)result[1], (uint8_t)result[2]);
}

void Light::entertainment() {
	
	if (this->wifiUdp_ptr != NULL) {
		this->wifiUdp_ptr->begin(2100);
	}
	else {
		SerialPrintln("[Light::entertainment] ERROR, WIFI UDP POINTER IS NULL");
		return;
	}

	uint8_t packetSize = this->wifiUdp_ptr->parsePacket();
	if (packetSize) {
		SerialPrint("[Light::entertainment] Received UDP packet - Size:");
		SerialPrintWithFormat(packetSize, DEC);
		SerialPrintln(" - Content:");
		for (uint16_t i = 0; i < packetSize; i++) {
			if (i > 0) {
				SerialPrint(",");
			}
			SerialPrintf("%02X", packetBuffer[i]);
		}
		SerialPrintln();

		if (!entertainmentRun) {
			entertainmentRun = true;
		}
		lastEPMillis = millis();
		this->wifiUdp_ptr->read(packetBuffer, packetSize);
		for (uint8_t i = 0; i < packetSize / 4; i++) {
			lightsData[packetBuffer[i * 4]].currentColors[0] = packetBuffer[i * 4 + 1] * rgbMultiplier[0] / 100;
			lightsData[packetBuffer[i * 4]].currentColors[1] = packetBuffer[i * 4 + 2] * rgbMultiplier[1] / 100;
			lightsData[packetBuffer[i * 4]].currentColors[2] = packetBuffer[i * 4 + 3] * rgbMultiplier[2] / 100;
		}
		for (uint8_t light = 0; light < LIGHTS_IN_STRIP; light++) {
			if (LIGHTS_IN_STRIP > 1) {
				if (light == 0) {
					for (int pixel = 0; pixel < ledsPerLight[0]; pixel++)
					{
						if (pixel < ledsPerLight[0] - TRANSITION_LEDS / 2) {
							strip.SetPixelColor(pixel, convFloat(lightsData[light].currentColors));
						}
						else {
							strip.SetPixelColor(pixel, blendingEntert(lightsData[0].currentColors, lightsData[1].currentColors, pixel + 1 - (ledsPerLight[0] - TRANSITION_LEDS / 2)));
						}
					}
				}
				else {
					for (int pixel = 0; pixel < ledsPerLight[light]; pixel++)
					{
						long pixelSum;
						for (int value = 0; value < light; value++)
						{
							if (value + 1 == light) {
								pixelSum += ledsPerLight[value] - TRANSITION_LEDS;
							}
							else {
								pixelSum += ledsPerLight[value];
							}
						}
						if (pixel < TRANSITION_LEDS / 2) {
							strip.SetPixelColor(pixel + pixelSum + TRANSITION_LEDS, blendingEntert(lightsData[light - 1].currentColors, lightsData[light].currentColors, pixel + 1));
						}
						else if (pixel > ledsPerLight[light] - TRANSITION_LEDS / 2 - 1) {
							//Serial.println(String(pixel));
							strip.SetPixelColor(pixel + pixelSum + TRANSITION_LEDS, blendingEntert(lightsData[light].currentColors, lightsData[light + 1].currentColors, pixel + TRANSITION_LEDS / 2 - ledsPerLight[light]));
						}
						else {
							strip.SetPixelColor(pixel + pixelSum + TRANSITION_LEDS, convFloat(lightsData[light].currentColors));
						}
						pixelSum = 0;
					}
				}
			}
			else {
				strip.ClearTo(convFloat(lightsData[light].currentColors), 0, TOTAL_LED_COUNT - 1);
			}
		}
		strip.Show();
	}
}

void  Light::run(bool isConnected)
{
	if (isConnected == true || hasBeenNoConnectionNotifyFired == true)
	{
		if (this->isAnyLightInFlashMode()) {
			for (uint8_t i = 0; i < LIGHTS_IN_STRIP; i++) {
				if (lightsData[i].mode == LightMode::Flashing && this->lightsData[i].flashFreqMs > 0) {
					this->handleFlashMode(i, &(this->lightsData[i]));
					break;
				}
			}
		}
		else {

			if (entertainmentRun == false) {
				this->lightEngine();
			}
			else {
				//Serial.println("entertainment run");
				if ((millis() - lastEPMillis) >= ENTERTAINMENT_TIMEOUT) {
					entertainmentRun = false;
					for (uint8_t i = 0; i < LIGHTS_IN_STRIP; i++) {
						processLightData(i, 4); //return to original colors with 0.4 sec transition
					}
				}
			}
			entertainment();

			// reset the 'not connected' flag to handle future 'not connected' error
			if (isConnected == true && hasBeenNoConnectionNotifyFired == true) {
				hasBeenNoConnectionNotifyFired = false;
			}

			// Avoid EEPROM wearing out, so save state only when it is 'stable'
			// not during rapid changes (eg user scroll through colors)
			if (requireSavingNewState == true)
			{
				if ((millis() - latestStateSave) >= EEPROM_SAVE_STATE_DELAY_MS)
				{
					saveState();
					requireSavingNewState = false;
				}
			}
		}

	}
	else {
		this->setLightInFlashMode(&(this->lightsData[0]), 20, 2000);
		hasBeenNoConnectionNotifyFired = true;
	}


}

#endif // WS2812B_RGB_LED_STRIP