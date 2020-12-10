#include "SaveRestoreDataModule.h"

bool SaveRestoreDataModule::init() {
    SerialPrintln("[SaveRestoreDataModule::init] BEGIN");
    if(SPIFFS.begin() == false) {
        SerialPrintln("[ERROR] Failed to mount file system");
        return false;
    }else{
        SerialPrintln("SPIFFS OK");
    }
    SerialPrintln("[SaveRestoreDataModule::init] END");
    return true;
}

bool SaveRestoreDataModule::getFileContent(const char *fileName, char buffer[], uint16_t bufferSize) {
    File configFile = SPIFFS.open(fileName, "r");
    if (!configFile) {
        SerialPrint("[ERROR] The file does not exist:"); SerialPrint(fileName); SerialPrintln();
        return false;
    }
    configFile.readString().toCharArray(buffer, bufferSize);
}

bool SaveRestoreDataModule::saveConfig(DeviceConfig *config){
    SerialPrintln("[SaveRestoreDataModule::saveConfig] BEGIN");
    config->debugPrint();

//    // JSON example: {"dv": 1, "n":"Lorem ipsum dolor sit amet","ip":[192,168,101,102],"g":[192,168,101,100],"sm":[255,255,255,255],"s":100,"m":10}
//    // Calculate space with https://arduinojson.org/v6/assistant/
//    DynamicJsonDocument  jsonDoc(SAVED_JSON_SIZE_CONFIG);
//    jsonDoc[SAVED_JSON_KEY_DEVICE_NAME] = config->getLightName();
//    jsonDoc[SAVED_JSON_KEY_SCENE] = config->scene;
//    jsonDoc[SAVED_JSON_KEY_STARTUP_MODE] = config->getStartupMode();
//    //jsonDoc["lightscount"] = 1;
//    JsonArray ip = jsonDoc.createNestedArray(SAVED_JSON_KEY_DEVICE_IP);
//    ip.add(config->device_ip[0]); ip.add(config->device_ip[1]); ip.add(config->device_ip[2]); ip.add(config->device_ip[3]);
//    JsonArray gateway = jsonDoc.createNestedArray(SAVED_JSON_KEY_DEVICE_GATEWAY);
//    gateway.add(config->gateway_ip[0]); gateway.add(config->gateway_ip[1]); gateway.add(config->gateway_ip[2]); gateway.add(config->gateway_ip[3]);
//    JsonArray mask = jsonDoc.createNestedArray(SAVED_JSON_KEY_DEVICE_SUBNET_MASK);
//    mask.add(config->subnet_mask[0]); mask.add(config->subnet_mask[1]); mask.add(config->subnet_mask[2]); mask.add(config->subnet_mask[3]);
//
//    serializeJson(jsonDoc, Serial);
//    bool result = saveJsonToSpiffs("/config.json", &jsonDoc);

    bool result = EepromModule::saveConfig(config);
  
    return result;
}

void SaveRestoreDataModule::restoreConfig(DeviceConfig* config) {
    EepromModule::restoreConfig(config);
}


bool SaveRestoreDataModule::saveJsonToSpiffs(const char * filename, DynamicJsonDocument *json) {
    //if(SaveRestoreDataModule::_isSpiffsInit == false){
    //    SaveRestoreDataModule::_isSpiffsInit = SPIFFS.begin();
    //    // If the SPIFFS was not init and after 'init' call is still 'false' ther there's an error
//        if(SaveRestoreDataModule::_isSpiffsInit == false) {
//            SerialPrintln("[ERROR] Failed to mount file system");
//            return false;
//        }
    //}

    File stateFile = SPIFFS.open(filename, "w");
    serializeJson(*json, stateFile);
}


bool SaveRestoreDataModule::saveAllLightsData(LightData lightsData[DEVICE_LIGHTS_COUNT])
{
	//SerialPrintln("[BaseLight::saveState] BEGIN");
	// TODO: evaluate to swithc to JSON and SPIFFS??

	// Write a secure value to identify data
	EepromModule::writeEepromDataControlIfNotSet();

	EepromModule::eepromBegin();
	uint16_t eepromAddr = ADDR_LIGHT_DATA_INIT;
	for (uint8_t i = 0; i < DEVICE_LIGHTS_COUNT; i++) {
		LightDataEeprom tmp;
		tmp.isOn		= lightsData->lightState;
		tmp.colorMode	= lightsData[i].colorMode;
		tmp.hue			= lightsData[i].hue;
		tmp.ct			= lightsData[i].ct;
		tmp.bri			= lightsData[i].bri;
		tmp.sat			= lightsData[i].sat;
		tmp.x			= lightsData[i].x;
		tmp.y			= lightsData[i].y;
		EEPROM.put(eepromAddr, tmp);
		
		SerialPrint("[SaveRestoreDataModule::saveState] Save light [");
		SerialPrintlnWithFormat(i, DEC);
		SerialPrint("] LighDataEeprom at addr: "); 
		SerialPrintf("%04X", eepromAddr);
		SerialPrintln();

		eepromAddr += LIGHT_DATA_SIZE;
	}
	EepromModule::eepromCommitAndEnd();

	SerialPrint("[SaveRestoreDataModule::saveState] Should end at:");
	SerialPrintf("%04X", ADDR_LIGHT_DATA_END);
	SerialPrintln();

	//SerialPrintln("[BaseLight::saveState] END");
	return true;
}

