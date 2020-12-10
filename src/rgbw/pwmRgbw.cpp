#include "pwmRgbw.h"


#ifdef  PWM_RGBW_ESP8285

//my92xx * _my92xx;

void Light::initPlatformSpecific()
{
	// Init LED driver
	pwmPins[0] = PWM_RED;
	pwmPins[1] = PWM_GREEN;
	pwmPins[2] = PWM_BLUE;
	pwmPins[3] = PWM_WHITE;

	for (uint8_t pin = 0; pin < CHANNELS_COUNT; pin++)
	{
		pinMode(pwmPins[pin], OUTPUT);
		analogWrite(pwmPins[pin], 0);
	}
}

String Light::getModelId()
{
	return "LCT015";
}

// Method to handle LEDs state
// It is stricly dependant by platform/hardware (eg. led driver ic, etc)
void Light::lightEngine()
{
	for (uint8_t color = 0; color < CHANNELS_COUNT; color++) {
		if (isLightOn()) {
			if (lightData.colors[color] !=  lightData.currentColors[color]) {
				in_transition = true;
				lightData.currentColors[color] += lightData.stepLevel[color];
				if ( (lightData.stepLevel[color] > 0.0f && lightData.currentColors[color] > lightData.colors[color]) || (lightData.stepLevel[color] < 0.0f && lightData.currentColors[color] < lightData.colors[color]) ){
				    lightData.currentColors[color] = lightData.colors[color];
				}
				analogWrite(pwmPins[color], (int)(lightData.currentColors[color] * 4.0));
			}
		}
		else {
			if (lightData.currentColors[color] != 0) {
				in_transition = true;
				lightData.currentColors[color] -= lightData.stepLevel[color];
				if (lightData.currentColors[color] < 0.0f) lightData.currentColors[color] = 0;
				analogWrite(pwmPins[color], (int)(lightData.currentColors[color] * 4.0));
			}
		}
	}
	if (in_transition) {
		delay(6);
		in_transition = false;
	}
	//else if (hwSwitch == true) {
	//	if (digitalRead(onPin) == HIGH) {
	//		int i = 0;
	//		while (digitalRead(onPin) == HIGH && i < 30) {
	//			delay(20);
	//			i++;
	//		}
	//		if (i < 30) {
	//			// there was a short press
	//			light.lightState = true;
	//		}
	//		else {
	//			// there was a long press
	//			light.bri += 56;
	//			if (light.bri > 254) {
	//				// don't increase the brightness more then maximum value
	//				light.bri = 254;
	//			}
	//		}
	//		processLightdata(4);
	//	}
	//	else if (digitalRead(offPin) == HIGH) {
	//		int i = 0;
	//		while (digitalRead(offPin) == HIGH && i < 30) {
	//			delay(20);
	//			i++;
	//		}
	//		if (i < 30) {
	//			// there was a short press
	//			light.lightState = false;
	//		}
	//		else {
	//			// there was a long press
	//			light.bri -= 56;
	//			if (light.bri < 1) {
	//				// don't decrease the brightness less than minimum value.
	//				light.bri = 1;
	//			}
	//		}
	//		processLightdata(4);
	//	}
	//}
		
}

void Light::applyColorsImmediatly()
{
	for (uint8_t color = 0; color < CHANNELS_COUNT; color++)
	{
		if (this->isLightOn())
		{
			// If the lamp is ON
			this->lightData.currentColors[color] = this->lightData.colors[color];
			analogWrite( pwmPins[color], (int)(lightData.currentColors[color]) );
		}
	}
}

//void Light::notifyNotConnected()
//{
//	//this->lightData.colorMode == COLOR_MODE_XY;
//	//this->lightData.bri = MAX_BRIGHTNESS / 2;
//
//	// Turn on the light, flash 3 times red, then turn off
//	this->setLightOn();
//	for (uint8_t i = 0; i < 3; i++)
//	{
//		this->lightData.colors[0] = MAX_BRIGHTNESS / 2;
//		this->lightData.colors[1] = 0;
//		this->lightData.colors[2] = 0;
//		this->lightData.colors[3] = 0;
//		applyColorsImmediatly();
//		delay(800);
//		this->lightData.colors[0] = 0;
//		this->lightData.colors[1] = 0;
//		this->lightData.colors[2] = 0;
//		this->lightData.colors[3] = 0;
//		applyColorsImmediatly();
//		delay(800);
//	}
//	this->setLightOff();
//}
#endif //  MY9291_RGBW