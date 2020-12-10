#include <ArduinoJson.hpp>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <EEPROM.h>
#include <ArduinoJson.h>

//***************************************************************
// READ CAREFULLY
// To configure the firmware for different lights, please use
// the 'light_config.h' file.
//***************************************************************


//-----------------------------------------------------------------------------------
// DEBUG
// If you want to use Serial.print.. then uncomment 'USE_SERIAL_AS_DEBUG'
// When the debug is active you can also choose the type of debug to show
//-----------------------------------------------------------------------------------
//#define USE_SERIAL_AS_DEBUG

// when the serial debug is enabled you can choose the to turn on/off specific debug type
#ifdef USE_SERIAL_AS_DEBUG
//#define USE_REST_DEBUG
//#define USE_WIFI_MANAGER_DEBUG
#endif







//-----------------------------------------------------------------------------------
// DO NOT CHANGE ANYTHING BELOW THIS LINE UNLESS YOU KNOW WHAT YOU ARE DOING
//-----------------------------------------------------------------------------------
//#define FIRMWARE_VERSION "1.0.0"



#include "src/models/AdvancedSerial.h"
#include "src/modules/OtaModule.h"
#include "src/modules/SaveRestoreDataModule.h"
#include "src/modules/EepromModule.h"

#include "src/internal_cfg.h"

// Import the right class according with user light choice
#ifdef PWM_CT_TYWE3S
#include "src/cct/pwmCct.h"
#endif
#ifdef WS2812B_RGB_LED_STRIP
#include <NeoPixelBus.h>
#include "src/rgb/Ws2812bRgbLedStrip.h"
#endif
#ifdef WS2812B_RGB_LED_RING_OR_BULB
#include <NeoPixelBus.h>
// TODO: add include for WS2812B led ring/bulb
#endif
#ifdef PWM_RGB_GENERIC
#include "src/rgb/pwmRgbGeneric.h"
#endif
#ifdef MY9291_RGBW
#include "src/rgbw/my92xxRgbw.h"
#endif
#ifdef PWM_RGBW_ESP8285
#include "src/rgbw/pwmRgbw.h"
#endif


#define BOARD_LED_ON LOW
#define BOARD_LED_OFF HIGH







Light light = Light();
static String lightName = "HueLight-" + String(ESP.getChipId());
bool isConnected = false;
byte mac[6];

// Use server to handle light requests and UDP to allow OTA
ESP8266WebServer server(80);
WiFiUDP wifiUdp;

OtaModule otaModule;






void debugInitSerial()
{
	Serial.begin(115200);
	delay(20);
	SerialPrintln(); SerialPrintln();
	SerialPrintln("*************************");
	SerialPrintln("*** " + lightName + " ***");
	SerialPrintln("*************************");
}

/*
 * Print some information about the device hardware
 * (useful to indentify flash size, etc)
 */
void printHardwareInfo()
{
	Serial.begin(115200);
	delay(10);
	SerialPrint("Chip ID:");  SerialPrintln(ESP.getChipId());
	SerialPrint("Flash size:");  SerialPrintln(ESP.getFlashChipSize());
	SerialPrint("Flash real size:");  SerialPrintln(ESP.getFlashChipRealSize());
	SerialPrint("FreeSketchSpace:"); SerialPrintln(ESP.getFreeSketchSpace());
}




bool initWifi()
{
	// For the first use configuration wifi network name use the light name + a rand value
	//uint8_t randVal = rand() % 255;
	String configWifiNetworkName = lightName;// +"-" +(String)randVal;
	SerialPrint("NETWORK NAME:");
	SerialPrintln(configWifiNetworkName);
	WiFi.hostname(configWifiNetworkName.c_str());
#ifdef USE_STATIC_IP
	WiFi.config(device_ip, gateway_ip, subnet_mask);
#endif

	WiFiManager wifiManager;

#ifndef USE_WIFI_MANAGER_DEBUG
	// By default debug is true
	wifiManager.setDebugOutput(false);
#endif
	
	// If you are facing errors with WiFiManager:
	// 1) Uncomment 'reset' then flash firmware and execute init
	// 2) Comment 'reset' again the reflash firwmare and entertainment_run
	// TODO: add reset button
	//wifiManager.resetSettings();

	// Set a timeout if you want the ESP doesn't hang waiting to be configured
	// (eg	for instance after a power failure)
	//wifiManager.setConfigPortalTimeout(120); // seconds
	
	wifiManager.setAPCallback(configModeCallback);
	wifiManager.setSaveConfigCallback(saveConfigCallback);	
	isConnected = wifiManager.autoConnect(configWifiNetworkName.c_str());
	WiFi.macAddress(mac);

#ifdef USE_STATIC_IP
	light.setNetworkCfg(device_ip, gateway_ip, subnet_mask);
#else
	light.setNetworkCfg(WiFi.localIP(), WiFi.gatewayIP(), WiFi.subnetMask());
#endif

	return isConnected;
}

