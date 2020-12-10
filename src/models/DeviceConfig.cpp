
#include "DeviceConfig.h"

void DeviceConfig::saveNetworkData(IPAddress *deviceIp, IPAddress *gatewayIp, IPAddress *subnetMask) {
    device_ip[0] = ((*deviceIp)[0]);
    device_ip[1] = ((*deviceIp)[1]);
    device_ip[2] = ((*deviceIp)[2]);
    device_ip[3] = ((*deviceIp)[3]);

    gateway_ip[0] = ((*gatewayIp)[0]);
    gateway_ip[1] = ((*gatewayIp)[1]);
    gateway_ip[2] = ((*gatewayIp)[2]);
    gateway_ip[3] = ((*gatewayIp)[3]);

    subnet_mask[0] = ((*subnetMask)[0]);
    subnet_mask[1] = ((*subnetMask)[1]);
    subnet_mask[2] = ((*subnetMask)[2]);
    subnet_mask[3] = ((*subnetMask)[3]);
}

void DeviceConfig::saveNetworkData(uint8_t *deviceIp, uint8_t *gatewayIp, uint8_t *subnetMask) {

}

void DeviceConfig::networdDataToStr(char *ipBuf, char *gatwBuf, char *sbnmskBuf) {
    sprintf(ipBuf, "%d.%d.%d.%d", device_ip[0], device_ip[1], device_ip[2], device_ip[3]);
    sprintf(gatwBuf, "%d.%d.%d.%d", gateway_ip[0], gateway_ip[1], gateway_ip[2], gateway_ip[3]);
    sprintf(sbnmskBuf, "%d.%d.%d.%d", subnet_mask[0], subnet_mask[1], subnet_mask[2], subnet_mask[3]);
}

void DeviceConfig::setStartMode(uint8_t mode) {
    _startupMode = mode;
}

bool DeviceConfig::isStartupModePowerFail() { return _startupMode == STARTUP_MODE_POWERFAIL; }
bool DeviceConfig::isStartupModeSafety() {return _startupMode == STARTUP_MODE_SAFETY;}
bool DeviceConfig::isStartupModeOff() {return _startupMode == STARTUP_MODE_OFF;}
uint8_t DeviceConfig::getStartupMode() { return _startupMode;}

void DeviceConfig::setLightName(String *name) {
    name->toCharArray(_lightName, DEVICE_NAME_MAX_LENGTH);
    _lightName[DEVICE_NAME_MAX_LENGTH - 1] = 0; // set null terminator as last char
}

char* DeviceConfig::getLightName() {
    return _lightName;
}


void DeviceConfig::debugPrint() {
    Serial.println("--- Device config (start)---");
    Serial.print("Light name:"); Serial.print(_lightName); Serial.println();
    Serial.print("Startup mode"); Serial.print(_startupMode, DEC); Serial.println();
    Serial.print("Scene:"); Serial.print(scene, DEC); Serial.println();

    char ipBuff[IP_AS_STR_MAX_LENGTH] = {'\0'};
    char gatwBuff[IP_AS_STR_MAX_LENGTH] = {'\0'};
    char sbnmskBuf[IP_AS_STR_MAX_LENGTH] = {'\0'};
    networdDataToStr(ipBuff, gatwBuff, sbnmskBuf);
    Serial.print("IP-ADDR--:"); Serial.print(ipBuff); Serial.println();
    Serial.print("Gateway--:"); Serial.print(gatwBuff); Serial.println();
    Serial.print("SubMsk---:"); Serial.print(sbnmskBuf); Serial.println();
    Serial.println("--- Device config (end) ---");
}