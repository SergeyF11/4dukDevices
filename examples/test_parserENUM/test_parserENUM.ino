#include <my4duk.h>


typedef Duk::Parser::d_Ptrs d_Ptrs;
void setup()
{
    d_Ptrs test;
    Serial.begin(115200);
    do { 
        delay(300);
    } while( !Serial);
    Serial.println();
    Serial.println("=======================");

    for ( test = d_Ptrs::_Device;
        test < d_Ptrs::Owersize;
        ++test )
    {
        Serial.println(test);
    }
    Serial.println("+++++++++++++++++++++++++");
        while( --test ){
            Serial.println(test);
        }

}

void loop(){}
