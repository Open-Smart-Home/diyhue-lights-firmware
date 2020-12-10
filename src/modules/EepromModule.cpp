#include "EepromModule.h"

void EepromModule::eraseAll() {
    EEPROM.begin(USED_EEPROM_SIZE);
    for(uint16_t i = 0; i < USED_EEPROM_SIZE; i++){
        EEPROM.write(i, EEPROM_DEFAULT_VALUE);
    }
    EEPROM.commit();
}


void EepromModule::printEepromDump() {
    EEPROM.begin(USED_EEPROM_SIZE);
    uint8_t data;

    const uint8_t itemSize = 2;
    char itemBuffer[itemSize] = { 0 };  // eg if byte is 255 we have '[' 'F' 'F' ']

    const uint8_t firstLineByteAddressCharsCount = 4; // 4 char
    const uint8_t firstLineByteAddressTotalLenght = firstLineByteAddressCharsCount + 2; // add 2 spaces

    // The buffer size is: 2 char for each byte + 1 space every byte +  every 16 byte 1 char more to print 'new line'
    const uint16_t bufferSize =
        (itemSize * USED_EEPROM_SIZE) * 2       // space for header and header separator
        + (itemSize * USED_EEPROM_SIZE)         // space for each byte
        + (uint16_t)ceil((float)USED_EEPROM_SIZE / 16)                             // space for 'new line' char every 16 byte
        + ((uint16_t)ceil((float)USED_EEPROM_SIZE / 16) * firstLineByteAddressTotalLenght)    // space for 'line number' char every 16 byte
        + USED_EEPROM_SIZE * 4;     // more bytes for spaces etc
    char buffer[bufferSize] = { '-' };

    uint16_t bufferIndex = 0;
    uint8_t indexOfPrintedByteInCurrentLine = 0;

    // Print first line with <N-space for line number>00 01 ... 15
    for (uint8_t i = 0; i < firstLineByteAddressTotalLenght; i++) {
        buffer[bufferIndex] = ' ';      bufferIndex++;
    }

    for (int i = 0; i < 16; i++){
        // Add one more space to separate line in two 8 bytes parts
        if( i == 8){
            buffer[bufferIndex] = ' ';      bufferIndex++;
            buffer[bufferIndex] = ' ';      bufferIndex++;
        }
        sprintf(itemBuffer, "%02d", i);
        for(uint8_t j = 0; j < sizeof(itemBuffer); j++)
        {
            buffer[bufferIndex] = itemBuffer[j];
            bufferIndex++;
        }
        buffer[bufferIndex] = ' ';          bufferIndex++;
    }
    // Print a sort of divisor (-----)
    buffer[bufferIndex] = '\n';     bufferIndex++;
    for (int i = 0; i < (16 + firstLineByteAddressTotalLenght); i++){
        buffer[bufferIndex] = '-';      bufferIndex++;
        buffer[bufferIndex] = '-';      bufferIndex++;
        buffer[bufferIndex] = '-';      bufferIndex++;
    }

    // Print eeprom bytes
    buffer[bufferIndex] = '\n';     bufferIndex++;
    for (int i = 0; i < USED_EEPROM_SIZE; i++)
    {
       
        if( i > 0){
            // when a line with 16 byte has been printed then print new line and next line number
            if( i % 16 == 0){
                buffer[bufferIndex] = '\n';     bufferIndex++;
                
                // print the address of the first byte of the line (eg. on line 2 --> 10 (16dec) due to line 1 is from 00-0F)
                char firstLineByteAddressBuffer[firstLineByteAddressCharsCount] = { 0 };
                sprintf(firstLineByteAddressBuffer, "%04X", i);
                for (uint8_t i = 0; i < firstLineByteAddressCharsCount; i++) {
                    buffer[bufferIndex] = firstLineByteAddressBuffer[i];      bufferIndex++;
                }
                buffer[bufferIndex] = ' ';                      bufferIndex++;
                buffer[bufferIndex] = ' ';                      bufferIndex++;

                indexOfPrintedByteInCurrentLine = 0;
            }else{
                buffer[bufferIndex] = ' ';
                bufferIndex++;
            }
        }
        else if(i == 0){
            // On first line (writteb byte = 0) as first print the initial address
            char firstLineByteAddressBuffer[firstLineByteAddressCharsCount] = { 0 };
            sprintf(firstLineByteAddressBuffer, "%04X", i );
            for (uint8_t i = 0; i < firstLineByteAddressCharsCount; i++) {
                buffer[bufferIndex] = firstLineByteAddressBuffer[i];      bufferIndex++;
            }
            buffer[bufferIndex] = ' ';                      bufferIndex++;
            buffer[bufferIndex] = ' ';                      bufferIndex++;
        }

        // Add one more space to separate line in two 8 bytes parts
        if( indexOfPrintedByteInCurrentLine == 8){
            buffer[bufferIndex] = ' ';      bufferIndex++;
            buffer[bufferIndex] = ' ';      bufferIndex++;
        }

        // read eeprom byte and print it
        data = (uint8_t)EEPROM.read(i);
        sprintf(itemBuffer, "%02X", data);
        for(uint8_t j = 0; j < sizeof(itemBuffer); j++)
        {
            buffer[bufferIndex] = itemBuffer[j];
            bufferIndex++;
        }

        indexOfPrintedByteInCurrentLine++;
    }
    EEPROM.end();

    Serial.println("### EEPROM DUMP ###");
    Serial.println(buffer);
    Serial.println("###################");
    //return returnString;
}


