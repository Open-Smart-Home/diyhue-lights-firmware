#ifndef ADVANCE_SERIAL_H
#define ADVANCE_SERIAL_H

// If the debug is off, then prevent Serial.print... to print
// See: https://arduino.stackexchange.com/questions/9857/can-i-make-the-arduino-ignore-serial-print/9858
#ifdef USE_SERIAL_AS_DEBUG
#define SerialPrintln(a) (Serial.println(a))
#define SerialPrintlnWithFormat(a,b) (Serial.println(a,))
#define SerialPrint(a)	(Serial.print(a))
#define SerialPrintWithFormat(a,b)	(Serial.print(a,b))
#define SerialPrintf(a,b)	(Serial.printf(a,b))
#else
#define SerialPrintln(a)
#define SerialPrintlnWithFormat(a,b)
#define SerialPrint(a)
#define SerialPrintWithFormat(a,b)
#define SerialPrintf(a,b)
#endif

#endif // !BASE_DEFINITIONS_H
