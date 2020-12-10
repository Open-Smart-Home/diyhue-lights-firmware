#include "BaseLight.h"



void BaseLight::init(String* lightName)
{
	//this->lightName = lightName;
	deviceConfig.setLightName(lightName);

	this->rgbMultiplier[COLOR_INDEX_RED] = 100;
	this->rgbMultiplier[COLOR_INDEX_GREEN] = 100;
	this->rgbMultiplier[COLOR_INDEX_BLUE] = 100;

    hasBeenNoConnectionNotifyFired = false;
	BaseLight::initLightData(&(this->lightData));

	// initialize HW (eg. led driver, etc)
	this->initPlatformSpecific();

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

void BaseLight::setNetworkCfg(IPAddress deviceIp, IPAddress gatewayIp, IPAddress subnetMask)
{
	/*this->device_ip = deviceIp;
	this->gateway_ip = gatewayIp;
	this->subnet_mask = subnetMask;*/
	this->deviceConfig.saveNetworkData(&deviceIp, &gatewayIp, &subnetMask);
}


//bool BaseLight::isLightOn(uint8_t lightIndex){ if(lightIndex == 1){ return this->lightData.lightState == true}}
//void BaseLight::setLightOn(uint8_t lightIndex);
//void BaseLight::setLightOff(uint8_t lightIndex);
bool BaseLight::isLightOn() { return lightData.lightState == true; }
void BaseLight::setLightOn() { lightData.lightState = true; }
void BaseLight::setLightOff() { lightData.lightState = false; }


uint8_t BaseLight::getColorChannelsCount()
{
	return CHANNELS_COUNT;
}


void BaseLight::saveStartupMode(uint8_t mode)
{
	SerialPrint("Set startup mode:"); SerialPrintlnWithFormat(mode, DEC);
//    EEPROM.begin(USED_EEPROM_SIZE);
//	EEPROM.write(EEPROM_ADDR_STARTUP_MODE, mode);
//	EEPROM.commit();
    deviceConfig.setStartMode(mode);
    SaveRestoreDataModule::saveConfig(&deviceConfig);
}

uint8_t  BaseLight::getStartupMode()
{
//    EEPROM.begin(USED_EEPROM_SIZE);
//	uint8_t mode = EEPROM.read(EEPROM_ADDR_STARTUP_MODE);
//	EEPROM.end();
//    return mode;
}

bool  BaseLight::isOnStartupRestoreLastState() { return deviceConfig.isStartupModePowerFail(); }
bool  BaseLight::isOnStartupWhiteMaxBrightness() { return deviceConfig.isStartupModeSafety(); }


bool BaseLight::hasStartupModeBeenSet()
{
	if (isFirstPowerUp() == true) {
		return false;
	}
	return true;
}
bool BaseLight::isFirstPowerUp()
{
	return (EepromModule::isEpromDataCompliant() == false);
}

//bool BaseLight::isEpromDataCompliant()
//{
//    EEPROM.begin(USED_EEPROM_SIZE);
//	uint8_t test = EEPROM.read(EEPROM_ADDR_CONTROL_VAL);
//	EEPROM.end();
//	
//	SerialPrint("[BaseLight::isEpromDataCompliant] EEPROM addr (");
//	SerialPrint(EEPROM_ADDR_CONTROL_VAL);
//	SerialPrint(")-->");
//	SerialPrint(test);
//	SerialPrint("/");
//	SerialPrint(EEPROM_CTRL_VAL);
//	SerialPrintln();
//    
//	return (test == EEPROM_CTRL_VAL);
//}

// Handle the startup procedure on power on
// (eg. after the light has been switched off by wall switch)
// The light may restore last state, set white max brightness, etc
void BaseLight::handleLightStartup()
{
	if (hasStartupModeBeenSet() == true)
	{
		if (isOnStartupWhiteMaxBrightness() == true)
		{
			SerialPrintln("[BaseLight] Startup mode set to white max brightness");
			setWhiteMaxBrightness();
		}
		else if (isOnStartupRestoreLastState() == true)
		{
			SerialPrintln("[BaseLight] Startup mode set to restore last state");
			restoreLastState();
		}
	}
	/*else {
        SerialPrintln("[BaseLight] No startup mode set");
		setFirstTimeMode();
	}*/
}


void BaseLight::copyLightData(LightData *in, LightData *out) {
    out->bri = in->bri;
    out->colorMode = in ->colorMode;
    out->ct = in ->ct;
    out->hue = in ->hue;
    out->lightState = in->lightState;
    out->sat = in ->sat;
    out->x = in ->x;
    out->y = in ->y;
    out->colors[0] = in ->colors[0];
    out->colors[1] = in ->colors[1];
    out->colors[2] = in ->colors[2];
    out->colors[3] = in ->colors[3];
    out->currentColors[0] = in ->currentColors[0];
    out->currentColors[1] = in ->currentColors[1];
    out->currentColors[2] = in ->currentColors[2];
    out->currentColors[3] = in ->currentColors[3];
    out->stepLevel[0] = in ->stepLevel[0];
    out->stepLevel[1] = in ->stepLevel[1];
    out->stepLevel[2] = in ->stepLevel[2];
    out->stepLevel[3] = in ->stepLevel[3];
}

void BaseLight::saveState()
{
	SaveRestoreDataModule::saveLightData(&(this->lightData), 0);
}

void BaseLight::restoreLastState()
{
	if (SaveRestoreDataModule::restoreLightData(&(this->lightData), 0))
	{

		if (this->lightData.colorMode == COLOR_MODE_XY && this->isLightOn()) {
			convertXy(&lightData);
		}
		else if (this->lightData.colorMode == COLOR_MODE_CT && this->isLightOn()) {
			convertCt(&lightData);
		}
		else if (this->lightData.colorMode == COLOR_MODE_HUE && this->isLightOn()) {
			convertHue(&lightData);
		}

		applyColorsImmediatly();

		//SerialPrintln("--------- Restored ---------");
	}
	else {
		SerialPrintln("[BaseLight::restoreLastState] Error on restoring last state");
	}
}


void BaseLight::initLightData(LightData* lightData_p) {
	lightData_p->mode = LightMode::Normal;
	lightData_p->toDoFlashesLeft = 0;
	lightData_p->lastFlash = 0;
	lightData_p->flashFreqMs = 0;
}

void BaseLight::setFirstTimeMode()
{
	this->setWhiteMaxBrightness();
}


// Set the light to a white (warm or cold) at full brightnes.
// This is may change according with light hardware
void BaseLight::setWhiteMaxBrightness()
{
	setLightOn();
	lightData.bri = MAX_BRIGHTNESS;

#ifdef HAS_COLOR_MODE_CT
	lightData.colorMode = COLOR_MODE_CT;
	lightData.ct = COLOR_TEMP_NEUTRAL + 20; // neutral/slight warm white
	convertCt(&lightData);
#endif
#ifndef HAS_COLOR_MODE_CT
#ifdef HAS_COLOR_MODE_XY
	lightData.colorMode = COLOR_MODE_XY;
	// set xy color value to slight warm white
	lightData.x = 0.47f; //? ? ? check
	lightData.y = 0.45f; //? ? ? check
	convertXy(&lightData);
#endif
#endif // !HAS_COLOR_MODE_CT

	/*SerialPrint("Color mode:");
	SerialPrintln(lightData.colorMode, DEC);*/

	applyColorsImmediatly();
}


//void BaseLight::convert_hue(uint8_t* rgbw_p, int hue, uint8_t sat, uint8_t bri)
//{
//	double      hh, p, q, t, ff, s, v;
//	long        i;
//
//	rgbw_p[CHANNELS_COUNT] = 0;
//	// normalize saturation and brightness
//	s = sat / 255.0;
//	v = bri / 255.0;
//
//	if (s <= 0.0) {      // < is bogus, just shuts up warnings
//		rgbw_p[0] = v;
//		rgbw_p[1] = v;
//		rgbw_p[2] = v;
//		return;
//	}
//
//	hh = lightData.hue;
//	if (hh >= 65535.0) hh = 0.0;
//	hh /= 11850, 0;
//	i = (long)hh;
//	ff = hh - i;
//	p = v * (1.0 - s);
//	q = v * (1.0 - (s * ff));
//	t = v * (1.0 - (s * (1.0 - ff)));
//
//	switch (i) {
//	case 0:
//		rgbw_p[0] = v * 255.0;
//		rgbw_p[1] = t * 255.0;
//		rgbw_p[2] = p * 255.0;
//		break;
//	case 1:
//		rgbw_p[0] = q * 255.0;
//		rgbw_p[1] = v * 255.0;
//		rgbw_p[2] = p * 255.0;
//		break;
//	case 2:
//		rgbw_p[0] = p * 255.0;
//		rgbw_p[1] = v * 255.0;
//		rgbw_p[2] = t * 255.0;
//		break;
//
//	case 3:
//		rgbw_p[0] = p * 255.0;
//		rgbw_p[1] = q * 255.0;
//		rgbw_p[2] = v * 255.0;
//		break;
//	case 4:
//		rgbw_p[0] = t * 255.0;
//		rgbw_p[1] = p * 255.0;
//		rgbw_p[2] = v * 255.0;
//		break;
//	case 5:
//	default:
//		rgbw_p[0] = v * 255.0;
//		rgbw_p[1] = p * 255.0;
//		rgbw_p[2] = q * 255.0;
//		break;
//	}
//}

void BaseLight::convertHue(LightData* lightData)
{
	double      hh, p, q, t, ff, s, v;
	long        i;

	s = lightData->sat / 255.0;
	v = lightData->bri / 255.0;

	if (s <= 0.0) {      // < is bogus, just shuts up warnings
		lightData->colors[0] = v;
		lightData->colors[1] = v;
		lightData->colors[2] = v;
		return;
	}
	hh = lightData->hue;
	if (hh >= 65535.0) hh = 0.0;
	hh /= 11850, 0;
	i = (long)hh;
	ff = hh - i;
	p = v * (1.0 - s);
	q = v * (1.0 - (s * ff));
	t = v * (1.0 - (s * (1.0 - ff)));

	switch (i) {
	case 0:
		lightData->colors[0] = v * 255.0;
		lightData->colors[1] = t * 255.0;
		lightData->colors[2] = p * 255.0;
		break;
	case 1:
		lightData->colors[0] = q * 255.0;
		lightData->colors[1] = v * 255.0;
		lightData->colors[2] = p * 255.0;
		break;
	case 2:
		lightData->colors[0] = p * 255.0;
		lightData->colors[1] = v * 255.0;
		lightData->colors[2] = t * 255.0;
		break;

	case 3:
		lightData->colors[0] = p * 255.0;
		lightData->colors[1] = q * 255.0;
		lightData->colors[2] = v * 255.0;
		break;
	case 4:
		lightData->colors[0] = t * 255.0;
		lightData->colors[1] = p * 255.0;
		lightData->colors[2] = v * 255.0;
		break;
	case 5:
	default:
		lightData->colors[0] = v * 255.0;
		lightData->colors[1] = p * 255.0;
		lightData->colors[2] = q * 255.0;
		break;
	}
}


void BaseLight::normalizeFloatRgb(float* r_ptr, float* g_ptr, float* b_ptr)
{
	if (*r_ptr > *b_ptr && *r_ptr > *g_ptr && *r_ptr > 1.0f) {
		// red is greatest and out of range
		*g_ptr = *g_ptr / *r_ptr;
		*b_ptr = *b_ptr / *r_ptr;
		*r_ptr = 1.0f;
	}
	else if (*g_ptr > *b_ptr && *g_ptr > *r_ptr && *g_ptr > 1.0f) {
		// green is greatest and out of range
		*r_ptr = *r_ptr / *g_ptr;
		*b_ptr = *b_ptr / *g_ptr;
		*g_ptr = 1.0f;
	}
	else if (*b_ptr > *r_ptr && *b_ptr > *g_ptr && *b_ptr > 1.0f) {
		// blue is greatest and out of range
		*r_ptr = *r_ptr / *b_ptr;
		*g_ptr = *g_ptr / *b_ptr;
		*b_ptr = 1.0f;
	}
}

uint8_t BaseLight::correctBrightness(uint8_t brightness)
{
	return brightness;
}

uint8_t BaseLight::calculateOptimalBrightness(uint8_t brightness)
{
	if (brightness < 5) {
		return 5;
	}
	return brightness;
}

uint8_t BaseLight::checkRgbInterval(int val)
{
	if (val < 0) { return  0; }
	if (val > 255) { return 255; };
	return val;
}

float BaseLight::applyGammaCorrection(float val)
{
	// TODO: this will change on different platforms

	// WS2812
	return (val <= 0.04045f ? val / 12.92f : pow((val + 0.055f) / (1.0f + 0.055f), 2.4f));
}


// USED UNTIL 2020-11-26
//// This algorithm convert color from X, Y and brightness to sRGB
//// It follow steps reported here
//// https://github.com/PhilipsHue/PhilipsHueSDK-iOS-OSX/blob/00187a3db88dedd640f5ddfa8a474458dff4e1db/ApplicationDesignNotes/RGB%20to%20xy%20Color%20conversion.md
//void BaseLight::convertXy(LightData* lightData_p)
//{
//	uint8_t optimalBri = calculateOptimalBrightness(lightData_p->bri);
//
//	float z = 1.0f - lightData_p->x - lightData_p->y;
//	float Y = ((float)(optimalBri)) / (float)MAX_BRIGHTNESS;
//	float X = (Y / lightData_p->y) * lightData_p->x;
//	float Z = (Y / lightData_p->y) * z;
//
//
//	// sRGB D65 conversion
//	float r = X * 3.2406f - Y * 1.5372f - Z * 0.4986f;
//	float g = -X * 0.9689f + Y * 1.8758f + Z * 0.0415f;
//	float b = X * 0.0557f - Y * 0.2040f + Z * 1.0570f;
//	/*Serial.print("r:"); Serial.print(r, DEC); Serial.print(", ");
//	Serial.print("g:"); Serial.print(g, DEC); Serial.print(", ");
//	Serial.print("g:"); Serial.print(b, DEC);
//	Serial.println();*/
//
//
//	this->normalizeFloatRgb(&r, &g, &b);
//	/*Serial.print("r:"); Serial.print(r, DEC); Serial.print(", ");
//	Serial.print("g:"); Serial.print(g, DEC); Serial.print(", ");
//	Serial.print("g:"); Serial.print(b, DEC);
//	Serial.println();*/
//
//	// Apply gamma correction
//	r = applyGammaCorrection(r);
//	g = applyGammaCorrection(g);
//	b = applyGammaCorrection(b);
//	/*Serial.print("r:"); Serial.print(r, DEC); Serial.print(", ");
//	Serial.print("g:"); Serial.print(g, DEC); Serial.print(", ");
//	Serial.print("g:"); Serial.print(b, DEC);
//	Serial.println();*/
//
//	this->normalizeFloatRgb(&r, &g, &b);
//	/*Serial.print("r:"); Serial.print(r, DEC); Serial.print(", ");
//	Serial.print("g:"); Serial.print(g, DEC); Serial.print(", ");
//	Serial.print("g:"); Serial.print(b, DEC);
//	Serial.println();*/
//
//
//	r = r < 0 ? 0 : r;
//	g = g < 0 ? 0 : g;
//	b = b < 0 ? 0 : b;
//
//#ifdef LIGHT_TYPE_CCT
//
//#endif
//#if defined(LIGHT_TYPE_RGB) || defined(LIGHT_TYPE_RGBW) || defined(LIGHT_TYPE_RGBCCT)
//	lightData.colors[COLOR_INDEX_RED] = (int)(r * 255.0f);
//	lightData.colors[COLOR_INDEX_GREEN] = (int)(g * 255.0f);
//	lightData.colors[COLOR_INDEX_BLUE] = (int)(b * 255.0f);
//#endif
//
//#if defined(LIGHT_TYPE_RGBW) || defined(LIGHT_TYPE_RGBCCT)
//	lightData.colors[COLOR_INDEX_WHITE] = 0;
//#endif
//#ifdef LIGHT_TYPE_RGBCCT
//	rgbw_p[COLOR_INDEX_WARM_WHITE] = 0;
//#endif
//
//}

// USED SINCE 2020-11-26
// This algorithm convert color from X, Y and brightness to sRGB
// It follow steps reported here
// https://github.com/PhilipsHue/PhilipsHueSDK-iOS-OSX/blob/00187a3db88dedd640f5ddfa8a474458dff4e1db/ApplicationDesignNotes/RGB%20to%20xy%20Color%20conversion.md
void BaseLight::convertXy(LightData* lightData_p)
{
	uint16_t optimalBri = int(10 + lightData_p->bri / 1.04);

	float Y = lightData_p->y;
	float X = lightData_p->x;
	float Z = 1.0f - lightData_p->x - lightData_p->y;


	// sRGB D65 conversion
	float r = X * 3.2406f - Y * 1.5372f - Z * 0.4986f;
	float g = -X * 0.9689f + Y * 1.8758f + Z * 0.0415f;
	float b = X * 0.0557f - Y * 0.2040f + Z * 1.0570f;
	/*Serial.print("r:"); Serial.print(r, DEC); Serial.print(", ");
	Serial.print("g:"); Serial.print(g, DEC); Serial.print(", ");
	Serial.print("g:"); Serial.print(b, DEC);
	Serial.println();*/

	// Apply gamma correction
	r = applyGammaCorrection(r);
	g = applyGammaCorrection(g);
	b = applyGammaCorrection(b);
	/*Serial.print("r:"); Serial.print(r, DEC); Serial.print(", ");
	Serial.print("g:"); Serial.print(g, DEC); Serial.print(", ");
	Serial.print("g:"); Serial.print(b, DEC);
	Serial.println();*/

#ifdef LIGHT_TYPE_RGB
	// Apply multiplier for white correction
	r = r * rgbMultiplier[0] / 100;
	g = g * rgbMultiplier[1] / 100;
	b = b * rgbMultiplier[2] / 100;
#endif // LIGHT_TYPE_RGB


	this->normalizeFloatRgb(&r, &g, &b);
	/*Serial.print("r:"); Serial.print(r, DEC); Serial.print(", ");
	Serial.print("g:"); Serial.print(g, DEC); Serial.print(", ");
	Serial.print("g:"); Serial.print(b, DEC);
	Serial.println();*/


	r = r < 0 ? 0 : r;
	g = g < 0 ? 0 : g;
	b = b < 0 ? 0 : b;

#ifdef LIGHT_TYPE_CCT

#endif
#if defined(LIGHT_TYPE_RGB) || defined(LIGHT_TYPE_RGBW) || defined(LIGHT_TYPE_RGBCCT)
	lightData_p->colors[COLOR_INDEX_RED] = (int)(r * optimalBri);
	lightData_p->colors[COLOR_INDEX_GREEN] = (int)(g * optimalBri);
	lightData_p->colors[COLOR_INDEX_BLUE] = (int)(b * optimalBri);
#endif

#if defined(LIGHT_TYPE_RGBW) || defined(LIGHT_TYPE_RGBCCT)
	lightData_p->colors[COLOR_INDEX_WHITE] = 0;
#endif
#ifdef LIGHT_TYPE_RGBCCT
	rgbw_p[COLOR_INDEX_WARM_WHITE] = 0;
#endif

}

// USED UNTIL 2020-11-26
//void BaseLight::convertCt(LightData* lightData_p)
//{
//#ifdef HAS_COLOR_MODE_CT
//	int bri = lightData_p->bri;
//
//	int hectemp = 10000 / lightData_p->ct;
//	int r, g, b;
//	if (hectemp <= 66) {
//		r = 255;
//		g = 99.4708025861 * log(hectemp) - 161.1195681661;
//		b = hectemp <= 19 ? 0 : (138.5177312231 * log(hectemp - 10) - 305.0447927307);
//	}
//	else {
//		r = 329.698727446 * pow(hectemp - 60, -0.1332047592);
//		g = 288.1221695283 * pow(hectemp - 60, -0.0755148492);
//		b = 255;
//	}
//	r = checkRgbInterval(r);
//	g = checkRgbInterval(g);
//	b = checkRgbInterval(b);
//
//	lightData_p->colors[COLOR_INDEX_RED] = r * (bri / 255.0f);
//	lightData_p->colors[COLOR_INDEX_GREEN] = g * (bri / 255.0f);
//	lightData_p->colors[COLOR_INDEX_BLUE] = b * (bri / 255.0f);
//	lightData_p->colors[COLOR_INDEX_WHITE] = bri;
//#endif
//}

// USED FROM 2020-11-26
void BaseLight::convertCt(LightData* lightData_p)
{
//#ifdef HAS_COLOR_MODE_CT
#ifdef LIGHT_TYPE_RGB
	convertCtForRGB(lightData_p);
#endif

#ifdef LIGHT_TYPE_RGBW
	convertCtForRGBW(lightData_p);
#endif

#ifdef LIGHT_TYPE_RGBCCT
	int optimal_bri = int(10 + lightData.bri / 1.04);

	lightData_p->colors[COLOR_INDEX_RED]	= 0;
	lightData_p->colors[COLOR_INDEX_GREEN]	= 0;
	lightData_p->colors[COLOR_INDEX_BLUE]	= 0;
	
	uint8 percent_warm = ((lightData.ct - 150) * 100) / 350;

	lightData_p->colors[COLOR_INDEX_WARM_WHITE] = (optimal_bri * (100 - percent_warm)) / 100;
	lightData_p->colors[COLOR_INDEX_COLD_WHITE] = (optimal_bri * percent_warm) / 100;
#endif
//#endif
}

void BaseLight::convertCtForRGB(LightData* lightData_p) {
#ifdef LIGHT_TYPE_RGB
	int hecTemp = 10000 / lightData_p->ct;
	int r, g, b;
	if (hecTemp <= 66) {
		r = 255;
		g = 99.4708025861 * log(hecTemp) - 161.1195681661;
		b = hecTemp <= 19 ? 0 : (138.5177312231 * log(hecTemp - 10) - 305.0447927307);
	}
	else {
		r = 329.698727446 * pow(hecTemp - 60, -0.1332047592);
		g = 288.1221695283 * pow(hecTemp - 60, -0.0755148492);
		b = 255;
	}

	r = r > 255 ? 255 : r;
	g = g > 255 ? 255 : g;
	b = b > 255 ? 255 : b;

	// Apply multiplier for white correction
	r = r * rgbMultiplier[0] / 100;
	g = g * rgbMultiplier[1] / 100;
	b = b * rgbMultiplier[2] / 100;

	lightData_p->colors[COLOR_INDEX_RED] = r * (lightData_p->bri / 255.0f);
	lightData_p->colors[COLOR_INDEX_GREEN] = g * (lightData_p->bri / 255.0f);
	lightData_p->colors[COLOR_INDEX_BLUE] = b * (lightData_p->bri / 255.0f);
#endif // LIGHT_TYPE_RGB
}


/*
 * Contribute: YannikW
 * This converts a CT to a mix of a white led with a color temperature defines in WHITE_TEMP,
 * plus RGB shades to achieve full white spectrum.
 * CT value is in mired: https://en.wikipedia.org/wiki/Mired
 * Range is between 153 (equals 6536K cold-white) and 500 (equals 2000K warm-white)
 * To shift the white led to warmer or colder white shades we mix a "RGB-white" to the white led.
 * This RGB white is calculated as in old convert_ct methode, with formulars by: http://www.tannerhelland.com/4435/convert-temperature-rgb-algorithm-code/
 * If the desired CT equals the white channel CT, we add 0% RGB-white, the more we shift away we add more RGB-white.
 * At the lower or higher end we add 100% RGB-white and reduce the led-white down to 50%
 */
void BaseLight::convertCtForRGBW(LightData* lightData_p) {
#ifdef HAS_COLOR_MODE_CT
	int optimal_bri = int(10 + lightData_p->bri / 1.04);
	int hectemp = 10000 / lightData_p->ct;
	int r, g, b;
	if (hectemp <= 66) {
		r = 255;
		g = 99.4708025861 * log(hectemp) - 161.1195681661;
		b = hectemp <= 19 ? 0 : (138.5177312231 * log(hectemp - 10) - 305.0447927307);
	}
	else {
		r = 329.698727446 * pow(hectemp - 60, -0.1332047592);
		g = 288.1221695283 * pow(hectemp - 60, -0.0755148492);
		b = 255;
	}
	r = r > 255 ? 255 : r;
	g = g > 255 ? 255 : g;
	b = b > 255 ? 255 : b;


	// calculate mix factor
	double mixFactor;
	int temp = hectemp * 100;
	if (temp >= WHITE_TEMP) {
		// mix cold-rgb-white to led-white
		mixFactor = (double)(temp - WHITE_TEMP) / (6536.0 - WHITE_TEMP); //0.0 - 1.0
	}
	else {
		// mix warm-rgb-white to led-white
		mixFactor = (double)(WHITE_TEMP - temp) / (WHITE_TEMP - 2000.0); //0.0 - 1.0
	}
	// constrain to 0-1
	mixFactor = mixFactor > 1.0 ? 1.0 : mixFactor;
	mixFactor = mixFactor < 0.0 ? 0.0 : mixFactor;

	lightData_p->colors[COLOR_INDEX_RED] = r * (optimal_bri / 255.0f) * mixFactor;
	lightData_p->colors[COLOR_INDEX_GREEN] = g * (optimal_bri / 255.0f) * mixFactor;
	lightData_p->colors[COLOR_INDEX_BLUE] = b * (optimal_bri / 255.0f) * mixFactor;

	// reduce white brightness by 50% on maximum mixFactor
	lightData_p->colors[COLOR_INDEX_WHITE] = optimal_bri * (1.0 - (mixFactor * 0.5));
#endif
}


void BaseLight::applyScene(uint8_t new_scene)
{
	if (new_scene == 0) {
		this->lightData.bri = 144; this->lightData.ct = 447; this->lightData.colorMode = COLOR_MODE_CT; convertCt(&lightData);
	}
	else if (new_scene == 1) {
		this->lightData.bri = 254; this->lightData.ct = 346; this->lightData.colorMode = COLOR_MODE_CT; convertCt(&lightData);
	}
	else if (new_scene == 2) {
		this->lightData.bri = 254; this->lightData.ct = 233; this->lightData.colorMode = COLOR_MODE_CT; convertCt(&lightData);
	}
	else if (new_scene == 3) {
		this->lightData.bri = 254; this->lightData.ct = 156; this->lightData.colorMode = COLOR_MODE_CT; convertCt(&lightData);
	}
	else if (new_scene == 4) {
		this->lightData.bri = 77; this->lightData.ct = 367; this->lightData.colorMode = COLOR_MODE_CT; convertCt(&lightData);
	}
	else if (new_scene == 5) {
		this->lightData.bri = 254; this->lightData.ct = 447; this->lightData.colorMode = COLOR_MODE_CT; convertCt(&lightData);
	}
	else if (new_scene == 6) {
		this->lightData.bri = 1; this->lightData.x = 0.561; this->lightData.y = 0.4042; this->lightData.colorMode = COLOR_MODE_XY; convertXy(&lightData);
	}
	else if (new_scene == 7) {
		this->lightData.bri = 203; this->lightData.x = 0.380328; this->lightData.y = 0.39986; this->lightData.colorMode = COLOR_MODE_XY; convertXy(&lightData);
	}
	else if (new_scene == 8) {
		this->lightData.bri = 112; this->lightData.x = 0.359168; this->lightData.y = 0.28807; this->lightData.colorMode = COLOR_MODE_XY; convertXy(&lightData);
	}
	else if (new_scene == 9) {
		this->lightData.bri = 142; this->lightData.x = 0.267102; this->lightData.y = 0.23755; this->lightData.colorMode = COLOR_MODE_XY; convertXy(&lightData);
	}
	else if (new_scene == 10) {
		this->lightData.bri = 216; this->lightData.x = 0.393209; this->lightData.y = 0.29961; this->lightData.colorMode = COLOR_MODE_XY; convertXy(&lightData);
	}
}

// Shortcut for color conversion method which use directly the
// base members (do no use them when set strip with multiple lights)
//void BaseLight::convert_hue() {
//	convertHue(&lightData);
//}
//void BaseLight::convert_xy() {
//	convertXy(&lightData)
//}
//void BaseLight::convert_ct() {
//	convertCt(&lightData)
//}



void BaseLight::raiseSaveStateRequest()
{
	requireSavingNewState = true;
	latestStateSave = millis();
}

void BaseLight::setLightInFlashMode(LightData* lightData_p, uint8_t lampsCount, uint16_t frequencyMs)
{
	// if frequency is odd transform to even number
	if ( (frequencyMs % 2) != 0){
		frequencyMs++;
	}

	lightData_p->mode = LightMode::Flashing;
	lightData_p->toDoFlashesLeft = lampsCount;
	lightData_p->flashFreqMs = frequencyMs;
}

void BaseLight::setLightInNormalMode(LightData* lightData_p) {
	lightData_p->mode = LightMode::Normal;
	lightData_p->lastFlash = 0;
	lightData_p->toDoFlashesLeft = 0;
	lightData_p->flashFreqMs = 0;
}

void BaseLight::handleFlashMode(LightData* lightData_p) {
	bool isFirstFlash = lightData_p->lastFlash == 0;
	if (isFirstFlash) {
		// before lamp start flashing for the first time
		// save lamp state so it can be restored when flashing ends
		saveState();
	}

	// if there're still to do flashes left (or if finished but light is still ON from the last flash)
	if (lightData_p->toDoFlashesLeft > 0 || this->isLightOn() == true) {
		// check last execution 
		// NOTE: lamp flash is obtained turn it ON and OFF. So if lamp frequency is 1Hz the code below
		// will be execute 2 times per seconds (2Hs): one for turning ON and one for turn OFF

		uint16_t lampFreqDividedMs = lightData_p->flashFreqMs / 2; // eg. 1 sec --> lamp freq is 2 seconds
		if ((millis() - lightData_p->lastFlash) > lampFreqDividedMs) {

			// turn the lamp ON if it is OFF or this is the first flash
			if (this->isLightOn() == false || isFirstFlash == true) {
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
				setLightOn();
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
				setLightOff();
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



// Handle the 'alert' request
// (tipically it will flash the light)
void BaseLight::handleAlert()
{
//    SerialPrintln("[BaseLight] handleAlert() - START");
    this->setLightOn();
    long lastExec = 0;
    bool isOn = false;
    uint8_t i = 0;
    while (i < 1)
    {
        if (millis() - lastExec >= ALERT_FLASH_DURATION_MS)
        {
            if (isOn == false) {
#if defined(LIGHT_TYPE_CCT) || defined(LIGHT_TYPE_RGBCCT)
                this->lightData.colors[COLOR_INDEX_COLD_WHITE] = FLASH_INTENSITY;
#endif
#if defined(LIGHT_TYPE_RGBW)
                this->lightData.colors[COLOR_INDEX_RED] = FLASH_INTENSITY;
                this->lightData.colors[COLOR_INDEX_GREEN] = FLASH_INTENSITY;
                this->lightData.colors[COLOR_INDEX_BLUE] = FLASH_INTENSITY;
#endif
#if defined(LIGHT_TYPE_RGBW)
                this->lightData.colors[COLOR_INDEX_WHITE] = FLASH_INTENSITY;
#endif

                applyColorsImmediatly();
                isOn = true;
                //SerialPrintln("Flash ON");
            }
            else {
                for (uint8_t i = 0; i < CHANNELS_COUNT; i++)
                {
                    this->lightData.colors[i] = 0;
                }
                applyColorsImmediatly();
                isOn = false;
                //SerialPrintln("Flash OFF");

                // Increment by one the ON-OFF cycle counter
                i++;
            }

            lastExec = millis();
        }
        else {
            yield(); // leave space to micro to perform its operations
        }
    }
    this->setLightOff();
}


void BaseLight::handleTurnOnOrOff(boolean isOn)
{
	if (isOn) {
		setLightOn();
	}
	else {
		setLightOff();
	}
	SaveRestoreDataModule::saveLightData( &(this->lightData), 0);
}



// Handle the simple base url '/' request
void BaseLight::srvOnBaseReq(ESP8266WebServer* srvPtr)
{
	float transitionTime = 100;
	if (srvPtr->hasArg("startup")) {
		if (getStartupMode() != srvPtr->arg("startup").toInt()) {
		    // Available stastes are , STARTUP_MODE_OFF, STARTUP_MODE_POWERFAIL, ...
			saveStartupMode(srvPtr->arg("startup").toInt());
		}
	}

	// OLD WAY TO INTERACT WITH LIGHT WEBSERVER PAGE (via GET ?param=value)
	//if (srvPtr->hasArg("scene")) {
	//	if (srvPtr->arg("bri") == "" && srvPtr->arg("hue") == "" && srvPtr->arg("ct") == "" && srvPtr->arg("sat") == "")
	//	{
 //           EEPROM.begin(USED_EEPROM_SIZE);
	//		if (EEPROM.read(EEPROM_ADDR_SCENE) != srvPtr->arg("scene").toInt() && getStartupMode() < STARTUP_MODE_OFF)
	//		{
	//			EEPROM.write(EEPROM_ADDR_SCENE, srvPtr->arg("scene").toInt());
	//			EEPROM.commit();
	//		}
 //           EEPROM.end();

	//		this->applyScene(srvPtr->arg("scene").toInt());
	//	}
	//	else
	//	{
	//		if (srvPtr->arg("bri") != "") {
	//			lightData.bri = (uint8_t)(srvPtr->arg("bri").toInt());
	//		}
	//		if (srvPtr->arg("hue") != "") {
	//			lightData.hue = (uint16_t)(srvPtr->arg("hue").toInt());
	//		}
	//		if (srvPtr->arg("sat") != "") {
	//			lightData.sat = (uint8_t)(srvPtr->arg("sat").toInt());
	//		}
	//		if (srvPtr->arg("ct") != "") {
	//			lightData.ct = (uint16_t)(srvPtr->arg("ct").toInt());
	//		}
	//		if (srvPtr->arg("colormode") == "1" && isLightOn()) {
	//			convertXy(&lightData);
	//		}
	//		else if (srvPtr->arg("colormode") == "2" && isLightOn()) {
	//			convertCt(&lightData);
	//		}
	//		else if (srvPtr->arg("colormode") == "3" && isLightOn()) {
	//			convertHue(&lightData);
	//		}
	//		lightData.colorMode = srvPtr->arg("colormode").toInt();
	//	}
	//}
	//else if (srvPtr->hasArg("on")) {
 //       EEPROM.begin(USED_EEPROM_SIZE);
	//	if (srvPtr->arg("on") == "true") {
	//		setLightOn(); {
	//			if (isOnStartupRestoreLastState() && EEPROM.read(EEPROM_ADDR_IS_ON) != 1) {
	//				EEPROM.write(EEPROM_ADDR_IS_ON, 1);
	//			}
	//		}
	//	}
	//	else {
	//		setLightOff();
	//		if (isOnStartupRestoreLastState() && EEPROM.read(EEPROM_ADDR_IS_ON) != 0) {
	//			EEPROM.write(EEPROM_ADDR_IS_ON, 0);
	//		}
	//	}
	//	EEPROM.commit();
	//}
	//else if (srvPtr->hasArg("alert")) {
	//	this->handleAlert();
	//	/*if (isLightOn()) {
	//		this->lightData.currentColors[0] = 0;
	//		this->lightData.currentColors[1] = 0;
	//		this->lightData.currentColors[2] = 0;
	//		this->lightData.currentColors[3] = 0;
	//	}
	//	else {
	//		this->lightData.currentColors[3] = 255;
	//	}*/
	//}


	//for (uint8_t color = 0; color < CHANNELS_COUNT; color++) {
	//	if (isLightOn()) {
	//		lightData.stepLevel[color] = ((float)this->lightData.colors[color] - this->lightData.currentColors[color]) / transitionTime;
	//	}
	//	else {
	//		lightData.stepLevel[color] = this->lightData.currentColors[color] / transitionTime;
	//	}
	//}


	//if (srvPtr->hasArg("reset")) {
	//	ESP.reset();
	//}

    EEPROM.begin(USED_EEPROM_SIZE);
	String http_content = "<!doctype html>";
	http_content += "<html>";
	http_content += "<head>";
	http_content += "<meta charset=\"utf-8\">";
	http_content += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">";
	http_content += "<title>Light Setup</title>";
	http_content += "<link rel=\"stylesheet\" href=\"https://unpkg.com/purecss@0.6.2/build/pure-min.css\">";
	http_content += "</head>";
	http_content += "<body>";
	http_content += "<fieldset>";
	http_content += "<h3>Light Setup</h3>";
	http_content += "<form class=\"pure-form pure-form-aligned\" action=\"/\" method=\"post\">";
	http_content += "<div class=\"pure-control-group\">";
	http_content += "<label for=\"power\"><strong>Power</strong></label>";
	http_content += "<a class=\"pure-button"; if (isLightOn()) http_content += "  pure-button-primary"; http_content += "\" href=\"/?on=true\">ON</a>";
	http_content += "<a class=\"pure-button"; if (!isLightOn()) http_content += "  pure-button-primary"; http_content += "\" href=\"/?on=false\">OFF</a>";
	http_content += "</div>";
	http_content += "<div class=\"pure-control-group\">";
	http_content += "<label for=\"startup\">Startup</label>";
	http_content += "<select onchange=\"this.form.submit()\" id=\"startup\" name=\"startup\">";
	http_content += "<option "; if (isOnStartupRestoreLastState()) http_content += "selected=\"selected\""; http_content += " value=\"0\">Last state</option>";
	http_content += "<option "; if (isOnStartupWhiteMaxBrightness()) http_content += "selected=\"selected\""; http_content += " value=\"1\">On</option>";
	http_content += "<option "; if (getStartupMode() == 2) http_content += "selected=\"selected\""; http_content += " value=\"2\">Off</option>";
	http_content += "</select>";
	http_content += "</div>";
	http_content += "<div class=\"pure-control-group\">";
	http_content += "<label for=\"scene\">Scene</label>";
	http_content += "<select onchange = \"this.form.submit()\" id=\"scene\" name=\"scene\">";
	http_content += "<option "; if (this->deviceConfig.scene == 0) http_content += "selected=\"selected\""; http_content += " value=\"0\">Relax</option>";
	http_content += "<option "; if (this->deviceConfig.scene == 1) http_content += "selected=\"selected\""; http_content += " value=\"1\">Read</option>";
	http_content += "<option "; if (this->deviceConfig.scene == 2) http_content += "selected=\"selected\""; http_content += " value=\"2\">Concentrate</option>";
	http_content += "<option "; if (this->deviceConfig.scene == 3) http_content += "selected=\"selected\""; http_content += " value=\"3\">Energize</option>";
	http_content += "<option "; if (this->deviceConfig.scene == 4) http_content += "selected=\"selected\""; http_content += " value=\"4\">Bright</option>";
	http_content += "<option "; if (this->deviceConfig.scene == 5) http_content += "selected=\"selected\""; http_content += " value=\"5\">Dimmed</option>";
	http_content += "<option "; if (this->deviceConfig.scene == 6) http_content += "selected=\"selected\""; http_content += " value=\"6\">Nightlight</option>";
	http_content += "<option "; if (this->deviceConfig.scene == 7) http_content += "selected=\"selected\""; http_content += " value=\"7\">Savanna sunset</option>";
	http_content += "<option "; if (this->deviceConfig.scene == 8) http_content += "selected=\"selected\""; http_content += " value=\"8\">Tropical twilight</option>";
	http_content += "<option "; if (this->deviceConfig.scene == 9) http_content += "selected=\"selected\""; http_content += " value=\"9\">Arctic aurora</option>";
	http_content += "<option "; if (this->deviceConfig.scene == 10) http_content += "selected=\"selected\""; http_content += " value=\"10\">Spring blossom</option>";
	http_content += "</select>";
	http_content += "</div>";
	http_content += "<br>";
	http_content += "<div class=\"pure-control-group\">";
	http_content += "<label for=\"state\"><strong>State</strong></label>";
	http_content += "</div>";
	http_content += "<div class=\"pure-control-group\">";
	http_content += "<label for=\"bri\">Bri</label>";
	http_content += "<input id=\"bri\" name=\"bri\" type=\"text\" placeholder=\"" + (String)lightData.bri + "\">";
	http_content += "</div>";
	http_content += "<div class=\"pure-control-group\">";
	http_content += "<label for=\"hue\">Hue</label>";
	http_content += "<input id=\"hue\" name=\"hue\" type=\"text\" placeholder=\"" + (String)lightData.hue + "\">";
	http_content += "</div>";
	http_content += "<div class=\"pure-control-group\">";
	http_content += "<label for=\"sat\">Sat</label>";
	http_content += "<input id=\"sat\" name=\"sat\" type=\"text\" placeholder=\"" + (String)lightData.sat + "\">";
	http_content += "</div>";
	http_content += "<div class=\"pure-control-group\">";
	http_content += "<label for=\"ct\">CT</label>";
	http_content += "<input id=\"ct\" name=\"ct\" type=\"text\" placeholder=\"" + (String)lightData.ct + "\">";
	http_content += "</div>";
	http_content += "<div class=\"pure-control-group\">";
	http_content += "<label for=\"colormode\">Color</label>";
	http_content += "<select id=\"colormode\" name=\"colormode\">";
	http_content += "<option "; if (lightData.colorMode == COLOR_MODE_XY) http_content += "selected=\"selected\""; http_content += " value=\"1\">xy</option>";
	http_content += "<option "; if (lightData.colorMode == COLOR_MODE_CT) http_content += "selected=\"selected\""; http_content += " value=\"2\">ct</option>";
	http_content += "<option "; if (lightData.colorMode == COLOR_MODE_HUE) http_content += "selected=\"selected\""; http_content += " value=\"3\">hue</option>";
	http_content += "</select>";
	http_content += "</div>";
	http_content += "<div class=\"pure-controls\">";
	http_content += "<span class=\"pure-form-message\"><a href=\"/?alert=1\">alert</a> or <a href=\"/?reset=1\">reset</a></span>";
	http_content += "<label for=\"cb\" class=\"pure-checkbox\">";
	http_content += "</label>";
	http_content += "<button type=\"submit\" class=\"pure-button pure-button-primary\">Save</button>";
	http_content += "</div>";
	http_content += "</fieldset>";
	http_content += "</form>";
	http_content += "</body>";
	http_content += "</html>";
    EEPROM.end();

	srvPtr->send(200, "text/html", http_content);
}


// Handle the 'switch/button' request
void BaseLight::srvOnSwitch(ESP8266WebServer* srvPtr) {
	int button;
	for (uint8_t i = 0; i < srvPtr->args(); i++) {
		if (srvPtr->argName(i) == "button") {
			button = srvPtr->arg(i).toInt();
		}
	}
	if (button == 1000) {
		if (isLightOn() == false) {
			this->setLightOn();
			deviceConfig.scene = 0;
		}
		else {
			this->applyScene(deviceConfig.scene);
            deviceConfig.scene++;
			if (deviceConfig.scene == 11) {
                deviceConfig.scene = 0;
			}
		}
	}
	else if (button == 2000) {
		if (isLightOn() == false) {
			this->lightData.bri = 30;
			this->setLightOn();
		}
		else {
			this->lightData.bri += 30;
		}
		if (this->lightData.bri > 255) this->lightData.bri = 255;
		if (this->lightData.colorMode == COLOR_MODE_XY) {
			convertXy(&lightData);
		}
		else if (this->lightData.colorMode == COLOR_MODE_CT) {
			convertCt(&lightData);
		}
		else if (this->lightData.colorMode == COLOR_MODE_HUE) {
			convertHue(&lightData);
		}
	}
	else if (button == 3000 && this->isLightOn()) {
		this->lightData.bri -= 30;
		if (this->lightData.bri < 1)
			this->lightData.bri = 1;
		else {
			if (this->lightData.colorMode == COLOR_MODE_XY) {
				convertXy(&lightData);
			}
			else if (this->lightData.colorMode == COLOR_MODE_CT) {
				convertCt(&lightData);
			}
			else if (this->lightData.colorMode == COLOR_MODE_HUE) {
				convertHue(&lightData);
			}
		}
	}
	else if (button == 4000) {
		this->setLightOff();
	}

	// Platform specific? (+)
	for (uint8_t color = 0; color < this->getColorChannelsCount(); color++) {
		if (isLightOn()) {
			this->lightData.stepLevel[color] =
				(this->lightData.colors[color] - this->lightData.currentColors[color]) / 54;
		}
		else {
			this->lightData.stepLevel[color] = this->lightData.currentColors[color] / 54;
		}
	}
	// Platform specific? (-)
}


// Handle request to set hue/brightnes/etc..
void BaseLight::srvOnSet(ESP8266WebServer* srvPtr)
{
	setLightOn();
	float transitionTime = 4;
	for (uint8_t i = 0; i < srvPtr->args(); i++)
	{
		/*Serial.print("Reading param ");
		Serial.print(i, DEC);
		Serial.print(":");
		Serial.println(srvPtr->argName(i));*/

		if (srvPtr->argName(i) == "on")
		{
			boolean isOn = (srvPtr->arg(i) == "True" || srvPtr->arg(i) == "true");
			handleTurnOnOrOff(isOn);
		}
		else if (srvPtr->argName(i) == "r") {
			this->lightData.colors[0] = srvPtr->arg(i).toInt();
			this->lightData.colorMode = 0;
		}
		else if (srvPtr->argName(i) == "g") {
			this->lightData.colors[1] = srvPtr->arg(i).toInt();
			this->lightData.colorMode = 0;
		}
		else if (srvPtr->argName(i) == "b") {
			this->lightData.colors[2] = srvPtr->arg(i).toInt();
			this->lightData.colorMode = 0;
		}
		else if (srvPtr->argName(i) == "w") {
			this->lightData.colors[3] = srvPtr->arg(i).toInt();
			this->lightData.colorMode = 0;
		}
		else if (srvPtr->argName(i) == "x") {
			lightData.x = srvPtr->arg(i).toFloat();
			this->lightData.colorMode = COLOR_MODE_XY;
		}
		else if (srvPtr->argName(i) == "y") {
			lightData.y = srvPtr->arg(i).toFloat();
			this->lightData.colorMode = COLOR_MODE_XY;
		}
		else if (srvPtr->argName(i) == "bri") {
			if (srvPtr->arg(i).toInt() != 0)
				this->lightData.bri = srvPtr->arg(i).toInt();
		}
		else if (srvPtr->argName(i) == "bri_inc") {
			this->lightData.bri += srvPtr->arg(i).toInt();
			if (this->lightData.bri > 255) this->lightData.bri = 255;
			else if (this->lightData.bri < 0) this->lightData.bri = 0;
		}
		else if (srvPtr->argName(i) == "ct") {
			this->lightData.ct = srvPtr->arg(i).toInt();
			this->lightData.colorMode = COLOR_MODE_CT;
		}
		else if (srvPtr->argName(i) == "sat") {
			this->lightData.sat = srvPtr->arg(i).toInt();
			this->lightData.colorMode = COLOR_MODE_HUE;
		}
		else if (srvPtr->argName(i) == "hue") {
			this->lightData.hue = srvPtr->arg(i).toInt();
			this->lightData.colorMode = COLOR_MODE_HUE;
		}
		else if (srvPtr->argName(i) == "alert" && srvPtr->arg(i) == "select") {
			this->handleAlert();
		}
		else if (srvPtr->argName(i) == "transitiontime") {
			transitionTime = srvPtr->arg(i).toInt();
		}
	}
	srvPtr->send(200, "text/plain", "OK, x: "
		+ (String)lightData.x
		+ ", y:" + (String)lightData.y
		+ ", bri:" + (String)this->lightData.bri
		+ ", ct:" + this->lightData.ct
		+ ", colormode:" + this->lightData.colorMode
		+ ", state:" + lightData.lightState);

	processLightData(transitionTime);


	//printDebug();
	//printColorsArray();
	//printStepLevelArray();

	raiseSaveStateRequest();

}


void BaseLight::srvOnStateGet(ESP8266WebServer* srvPtr)
{
	//uint8_t light = server.arg("light").toInt() - 1; // only for multilight

	DynamicJsonDocument  jsonDoc(200); // calculated with https://arduinojson.org/v6/assistant/
	jsonDoc["on"] = isLightOn();
	jsonDoc["bri"] = lightData.bri;

	JsonArray xy = jsonDoc.createNestedArray("xy");
	xy.add(lightData.x);
	xy.add(lightData.y);
	jsonDoc["ct"] = lightData.ct;
	jsonDoc["hue"] = lightData.hue;
	jsonDoc["sat"] = lightData.sat;
	if (lightData.colorMode == COLOR_MODE_XY)
		jsonDoc["colormode"] = "xy";
	else if (lightData.colorMode == COLOR_MODE_CT)
		jsonDoc["colormode"] = "ct";
	else if (lightData.colorMode == COLOR_MODE_HUE)
		jsonDoc["colormode"] = "hs";
	String output;
	serializeJson(jsonDoc, output);

	srvPtr->send(200, "text/plain", output);
}

// TODO: use this base from original project for multiple lights
//void BaseLight::srvOnStatePut(ESP8266WebServer* srvPtr) {
//	bool stateSave = false;
//	DynamicJsonDocument root(1024);
//	DeserializationError error = deserializeJson(root, server.arg("plain"));
//
//	if (error) {
//		server.send(404, "text/plain", "FAIL. " + server.arg("plain"));
//	}
//	else {
//		for (JsonPair state : root.as<JsonObject>()) {
//			const char* key = state.key().c_str();
//			int light = atoi(key) - 1;
//			JsonObject values = state.value();
//			int transitiontime = 4;
//
//			if (values.containsKey("xy")) {
//				lights[light].x = values["xy"][0];
//				lights[light].y = values["xy"][1];
//				lights[light].colorMode = 1;
//			}
//			else if (values.containsKey("ct")) {
//				lights[light].ct = values["ct"];
//				lights[light].colorMode = 2;
//			}
//			else {
//				if (values.containsKey("hue")) {
//					lights[light].hue = values["hue"];
//					lights[light].colorMode = 3;
//				}
//				if (values.containsKey("sat")) {
//					lights[light].sat = values["sat"];
//					lights[light].colorMode = 3;
//				}
//			}
//
//			if (values.containsKey("on")) {
//				if (values["on"]) {
//					lights[light].lightState = true;
//				}
//				else {
//					lights[light].lightState = false;
//				}
//				if (startup == 0) {
//					stateSave = true;
//				}
//			}
//
//			if (values.containsKey("bri")) {
//				lights[light].bri = values["bri"];
//			}
//
//			if (values.containsKey("bri_inc")) {
//				lights[light].bri += (int)values["bri_inc"];
//				if (lights[light].bri > 255) lights[light].bri = 255;
//				else if (lights[light].bri < 1) lights[light].bri = 1;
//			}
//
//			if (values.containsKey("transitiontime")) {
//				transitiontime = values["transitiontime"];
//			}
//
//			if (values.containsKey("alert") && values["alert"] == "select") {
//				if (lights[light].lightState) {
//					lights[light].currentColors[0] = 0; lights[light].currentColors[1] = 0; lights[light].currentColors[2] = 0; lights[light].currentColors[3] = 0;
//				}
//				else {
//					lights[light].currentColors[3] = 126; lights[light].currentColors[4] = 126;
//				}
//			}
//			processLightdata(light, transitiontime);
//		}
//		String output;
//		serializeJson(root, output);
//		server.send(200, "text/plain", output);
//		if (stateSave) {
//			saveState();
//		}
//	}
//}


void BaseLight::srvOnStatePut(ESP8266WebServer* srvPtr) {
	bool stateSave = false;
	DynamicJsonDocument root(1024);
	DeserializationError error = deserializeJson(root, srvPtr->arg("plain"));

	if (error) {
		srvPtr->send(404, "text/plain", "FAIL. " + srvPtr->arg("plain"));
	}
	else {
        JsonObject values = root.as<JsonObject>();
		int transitiontime = 4;

		if (values.containsKey("xy")) {
//		    SerialPrint("Set XY (size");
            JsonArray xy = values["xy"];
            //SerialPrintlnWithFormat(xy.size(), DEC);
			lightData.x = xy[0];
			lightData.y = xy[1];
			lightData.colorMode = COLOR_MODE_XY;
		}
		else if (values.containsKey("ct")) {
            SerialPrintln("Set CT");
			lightData.ct = values["ct"];
			lightData.colorMode = COLOR_MODE_CT;
		}
		else {
			if (values.containsKey("hue")) {
                SerialPrintln("Set HUE");
				lightData.hue = values["hue"];
				lightData.colorMode = COLOR_MODE_HUE;
			}
			if (values.containsKey("sat")) {
                SerialPrintln("Set SAT");
				lightData.sat = values["sat"];
				lightData.colorMode = COLOR_MODE_HUE;
			}
		}

		if (values.containsKey("on")) {
            handleTurnOnOrOff(values["on"]);
            if (isOnStartupRestoreLastState() == true) {
                stateSave = true;
            }
		}

		if (values.containsKey("bri")) {
			lightData.bri = values["bri"];
		}

		if (values.containsKey("bri_inc")) {
			lightData.bri += (int)values["bri_inc"];
			if (lightData.bri > 255) lightData.bri = 255;
			else if (lightData.bri < 1) lightData.bri = 1;
		}

		if (values.containsKey("transitiontime")) {
			// Transition Time is given in 100ms units (eg 50 equal to 5 seconds)
			transitiontime = values["transitiontime"];
			SerialPrint("[BaseLight::srvOnStatePut] Set transitiontime="); SerialPrintWithFormat(transitiontime, DEC);
			SerialPrint("-->"); SerialPrintWithFormat(transitiontime/10, DEC); SerialPrint("s");
			SerialPrintln();
			SerialPrint("Transition time has been set at: "); SerialPrintWithFormat(millis(), DEC); SerialPrint("ms");
			SerialPrintln();
		}

		// request contains info about alerts (flashing light single or multiple times)
		if ( values.containsKey("alert") ) {
			if (values["alert"] == ALERT_SINGLE_FLASH) {
				this->setLightInFlashMode(&(this->lightData), ALERT_SINGLE_FLASH_FLASHES_COUNT, ALERT_SINGLE_FLASH_FLASHES_FREQUENCY_MS);
				//handleAlert();
			}
			else if (values["alert"] == ALERT_MULTIPLE_FLASH) {
				this->setLightInFlashMode(&(this->lightData), ALERT_MULTIPLE_FLASH_FLASHES_COUNT, ALERT_MULTIPLE_FLASH_FLASHES_FREQUENCY_MS);
			}
			else {
				this->setLightInNormalMode(&(this->lightData));
			}
		}


		processLightData(transitiontime);

		String output;
		serializeJson(root, output);
		srvPtr->send(200, "text/plain", output);


//		if (stateSave) {
//			saveState();
//		}
        raiseSaveStateRequest();
	}
}



//void BaseLight::srvOnStatePut(ESP8266WebServer* srvPtr){
//    bool stateSave = false;
//
//	// Size calculated with: https://arduinojson.org/v6/assistant/
//	DynamicJsonDocument  values(200);
//	DeserializationError err = deserializeJson(values, srvPtr->arg("plain"));
//	if (err != DeserializationError::Ok) {
//        srvPtr->send(404, "text/plain", "FAIL. " + srvPtr->arg("plain") + ". Json error code:" + err.c_str());
//	}
//	else {
//        // The color modes are mutual-exclusives
//	    if( values.containsKey("xy") ){
//            lightData.x = values["xy"][0];
//            lightData.y = values["xy"][1];
//            lightData.colorMode = 1;
//	    }
//        else if( values.containsKey("ct") ){
//            lightData.ct = values["ct"];
//            lightData.colorMode = 2;
//        }else {
//            if( values.containsKey("hue")){
//                lightData.hue = values["hue"];
//                lightData.colorMode = 3;
//            }
//            if( values.containsKey("sat")){
//                lightData.hue = values["sat"];
//                lightData.colorMode = 3;
//            }
//        }
//
//        if (values.containsKey("on")) {
//            handleTurnOnOrOff(values["on"]);
//            if (isOnStartupRestoreLastState() == true) {
//                stateSave = true;
//            }
//        }
//
//        if (values.containsKey("bri")) {
//            lightData.bri = values["bri"];
//        }
//        if (values.containsKey("bri_inc")) {
//            lightData.bri += (int)values["bri_inc"];
//            if (lightData.bri > 255) {
//                lightData.bri = 255;
//            }else if (lightData.bri < 1){
//                lightData.bri = 1;
//            }
//        }
//
//        if (values.containsKey("transitiontime")) {
//            transitiontime = values["transitiontime"];
//        }
//
//        if (values.containsKey("alert") && values["alert"] == "select") {
//            this->handleAlert();
//        }
//        processLightData(transitiontime);
//
//        String output;
//        serializeJson(values, output);
//        srvPtr->send(200, "text/plain", output);
//
//		if (stateSave) {
//			saveState();
//		}
//	}
//}


void BaseLight::srvOnConfigGet(ESP8266WebServer* srvPtr)
{
	DynamicJsonDocument  jsonDoc(400); // calculated with https://arduinojson.org/v6/assistant/
	jsonDoc["name"] = deviceConfig.getLightName();
	jsonDoc["scene"] = deviceConfig.scene;
	jsonDoc["startup"] = getStartupMode();
	//jsonDoc["hw"] = hwSwitch;
	//jsonDoc["on"] = onPin;
	//jsonDoc["off"] = offPin;
	//jsonDoc["hwswitch"] = (int)hwSwitch;
	jsonDoc["lightscount"] = 1;

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
	char ipBuff[IP_AS_STR_MAX_LENGTH] = {'\0'};
    char gatwBuff[IP_AS_STR_MAX_LENGTH] = {'\0'};
    char sbnmskBuf[IP_AS_STR_MAX_LENGTH] = {'\0'};
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



void BaseLight::processLightData(float transitionTime)
{
	if (this->lightData.colorMode == COLOR_MODE_XY && this->isLightOn()) {
		convertXy(&lightData);
	}
	else if (this->lightData.colorMode == COLOR_MODE_CT && this->isLightOn()) {
		convertCt(&lightData);
	}
	else if (this->lightData.colorMode == COLOR_MODE_HUE && this->isLightOn()) {
		convertHue(&lightData);
	}

	transitionTime *= 16;
	for (uint8_t color = 0; color < CHANNELS_COUNT; color++) {
		if (isLightOn()) {
			lightData.stepLevel[color] = (this->lightData.colors[color] - this->lightData.currentColors[color]) / transitionTime;
		}
		else {
			lightData.stepLevel[color] = this->lightData.currentColors[color] / transitionTime;
		}
	}
}


void BaseLight::srvOnGet(ESP8266WebServer* srvPtr)
{
	String colorModeStr;
	String power_status;
	power_status = isLightOn() ? "true" : "false";
	if (lightData.colorMode == COLOR_MODE_XY)
		colorModeStr = "xy";
	else if (lightData.colorMode == COLOR_MODE_CT)
		colorModeStr = "ct";
	else if (lightData.colorMode == COLOR_MODE_HUE)
		colorModeStr = "hs";

	srvPtr->send(
		200,
		"text/plain",
		"{\"on\": " + power_status
		+ ", \"bri\": " + (String)lightData.bri
		+ ", \"xy\": [" + (String)lightData.x + ", " + (String)lightData.y + "], "
		+ "\"ct\":" + (String)lightData.ct
		+ ", \"sat\": " + (String)lightData.sat
		+ ", \"hue\": " + (String)lightData.hue
		+ ", \"colormode\": \"" + colorModeStr
		+ "\"}");
}


void BaseLight::srvOnDetect(ESP8266WebServer* srvPtr, byte* macPtr)
{
	/*srvPtr->send(200, "text/plain", "{\"hue\": \"bulb\",\"lights\": 1,\"modelid\": \"" + this->getModelId()
		+ "\",\"mac\": \"" + String( *(macPtr), HEX)
		+ ":" + String( *(macPtr + 1), HEX)
		+ ":" + String( *(macPtr + 2), HEX)
		+ ":" + String( *(macPtr + 3), HEX)
		+ ":" + String( *(macPtr + 4), HEX)
		+ ":" + String( *(macPtr + 5), HEX)
		+ "\"}"
	);*/

	char macString[32] = { 0 };
	sprintf(macString, "%02X:%02X:%02X:%02X:%02X:%02X", *(macPtr), *(macPtr + 1), *(macPtr + 2), *(macPtr + 3), *(macPtr + 4), *(macPtr + 5));

#if ARDUINOJSON_VERSION_MAJOR >= 6
	DynamicJsonDocument  root(200); // calculated with https://arduinojson.org/v6/assistant/
#else
	DynamicJsonBuffer newBuffer;
	JsonObject& root = newBuffer.createObject();
#endif

#ifdef USE_LEGACY_PROTOCOL
	// {"hue": "bulb","lights": 1,"modelid": "LCT015","mac": "cc:50:e3:50:38:d1"}
	root["hue"] = "bulb";
	root["lights"] = 1;
	root["modelid"] = this->getModelId();
	root["mac"] = String(macString);
#else
	root["name"] = deviceConfig.getLightName();
	root["lights"] = 1;
	root["protocol"] = "native_single";
	root["modelid"] = this->getModelId();
	root["type"] = this->getType();
	root["mac"] = String(macString);
	root["version"] = LIGHT_VERSION;
#endif // USE_LEGACY_PROTOCOL

	String output;
	serializeJson(root, output);
	srvPtr->send(200, "text/plain", output);

}


void BaseLight::notifyNotConnected()
{
	// Turn on the light, flash 3 times red, then turn off
	this->setLightOn();
	long lastExec = millis();
	bool isOn = false;
	uint8_t i = 0;
	while (i < NO_CONNECTION_FLASH_LOOPS_COUNT)
	{
		if (millis() - lastExec >= NO_CONNECTION_FLASH_DURATION_MS)
		{
			if (isOn == false) {
#if defined(LIGHT_TYPE_CCT) || defined(LIGHT_TYPE_RGBCCT)
				this->lightData.colors[COLOR_INDEX_COLD_WHITE] = FLASH_INTENSITY;
#endif
#if defined(LIGHT_TYPE_RGBW)
				this->lightData.colors[COLOR_INDEX_RED] = FLASH_INTENSITY;
				this->lightData.colors[COLOR_INDEX_GREEN] = FLASH_INTENSITY;
				this->lightData.colors[COLOR_INDEX_BLUE] = FLASH_INTENSITY;
#endif
#if defined(LIGHT_TYPE_RGBW)
				this->lightData.colors[COLOR_INDEX_WHITE] = FLASH_INTENSITY;
#endif

				applyColorsImmediatly();
				isOn = true;
				//SerialPrintln("Flash ON");
			}
			else {
				for (uint8_t i = 0; i < CHANNELS_COUNT; i++)
				{
					this->lightData.colors[i] = 0;
				}
				applyColorsImmediatly();
				isOn = false;
				//SerialPrintln("Flash OFF");

				// Increment by one the ON-OFF cycle counter
				i++;
			}

			lastExec = millis();
		}
		else {
			yield(); // leave space to micro to perform its operations
		}
	}
	this->setLightOff();

}


void BaseLight::printDebug()
{
	SerialPrint("hue:"); SerialPrintlnWithFormat(this->lightData.hue, DEC);
	SerialPrint("bri:"); SerialPrintlnWithFormat(this->lightData.bri, DEC);
	SerialPrint("sat:"); SerialPrintlnWithFormat(this->lightData.sat, DEC);
	SerialPrint("x:"); SerialPrintlnWithFormat(lightData.x, DEC);
	SerialPrint("y:"); SerialPrintlnWithFormat(lightData.y, DEC);
	SerialPrint("lightData.colorMode:"); SerialPrintlnWithFormat(this->lightData.colorMode, DEC);
	SerialPrint("light_state:"); SerialPrintlnWithFormat(isLightOn(), DEC);
}


bool reached;
void BaseLight::run(bool isConnected)
{
	if (isConnected == true || hasBeenNoConnectionNotifyFired == true)
	{
		if (this->lightData.mode == LightMode::Flashing && this->lightData.flashFreqMs > 0) {
			this->handleFlashMode(&(this->lightData));
		}
		else {
			this->lightEngine();

			// DEBUG to check if transition is performed in correct time
			/*for (uint8_t color = 0; color < CHANNELS_COUNT; color++)
			{
				if (this->lightData.currentColors[color] == this->lightData.colors[color]) {
					if (reached == false) {
						Serial.print("Color ["); Serial.print(color, DEC);
						Serial.print("] --> reached the target color at:"); Serial.print(millis(), DEC); Serial.print("ms");
						Serial.println();
						reached = true;
					}
				}
				else {
					reached = false;
				}
			}*/

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
	 //   // temporarily save current lamp data
	 //   LightData tmp;
	 //   copyLightData(&lightData, &tmp);
  //      // notify the 'not connected state'
  //      //Serial.println("[BaseLight] Notify non connected start");
		//notifyNotConnected();
  //      hasBeenNoConnectionNotifyFired = true;
  //      //Serial.println("[BaseLight] Notify non connected end");
  //      // then restore the saved state
  //      copyLightData(&tmp, &lightData);
  //      this->lightEngine();

		this->setLightInFlashMode( &(this->lightData), 20, 2000);
		hasBeenNoConnectionNotifyFired = true;
	}


}