bool EepromModule::saveConfig(DeviceConfig *config) {
    Serial.println("[EepromModule] saveConfig BEGIN");
    eepromBegin();
    EEPROM.put(ADDR_CONFIG_INIT, *config);
    eepromCommitAndEnd();
    printEepromDump();
    Serial.println("[EepromModule] saveConfig END");
    return true;
}

void EepromModule::restoreConfig(DeviceConfig *config) {
    Serial.println("[EepromModule] restoreConfig");
    //DeviceConfig tmpConfig;
    eepromBegin();
    EEPROM.get(ADDR_CONFIG_INIT, *config);
    eepromEnd();
    Serial.println("[EepromModule] restoreConfig END");
}


void EepromModule::testEeprom() {
    uint16_t addr = USED_EEPROM_SIZE - 1;
    eepromBegin();
    EEPROM.write(addr, 3);
    eepromCommitAndEnd();

    eepromBegin();
    uint8_t val = EEPROM.read(addr);
    if (val == 3) {
        Serial.println("[EepromModule::testEeprom] --> EEPROM IS OK");
    }else{
        Serial.println("[EepromModule::testEeprom] --> EEPROM NOT WORKING");
    }
}

void EepromModule::writeEepromDataControlIfNotSet() {
    //Serial.println("[EepromModule::writeEepromDataControlIfNotSet] BEGIN");

    if (isEpromDataCompliant() == false)
    {
        //Serial.println("Control value not found, write it");
        eepromBegin();
        EEPROM.write(EEPROM_ADDR_CONTROL_VAL, EEPROM_CTRL_VAL);
        eepromCommitAndEnd();
    }
    else {
        //Serial.println("Control value already set");
    }
    //Serial.println("[EepromModule::writeEepromDataControlIfNotSet] END");
}

bool EepromModule::isEpromDataCompliant()
{
    EEPROM.begin(USED_EEPROM_SIZE);
    uint8_t test = EEPROM.read(EEPROM_ADDR_CONTROL_VAL);
    EEPROM.end();

    /*Serial.print("[EepromModule::isEpromDataCompliant] EEPROM addr (");
    Serial.print(EEPROM_ADDR_CONTROL_VAL);
    Serial.print(")-->");
    Serial.print(test);
    Serial.print("/");
    Serial.print(EEPROM_CTRL_VAL);
    Serial.println();*/

    return (test == EEPROM_CTRL_VAL);
}



void EepromModule::eepromBegin() {
    EEPROM.begin(USED_EEPROM_SIZE);
}

void EepromModule::eepromCommitAndEnd() {
    //uint16_t addr = ADDR_EEPROM_DATA_VERSION;
    //EEPROM.write(addr, (uint8_t)EEPROM_DATA_VERSION);    addr += sizeof(uint8_t);

    // actually write the content of byte-array cache to
    // hardware flash.  flash write occurs if and only if one or more byte
    // in byte-array cache has been changed, but if so, ALL 512 bytes are
    // written to flash
    EEPROM.commit();
    EEPROM.end();
}

void EepromModule::eepromEnd() {
    EEPROM.end();
}


void EepromModule::debugPrintWriteAddress(uint16_t addr, bool addEol) {
    Serial.print("[EepromModule] Write at: "); Serial.print(addr, DEC);
    if( addEol){
        Serial.println();
    }
}

void EepromModule::debugPrintAddressAndByteUse(const char* fieldName, uint16_t addr, uint16_t usedByte) {
    Serial.print("Write "); Serial.print(fieldName); Serial.print(" from "); Serial.print(addr, DEC);
    Serial.print(" and use "); Serial.print(usedByte, DEC); Serial.print(" bytes");
    Serial.println();
}
