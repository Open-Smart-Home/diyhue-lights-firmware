#ifndef LIGHT_DATA_H
#define LIGHT_DATA_H

enum LightMode { Normal, Flashing };
enum AlertType { None, SingleFlash, MultipleFlash };

struct LightData {
	uint8_t colors[CHANNELS_COUNT];
	//uint8_t bri = 100, sat = 254, colorMode = COLOR_MODE_CT;
	uint8_t bri, sat, colorMode;
	bool lightState;
	int ct, hue;
	float stepLevel[CHANNELS_COUNT];
	float currentColors[CHANNELS_COUNT];
	float x, y;
	LightMode mode;
	uint16_t flashFreqMs; // WARNING: always choose a even number (2000, 3600, etc)
	unsigned long lastFlash;
	uint8_t toDoFlashesLeft;
};

/*
 * A light-weight data structure to save/read LightData from EEPROM
 * We need to store only
 * #define EEPROM_ADDR_COLOR_MODE		4   
 * #define EEPROM_ADDR_HUE				5	// int = 2 byte
 * #define EEPROM_ADDR_CT				7	// int = 2 byte
 * #define EEPROM_ADDR_BRI				9	// int = 2 byte
 * #define EEPROM_ADDR_X				11	// float = 4 byte
 * #define EEPROM_ADDR_Y				15	// float = 4 byte
 */
struct LightDataEeprom {
	bool isOn;
	uint8_t colorMode;
	uint8_t bri;
	uint8_t sat;
	int hue;
	int ct;
	float x;
	float y;
};
#endif