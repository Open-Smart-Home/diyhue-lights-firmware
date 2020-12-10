#ifndef CONFIG_H
#define CONFIG_H

#include "../light_config.h"

#define ALERT_FLASH_DURATION_MS             800
#define NO_CONNECTION_FLASH_DURATION_MS     600
#define NO_CONNECTION_FLASH_LOOPS_COUNT     5

//-----------------------------------------------------------------------------------
// DO NOT CHANGE ANYTHING BELOW THIS LINE UNLESS YOU KNOW WHAT YOU ARE DOING
//-----------------------------------------------------------------------------------
// Define the light type

#if (defined PWM_CT_TYWE3S)
#define LIGHT_TYPE_CCT
#endif
#if (defined WS2812B_RGB_LED_STRIP) || (defined WS2812B_RGB_LED_RING_OR_BULB) || (defined PWM_RGB_GENERIC)
#define LIGHT_TYPE_RGB
#endif

#if (defined MY9291_RGBW) || (defined PWM_RGBW_ESP8285)
#define LIGHT_TYPE_RGBW
#endif

// #ifdef .....
// #define LIGHT_TYPE_CCT
// #endif

// #ifdef .....
// #define LIGHT_TYPE_RGBCCT
// #endif


// Define how many color channels light can handle
#ifdef LIGHT_TYPE_CCT
#define CHANNELS_COUNT	2
#endif
#ifdef LIGHT_TYPE_RGB
#define CHANNELS_COUNT	3
#endif
#ifdef LIGHT_TYPE_RGBW 
#define CHANNELS_COUNT	4 
#endif
#ifdef LIGHT_TYPE_RGBCCT
#define CHANNELS_COUNT	5 
#endif


// Define light color mode capabilities
#if (defined LIGHT_TYPE_RGBW) || (defined LIGHT_TYPE_RGB)
#define HAS_COLOR_MODE_XY
#endif
#if (defined LIGHT_TYPE_RGBW) || (defined LIGHT_TYPE_RGBCCT) || (defined LIGHT_TYPE_CCT)
#define HAS_COLOR_MODE_CT
#endif


// Extra defines
#ifdef WS2812B_RGB_LED_BULB
#define LIGHTS_IN_STRIP 1	// The number of lights you want on the strip (each light will have TOTAL_LED_COUNT / LIGHTS_IN_STRIP leds)
#define TRANSITION_LEDS 0 	// Must be even number
#endif

// Define the number of ligths in the device
// For the WS2812b (NeoPixel) based strip get the 'lights in the strip' count
// For other devices set this to 1
#ifdef WS2812B_RGB_LED_STRIP
#define DEVICE_LIGHTS_COUNT LIGHTS_IN_STRIP
#else
#define DEVICE_LIGHTS_COUNT 1
#endif

#ifdef WS2812B_RGB_LED_STRIP
#define USE_ENTERTAINMENT
#endif


#endif