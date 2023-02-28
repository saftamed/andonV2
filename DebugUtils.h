#ifndef DEBUGUTILS_H
#define DEBUGUTILS_H


#ifdef DEBUG
#define DEBUG_PRINTLN(str)    \
    Serial.print(millis());     \
    Serial.print(": ");    \
    Serial.print(__PRETTY_FUNCTION__); \
    Serial.print(' ');      \
    Serial.print(__LINE__);     \
    Serial.print(' ');      \
    Serial.println(str);\
    delay(10);
#define DEBUG_PRINT(str)\
    Serial.print(str);\
    delay(10);
#else
#define DEBUG_PRINT(str)
#define DEBUG_PRINTLN(str)
#endif

#endif