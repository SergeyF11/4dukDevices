
//#define ND_STRTOK_R_DEBUG
#define MAX_TOKEN_LENGTH 32
// static const int _MAX_TOKEN_LENGTH=128;
#include "nd_strtok_r.h"

const char str[]  ="device:test:action:on_off:value:on:other:Value:test:memory:leaks";
void printMem(){
    Serial.printf((PGM_P)PSTR("%ld (%ld)\n"), ESP.getFreeHeap(), ESP.getMaxFreeBlockSize());
};

//const char tok[] PROGMEM = "test";
void setup(){
    Serial.begin(115200);
    delay(1000);
    Serial.println("++++++++++++++++++++++++++");
    printMem();
    {
    NotDestroyed::Token data;
    const char * rest = str;
    printMem();

    while( ( data = NotDestroyed::strtok_r(rest, ':', &rest ))){
        // в конце осталось 51832
        Serial.printf((PGM_P)PSTR("Data %s, len=%d\n"), data.c_str(), data.len() );
        printMem();
        //Serial.printf("Token '%s' is 'on'? %s\n", data.toStr().c_str(), ( data == "on") ? "true" : "false" );
        Serial.printf((PGM_P)PSTR("Token '%s' is 'on'? %s\n"), data.c_str(), ( data == "on") ? F("true") : F("false") );
        //if( data.isEquals_P(tok) ) Serial.println(F("Token 'test' found"));
        if( data.isEquals( F("memory") )) Serial.println(F("Token 'memory' found"));
        if( data == F("Value") ) Serial.println(F("Token 'Value' found"));
    }
    Serial.println("++++++++++++++++++++++++++");

    printMem();
    }
    printMem();

    //NotDestroyed::Token data;
    auto start = micros64();
    auto value = NotDestroyed::getTokenValue( str, ':', "Value");
    Serial.println( micros64()-start);
    Serial.println(value);
    free(value);
    printMem();

    start = micros64();
    value = NotDestroyed::getToken(str, ':', "memory").value();
    Serial.println( micros64()-start);
    Serial.println(value);
    free(value);
    printMem();
    {
        auto start = micros64();
    auto value = NotDestroyed::getToken( str, ':', "Value").value();
    Serial.println( micros64()-start);
    Serial.println(value);
    free(value);
    printMem();
    }
    {
    char val[64];
    char (*adr)[64];
    adr = &val;
    Serial.print("1: ");
    Serial.println( NotDestroyed::getToken( str, ':', "action").value(val));
    Serial.print("2: ");
    Serial.println( NotDestroyed::getToken( str, ':', "none").value(val));
    Serial.print("3: ");
    Serial.println( NotDestroyed::getToken( str, ':', "leaks").value(val));
    Serial.print("4: ");
    }
    printMem();
    Serial.println("++++++++++++ the End ++++++++++++++");
}

void loop(){

}