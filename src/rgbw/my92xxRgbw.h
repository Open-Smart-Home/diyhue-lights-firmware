
#ifndef LIGHT_H
#define LIGHT_H



#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <EEPROM.h>
#include <my92xx.h>
#include "../BaseLight.h"
#include "../internal_cfg.h"


//-------------------------------------------------
// Very often you can leave these settings as is
// If you facing any issue you can investigate
// using the tutorial/guide on the site
#define RED_CHANNEL     0
#define GREEN_CHANNEL   1
#define BLUE_CHANNEL    2
#define WHITE_CHANNEL   3

#define MTCK_PIN    13 //D7
#define MTDO_PIN    15 //D8

#define MY92XX_MODEL        MY92XX_MODEL_MY9291
#define MY92XX_CHIPS        1
#define MY92XX_DI_PIN       MTCK_PIN
#define MY92XX_DCKI_PIN     MTDO_PIN
//-------------------------------------------------





class Light : public BaseLight
{
    private:
		// Declare RGBW channels of the MY9291 chip (OUT_x port)
		uint8_t PWM_PINS[CHANNELS_COUNT];
		my92xx* _my92xx;

	protected:
		void lightEngine();
		String getModelId();
		String getType();
		void applyColorsImmediatly();

	public:
		void initPlatformSpecific();

        
};

#endif