//gets called when WiFiManager enters configuration mode
void configModeCallback(WiFiManager *myWiFiManager)
{
	isConnected == false;
	//SerialPrintln("WiFiManager config mode callback"); 

	light.notifyNotConnected();
}

void saveConfigCallback()
{
	//SerialPrintln("WiFiManager save config callback");
	isConnected = true;
}

void initIOPins()
{
	pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  	digitalWrite(LED_BUILTIN, BOARD_LED_OFF);  // Turn the LED off by making the voltage HIGH
}


void handleNotFound() {
	logRestRequest(&server);

	String message = "File Not Found\n\n";
	message += "URI: ";
	message += server.uri();
	message += "\nMethod: ";
	message += (server.method() == HTTP_GET) ? "GET" : server.method() == HTTP_POST ? "POST" : server.method() == HTTP_PUT ? "PUT" : server.method() == HTTP_DELETE ? "DELETE" : "unknown";
	message += "\nArguments: ";
	message += server.args();
	message += "\n";
	for (uint8_t i = 0; i < server.args(); i++) {
		message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
	}
  	server.send(404, "text/plain", message);
}





#define REST_LOG_BUFFER_SIZE	400
#define REST_LOG_LINES			5
char restRequestLog[REST_LOG_LINES][REST_LOG_BUFFER_SIZE];
uint8_t lastLogIndex = 0;

void logRestRequest(ESP8266WebServer* server)
{
	String message = "[REST] ";
	//message += ((int)lastLogIndex) + ") ";
	message += server->uri();
	message += " [";
	message += (server->method() == HTTP_GET) ? "GET" : server->method() == HTTP_POST ? "POST" : server->method() == HTTP_PUT ? "PUT" : server->method() == HTTP_DELETE ? "DELETE" : "unknown";
	message += "] ";
	//message += " ["; message += server->args(); message += "]";
	for (uint8_t i = 0; i < server->args(); i++) {
		message += " " + server->argName(i) + ": " + server->arg(i) + "\n";
	}

	//server->arg("plain").toCharArray(restRequestLog[0], REST_LOG_BUFFER_SIZE);
	message.toCharArray(restRequestLog[lastLogIndex], REST_LOG_BUFFER_SIZE);
	
	lastLogIndex++;
	if (lastLogIndex >= REST_LOG_LINES) {
		lastLogIndex = 0;
	}

	SerialPrintln(message);
}


void registerServerEntryPoints()
{
	server.on("/", []() {
		//logRestRequest();
		logRestRequest(&server);
		light.srvOnBaseReq(&server);
	});

	// ---------- OLD PROTOCOL ----------
	server.on("/switch", []() {
		light.srvOnSwitch(&server);
	});

	server.on("/set", []() {
		//logRestRequest();
		logRestRequest(&server);
		light.srvOnSet(&server);
	});

	server.on("/get", []() {
		//light.srvOnGet(&server);
		light.srvOnStateGet(&server);
	});

	server.on("/detect", []() {
		logRestRequest(&server);
		light.srvOnDetect(&server, mac);
	});

	// ---------- NEW PROTOCOL ----------
	server.on("/state", HTTP_GET, []() {
		logRestRequest(&server);
		light.srvOnStateGet(&server);
	});
	server.on("/state", HTTP_POST, []() {
		logRestRequest(&server);
		light.srvOnStatePut(&server);
	});
	server.on("/state", HTTP_PUT, []() {
		logRestRequest(&server);
		light.srvOnStatePut(&server);
	});
	server.on("/config", HTTP_GET, []() {
		logRestRequest(&server);
		light.srvOnConfigGet(&server);
	});
	server.on("/config", HTTP_POST, []() {
		logRestRequest(&server);
	});
	server.on("/config", HTTP_PUT, []() {
		logRestRequest(&server);
	});

	server.on("/reset", []() {
		server.send(200, "text/html", "reset");
		delay(1000);
		ESP.restart();
	});

	// ---------- DEBUG ----------
	server.on("/remoteconsole", HTTP_POST, []() {
		DynamicJsonDocument root(200);
		DeserializationError error = deserializeJson(root, server.arg("plain"));

		if (error) {
			server.send(404, "text/plain", "FAIL. " + server.arg("plain"));
		}
		else {
			SerialPrint("REMOTE CONSOLE COMMAND:");
			if (root.containsKey("cmd")) {
				const char* command = root["cmd"];
				SerialPrintln(command);
				executeConsoleCommand(command);
			}
		}
		server.send(200, "text/plain", "");
	});

	/*server.on("/restlog", []() {
		char response[100 + REST_LOG_LINES * REST_LOG_BUFFER_SIZE + REST_LOG_LINES] = { '\0' };
		char tmp[40] = { '\0' };
		sprintf(tmp, "Last REST is: %d\n", lastLogIndex);
		strcat(response, tmp);

		for (uint8_t i = 0; i < 5; i++) {
			strcat(response, restRequestLog[i]);
			strcat(response, "\n");
		}
		server.send(200, "text/plain", response);
	});*/
	// --------------------------

	server.onNotFound(handleNotFound);

	server.begin();
}



