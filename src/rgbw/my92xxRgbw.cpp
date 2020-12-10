#include "my92xxRgbw.h"
#include <my92xx.h>


#ifdef  MY9291_RGBW

//my92xx * _my92xx;

void Light::initPlatformSpecific()
{
	// Init LED driver
	PWM_PINS[0] = RED_CHANNEL;
	PWM_PINS[1] = GREEN_CHANNEL;
	PWM_PINS[2] = BLUE_CHANNEL;
	PWM_PINS[3] = WHITE_CHANNEL;

	this->_my92xx = new my92xx(MY92XX_MODEL, MY92XX_CHIPS, MY92XX_DI_PIN, MY92XX_DCKI_PIN, MY92XX_COMMAND_DEFAULT);
	this->_my92xx->setState(true);
}

String Light::getModelId()
{
	return LIGHT_MODEL_RGB_AND_WHITE;
}

String Light::getType()
{
	return "rgbw";
}

// Method to handle LEDs state
// It is stricly dependant by platform/hardware (eg. led driver ic, etc)
void Light::lightEngine() 
{

    for (uint8_t color = 0; color < CHANNELS_COUNT; color++)
    {
        if (this->isLightOn())
        {
            if (this->lightData.colors[color] != this->lightData.currentColors[color] )
            {
                this->in_transition = true;
                this->lightData.currentColors[color] += lightData.stepLevel[color];
                if ( (lightData.stepLevel[color] > 0.0f && this->lightData.currentColors[color] > this->lightData.colors[color]) 
					|| (lightData.stepLevel[color] < 0.0f && this->lightData.currentColors[color] < this->lightData.colors[color])){
                     this->lightData.currentColors[color] = this->lightData.colors[color];
                }
                
				if (_my92xx != NULL) {
					_my92xx->setChannel(this->PWM_PINS[color], this->lightData.currentColors[color]);
					_my92xx->update();
				}
				else {
					Serial.println("WARNING: The ptr to my92xx is null");
					delay(2000);
				}
            }
        } 
        else 
		{
            if (this->lightData.currentColors[color] != 0)
            {
                this->in_transition = true;
                this->lightData.currentColors[color] -= lightData.stepLevel[color];
                if (this->lightData.currentColors[color] < 0.0f){
                    this->lightData.currentColors[color] = 0;
                }
                
				if (_my92xx != NULL) {
					_my92xx->setChannel(PWM_PINS[color], this->lightData.currentColors[color]);
					_my92xx->update();
				}
				else {
					Serial.println("WARNING: The ptr to my92xx is null");
					delay(2000);
				}

                
            }
        }
    }

    // Wait a few millis to allow led driver to update
    if (this->in_transition == true) 
    {
        delay(6);
        this->in_transition = false;
    }
}

void Light::applyColorsImmediatly()
{
	for (uint8_t color = 0; color < CHANNELS_COUNT; color++)
	{
		if (this->isLightOn())
		{
			// If the lamp is ON
			this->lightData.currentColors[color] = this->lightData.colors[color];
			_my92xx->setChannel(PWM_PINS[color], this->lightData.currentColors[color]);
			_my92xx->update();
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
//	long lastExec = millis();
//	bool isOn = false;
//	uint8_t i = 0;
//	while (i < 3)
//	{
//		if (millis() - lastExec > 1000)
//		{
//			if (isOn == false) {
//				this->lightData.colors[COLOR_INDEX_RED]	= 0;
//				this->lightData.colors[COLOR_INDEX_GREEN] = 0;
//				this->lightData.colors[COLOR_INDEX_BLUE]	= 0;
//				this->lightData.colors[COLOR_INDEX_WHITE] = MAX_BRIGHTNESS / 2;
//				applyColorsImmediatly();
//				isOn = true;
//				//Serial.println("Flash ON");
//			}
//			else {
//				this->lightData.colors[COLOR_INDEX_RED]	= 0;
//				this->lightData.colors[COLOR_INDEX_GREEN] = 0;
//				this->lightData.colors[COLOR_INDEX_BLUE]	= 0;
//				this->lightData.colors[COLOR_INDEX_WHITE] = 0;
//				applyColorsImmediatly();
//				isOn = false;
//				//Serial.println("Flash OFF");
//
//				// Increment by one the ON-OFF cycle counter
//				i++;
//			}
//
//			lastExec = millis();
//		}
//		else {
//			yield(); // leave space to micro to perform its operations
//		}
//	}
//	this->setLightOff();
//}
#endif //  MY9291_RGBW