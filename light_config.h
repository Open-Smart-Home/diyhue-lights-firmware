//-----------------------------------------------------------------------------------
// Bridge configuration
//-----------------------------------------------------------------------------------
// If you do not updated the HueEmulator on your Raspberry Pi, uncomment
// this to use the old REST protocol
//#define USE_LEGACY_PROTOCOL

//-----------------------------------------------------------------------------------
// Hardware configuration
//-----------------------------------------------------------------------------------
// Select the light type you want to use uncommenting the correspondig line. 
// On repo site you can find some references and guides to hardware connection 
// to flash both custom light and some ESP8266 based ready-made led bulbs/strip
// 
// >>> WARNING: DOUBLE CHECK YOU SELECT ONLY ONE LIGHT MODEL <<<

// --- CT (warm white and cold white) only ---
//#define PWM_CT_TYWE3S		// it use TYWE3S, a ESP8266 board

// --- RGB only ---
//#define WS2812B_RGB_LED_STRIP			// use WS2812B led chip (aka NeoPixel)
//#define WS2812B_RGB_LED_RING			// use WS2812B led chip (aka NeoPixel)
//#define WS2812B_RGB_LED_BULB			// use WS2812B led chip (aka NeoPixel)
//#define MY9231_RGB					// use MY9231 led driver (3 channels)
//#define PWM_RGB_GENERIC				// usually RGB are directly controlled by ESP8266 via PWM (bulb or led strip)

// --- RGB + W ---
#define MY9291_RGBW		// use MY9291 led driver (4 channels)
//#define PWM_RGBW_ESP8285	// it use ESP8285 instead of ESP8266


//------------------------------------------------------------
// RGB-W ONLY LAMP - WHITE TEMP CONFIG
//------------------------------------------------------------
// Define (W)hite led color temp here (Range 2000-6536K).
// For warm-white led try 2700K, for cold-white try 6000K
#define WHITE_TEMP 4500 // kelvin





//------------------------------------------------------------
// WS2812B advanced configuration
//------------------------------------------------------------
#if (defined WS2812B_RGB_LED_STRIP) || (defined WS2812B_RGB_LED_RING) || (defined WS2812B_RGB_LED_BULB)
#define TOTAL_LED_COUNT 60	// Total number of WS2812B leds (aka PIXELs). 
								// If you use multiple light tot leds must be N_LIGHTS * LED_OF_EACH_LIGHT
#endif

// Only for WS2812B led strip (in other cases it does not make sense)
#ifdef WS2812B_RGB_LED_STRIP
// If you want, you can divide the strip in multiple light, so you can obtain 
// nice effects. To achieve it you have to configure these parameters:
#define LIGHTS_IN_STRIP 3	// The number of lights you want on the strip (each light will have TOTAL_LED_COUNT / LIGHTS_IN_STRIP leds)
#define TRANSITION_LEDS 6 	// MUST BE even number AND 'TOTAL_LED_COUNT' must be divisible by this number
#endif
//------------------------------------------------------------



//------------------------------------------------------------
// PWM-based light advanced configuration
//------------------------------------------------------------
// DEFINE THE GPIO-x PIN USED TO DRIVE PWM CHANNELS
#ifdef PWM_RGB_GENERIC
#define PWM_RED     12
#define PWM_GREEN   5
#define PWM_BLUE    13
#endif
#ifdef PWM_RGBW_ESP8285
#define PWM_RED     5
#define PWM_GREEN   14
#define PWM_BLUE    12
#define PWM_WHITE   13
#endif
#ifdef PWM_CT_TYWE3S
#define PWM_WARM_WHITE   13
#define PWM_COOL_WHITE   5
#endif


// NOTE: if you are using a my92xx chip based light, you can
// change advanced settings in my92xxRgbw.h file if you face any issue







//-----------------------------------------------------------------------------------
// Network configuration
//-----------------------------------------------------------------------------------
//#define USE_STATIC_IP // --> uncomment this line to enable Static IP Adress
// then configure static IP data below
#ifdef USE_STATIC_IP
IPAddress device_ip(192, 168, 0, 95); // choose an unique IP Adress
IPAddress gateway_ip(192, 168, 0, 1); // Router IP
IPAddress subnet_mask(255, 255, 255, 0);
#endif