bool SaveRestoreDataModule::saveLightData(LightData* lightData, uint8_t lightIndex) {
	// Write a secure value to identify data
	EepromModule::writeEepromDataControlIfNotSet();

	EepromModule::eepromBegin();
	uint16_t eepromAddr = ADDR_LIGHT_DATA_INIT;
	for (uint8_t i = 0; i < DEVICE_LIGHTS_COUNT; i++) {
		if (i == lightIndex) {
			LightDataEeprom tmp;
			tmp.isOn		= lightData->lightState;
			tmp.colorMode	= lightData->colorMode;
			tmp.hue			= lightData->hue;
			tmp.ct			= lightData->ct;
			tmp.bri			= lightData->bri;
			tmp.sat			= lightData->sat;
			tmp.x			= lightData->x;
			tmp.y			= lightData->y;
			EEPROM.put(eepromAddr, tmp);
		
			SerialPrint("[SaveRestoreDataModule::saveState] Save light [");
			SerialPrintlnWithFormat(i, DEC);
			SerialPrint("] LighDataEeprom at addr: "); 
			SerialPrintf("%04X", eepromAddr);
			SerialPrintln();
		}

		eepromAddr += LIGHT_DATA_SIZE;
	}
	EepromModule::eepromCommitAndEnd();

	SerialPrint("[SaveRestoreDataModule::saveState] Should end at:");
	SerialPrintf("%04X", ADDR_LIGHT_DATA_END);
	SerialPrintln();

	return true;
}




bool SaveRestoreDataModule::restoreAllLightsData(LightData lightsData[DEVICE_LIGHTS_COUNT])
{
	// TODO: evaluate to swithc to JSON and SPIFFS??

	if (EepromModule::isEpromDataCompliant() == false) {
		// eemprom has no saved value (first execution maybe?)
		SerialPrintln("Control value not found, do not restore anything");
		return false;
	}

	//SerialPrintln("--------- Restoring state ---------");
	EepromModule::eepromBegin();
	uint16_t eepromAddr = ADDR_LIGHT_DATA_INIT;
	for (uint8_t i = 0; i < DEVICE_LIGHTS_COUNT; i++) 
	{
		LightDataEeprom tmp;
		EEPROM.get(eepromAddr, tmp);
		lightsData[i].lightState	= tmp.isOn;
		lightsData[i].colorMode = tmp.colorMode;
		lightsData[i].hue		= tmp.hue;
		lightsData[i].ct		= tmp.ct;
		lightsData[i].bri		= tmp.bri;
		lightsData[i].sat		= tmp.sat;
		lightsData[i].x			= tmp.x;
		lightsData[i].y			= tmp.y;

		SerialPrint("isLightOn > ");	SerialPrintlnWithFormat(lightsData[i].lightState, DEC);
		SerialPrint("colorMode > ");	SerialPrintlnWithFormat(lightsData[i].colorMode, DEC);
		SerialPrint("hue > ");			SerialPrintlnWithFormat(lightsData[i].hue, DEC);
		SerialPrint("ct > ");			SerialPrintlnWithFormat(lightsData[i].ct, DEC);
		SerialPrint("bri > ");			SerialPrintlnWithFormat(lightsData[i].bri, DEC);
		SerialPrint("x > ");			SerialPrintlnWithFormat(lightsData[i].x, DEC);
		SerialPrint("y > ");			SerialPrintlnWithFormat(lightsData[i].y, DEC);

		eepromAddr += LIGHT_DATA_SIZE;
		
	}
	EepromModule::eepromEnd();

	//SerialPrintln("--------- Restored ---------");
	return true;
}


bool SaveRestoreDataModule::restoreLightData(LightData* lightData, uint8_t lightIndex) {
	if (EepromModule::isEpromDataCompliant() == false) {
		// eemprom has no saved value (first execution maybe?)
		SerialPrintln("Control value not found, do not restore anything");
		return false;
	}

	//SerialPrintln("--------- Restoring state ---------");
	EepromModule::eepromBegin();
	uint16_t eepromAddr = ADDR_LIGHT_DATA_INIT;
	for (uint8_t i = 0; i < DEVICE_LIGHTS_COUNT; i++)
	{
		if (i == lightIndex) {
			LightDataEeprom tmp;
			EEPROM.get(eepromAddr, tmp);
			lightData->lightState = tmp.isOn;;
			lightData->colorMode = tmp.colorMode;
			lightData->hue = tmp.hue;
			lightData->ct = tmp.ct;
			lightData->bri = tmp.bri;
			lightData->sat = tmp.sat;
			lightData->x = tmp.x;
			lightData->y = tmp.y;

			SerialPrint("isLightOn > ");	SerialPrintlnWithFormat(lightData->lightState, DEC);
			SerialPrint("colorMode > ");	SerialPrintlnWithFormat(lightData->colorMode, DEC);
			SerialPrint("hue > ");			SerialPrintlnWithFormat(lightData->hue, DEC);
			SerialPrint("ct > ");			SerialPrintlnWithFormat(lightData->ct, DEC);
			SerialPrint("bri > ");			SerialPrintlnWithFormat(lightData->bri, DEC);
			SerialPrint("x > ");			SerialPrintlnWithFormat(lightData->x, DEC);
			SerialPrint("y > ");			SerialPrintlnWithFormat(lightData->y, DEC);
		}
		

		eepromAddr += LIGHT_DATA_SIZE;
	}
	EepromModule::eepromEnd();
	return true;
}