void clearEeprom() {
	EepromModule::eraseAll();       // Clear all EEPROM data
	EepromModule::printEepromDump();
}


char serialBuffer[128] = { '\0' };
uint8_t serialBufferIndex = 0;
uint8_t eolChar = 10;
uint32_t lastSerialCharMillis;

void resetSerialBuffer() {
    for (uint8_t i = 0; i < sizeof(serialBuffer); i++) {
        serialBuffer[i] = '\0';
    }
    serialBufferIndex = 0;
}


void executeConsoleCommand(const char* command) {
	SerialPrint("[Console command]: "); SerialPrintln(command);

	if (strcmp(command, "/factoryreset") == 0) {
		//factoryReset();
	}
	else if (strcmp(command, "/cleareeprom") == 0) {
		clearEeprom();
	}
	else if (strcmp(command, "/clearwifi") == 0) {
		//clearSavedWifi();
	}
	/*else if (strcmp(command, "/clearfirstboot") == 0) {
		EepromModule::clearFirstBootCompletedFlag();
	}
	else if (strcmp(command, "/clearmqtt") == 0) {
		EepromModule::clearMqttConfig();
	}
	else if (strcmp(command, "/eepromnotusedclear") == 0) {
		EepromModule::resetNotUsedEeprom();
	}*/
	else if (strcmp(command, "/reboot") == 0) {
		ESP.restart(); //ESP.reset();
	}
	else if (strcmp(command, "/eepromdump") == 0) {
		EepromModule::printEepromDump();
	}
	else if (strcmp(command, "/testeeprom") == 0) {
		EepromModule::testEeprom();
	}
	else if (strcmp(command, "/eeprominitialize") == 0) {
		EepromModule::writeEepromDataControlIfNotSet();
	}
	else if (strcmp(command, "/readconfig") == 0) {
		DeviceConfig tmp;
		SaveRestoreDataModule::restoreConfig(&tmp);
		tmp.debugPrint();
	}
	else if (strcmp(command, "/setconnectedoff") == 0) {
		isConnected = false;
	}
	else if (strcmp(command, "/setconnectedon") == 0) {
		isConnected = true;
	}
	else {
		SerialPrintln("Unknow command");
	}
}

void debugHandleSerialCommands()
{
    if (millis() - lastSerialCharMillis > 5000) {
        resetSerialBuffer();
    }

    while(Serial.available() > 0)
    {
        // read the incoming byte:
        uint8_t readChar = Serial.read();
        lastSerialCharMillis = millis();

        if (readChar == eolChar)
        {
			executeConsoleCommand(serialBuffer);
           
            // Reset serial buffer
            resetSerialBuffer();
        }
        else {
            serialBuffer[serialBufferIndex] = readChar;
            serialBufferIndex++;
        }
        delay(10);
    }

}





void setup()
{
	debugInitSerial();
	printHardwareInfo();

	//EepromModule::testEeprom();

	SaveRestoreDataModule::init();
	
#ifdef USE_ENTERTAINMENT
	light.setUdpForEntertainment(&wifiUdp);
#endif

	light.init(&lightName);
	
	initWifi();

	registerServerEntryPoints();

	if( isConnected == true) {
	    SerialPrintln("[OTA] WiFi connected, init OTA module");
        char nameBuffer[100 + 1] = {'\0'};
        lightName.toCharArray(nameBuffer, 101);
        delay(100);
        otaModule.init(nullptr, nameBuffer);
    }else{
        SerialPrintln("[OTA] WiFi NOT connected, skip init OTA module");
	}
}

unsigned long lastMillis = 0;
void loop()
{
    debugHandleSerialCommands();

    //ArduinoOTA.handle();
    otaModule.run();
    server.handleClient();
    light.run(isConnected);

    if( millis() - lastMillis > 10000){
        SerialPrint("Running OK (Free Heap: ");
		SerialPrint(ESP.getFreeHeap());
		SerialPrintln(")");
		lastMillis = millis();
    }
}