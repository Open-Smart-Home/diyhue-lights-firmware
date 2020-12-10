#include "pwmCct.h"


#ifdef  PWM_CT_TYWE3S


void Light::initPlatformSpecific()
{
	// Init LED driver
	pwmPins[COLOR_INDEX_WARM_WHITE] = PWM_WARM_WHITE;
	pwmPins[COLOR_INDEX_COLD_WHITE] = PWM_COOL_WHITE;


	for (uint8_t pin = 0; pin < CHANNELS_COUNT; pin++)
	{
		pinMode(pwmPins[pin], OUTPUT);
		analogWrite(pwmPins[pin], 0);
	}
}

String Light::getModelId()
{
	return "LTW001";
}


void Light::convert_ct(uint8_t* rgbw_p, int ct, int bri)
{
	int optimal_bri = int(10 + bri / 1.04);

	uint8 percent_warm = ((ct - 150) * 100) / 350;

	rgbw_p[COLOR_INDEX_WARM_WHITE] = (optimal_bri * percent_warm) / 100;
	rgbw_p[COLOR_INDEX_COLD_WHITE] = (optimal_bri * (100 - percent_warm)) / 100;
}

// Method to handle LEDs state
// It is stricly dependant by platform/hardware (eg. led driver ic, etc)
void Light::lightEngine()
{
	for (uint8_t color = 0; color < CHANNELS_COUNT; color++) {
		if (isLightOn()) {
			if (colors[color] != lightData.currentColors[color]) {
				in_transition = true;
				lightData.currentColors[color] += lightData.stepLevel[color];
				if ((lightData.stepLevel[color] > 0.0f && lightData.currentColors[color] > colors[color]) || (lightData.stepLevel[color] < 0.0f && lightData.currentColors[color] < colors[color])) lightData.currentColors[color] = colors[color];
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
	//else if (use_hardware_switch == true) {
	//	if (digitalRead(button1_pin) == HIGH) {
	//		int i = 0;
	//		while (digitalRead(button1_pin) == HIGH && i < 30) {
	//			delay(20);
	//			i++;
	//		}
	//		if (i < 30) {
	//			// there was a short press
	//			setLightOn();
	//		}
	//		else {
	//			// there was a long press
	//			bri += 56;
	//			if (bri > 254) {
	//				// don't increase the brightness more then maximum value
	//				bri = 254;
	//			}
	//		}
	//		process_lightdata(4);
	//	}
	//	else if (digitalRead(button2_pin) == HIGH) {
	//		int i = 0;
	//		while (digitalRead(button2_pin) == HIGH && i < 30) {
	//			delay(20);
	//			i++;
	//		}
	//		if (i < 30) {
	//			// there was a short press
	//			setLightOff();
	//		}
	//		else {
	//			// there was a long press
	//			bri -= 56;
	//			if (bri < 1) {
	//				// don't decrease the brightness less than minimum value.
	//				bri = 1;
	//			}
	//		}
	//		process_lightdata(4);
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
			analogWrite(pwmPins[color], (int)(lightData.currentColors[color]));
		}
	}
}

void Light::notifyNotConnected()
{
	//this->lightData.colorMode == COLOR_MODE_XY;
	//this->lightData.bri = MAX_BRIGHTNESS / 2;

	// Turn on the light, flash 3 times then set default
	this->setLightOn();
	for (uint8_t i = 0; i < 3; i++)
	{
		this->lightData.colors[COLOR_INDEX_WARM_WHITE] = MAX_BRIGHTNESS / 2;
		this->lightData.colors[COLOR_INDEX_COLD_WHITE] = MAX_BRIGHTNESS / 2;
		applyColorsImmediatly();
		delay(800);
		this->lightData.colors[0] = 0;
		this->lightData.colors[1] = 0;
		applyColorsImmediatly();
		delay(800);
	}
	this->setLightOff();
}
#endif //  PWM_CT_TYWE3S