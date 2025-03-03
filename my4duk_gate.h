#pragma once
#include <vector>
#include <functional>
#include <IPAddress.h>
#include <WiFiUdp.h>
#include <WiFiClient.h>
#include "my4duk_device.h"

#define MQTT_WAIT_RESPONSE 3000

//#define MY_4DUK_DEBUG
#define F_TRUE  F("True")
#define F_FALSE F("False")

#ifdef MY_4DUK_DEBUG
#define INIT_DEBUG { while( !Serial){ Serial.begin(115200); }}
#define FUNC_LINE(...) INIT_DEBUG { Serial.print(__PRETTY_FUNCTION__); Serial.println(__LINE__); Serial.println(__VA_ARGS__); Serial.flush(); }
#else
#define FUNC_LINE

#endif

#define  constLength(array) (((sizeof(array))/(sizeof(array[0])))-1)

namespace Duk { 
        //bool endStr( const char s);

        //static const char magicStr[] PROGMEM = "i54478494974d";
        static const char helloStr[] PROGMEM = "hello";    

/// @brief  gate к 4duk по mqtt. 
/// @param id = id шлюза 4duk
/// @param  mdns_name mdns_name.local - имя шлюза в лосальной сети
/// @param  thisGate device для этого шлюза
/// @note Если существует, то устройства общаются через него, 
    /// иначе по udp через внешний роутер  
    class Gate {
        private:

        WiFiClient mqtt;
        const char *gate_id;
        const char *mdns_name;
        bool connected;
        #if defined(ESP32)
        uint8_t pack_buf[1024];
        #elif defined(ESP8266)
        char pack_buf[1024];
        void printBuf(Print& p){
            p.print("pack_buf[]: ");
            int i =0;
            unsigned char c;
            while (( c = pack_buf[i++])){
            p.printf("%c[0x%02x] ",
                isPrintable(c) ? c : '*',
                c );
            }
            p.println();
        };
        #endif
        int trimBuffer(const int size ){
            int i = size - 1;
            //i--;
            while( i >= 0 ){ 
                if ( pack_buf[i] > 0x20 ) {
                    break;
                }
                i--;
            }
            i++;
            pack_buf[i] = '\0';
            return i;
        };
        uint32_t cmil;
        int hasMqttResponse(){ return mqtt.available(); };
        int mqttReadToBuf(){
            int readed = 0;
            auto size = hasMqttResponse();
            if ( size != 0 ) {
                //readed =  mqtt.read(pack_buf, size);
                readed = trimBuffer( mqtt.read(pack_buf, size) );
            }
            return readed; 
        };

     
        bool checkOk(){
            auto size =  mqttReadToBuf();
            return (constLength("OK") <= size &&
                strncmp( pack_buf, "OK", constLength("OK") ) == 0 ); 
        };
        int getMqttResponse( unsigned long ms=MQTT_WAIT_RESPONSE ){ 
            pack_buf[0] = '\0';
            int pack_size=0;
            auto start = millis();
            while ( pack_size == 0 ) {
                if ( ( millis() - start) >= ms ) return pack_size;
                delay(10);
                pack_size=mqtt.available();
            }
            auto readed = mqtt.read(pack_buf,pack_size);
            
            #ifdef DEBUG_PRINTF
            debugPrintf("Readed raw %d bytes, ", readed);
            printBuf(Serial);
            #endif

            readed = trimBuffer(readed);
            Serial.printf("Trimed %d bytes: %s\n", readed, pack_buf);
            return readed;
        };
        bool checkResponse(const char * str){
            FUNC_LINE(0);
            pack_buf[0] = '\0';
            auto len = strlen(str) - 1;
            auto readed = getMqttResponse();
            if ( readed < len ) return false;
            return ( strncmp(pack_buf, str, len ) == 0 );
        };
        bool checkResponseP(const char * str){
            FUNC_LINE(0);
            pack_buf[0] = '\0';
            auto len = strlen_P(str) - 1;
            auto readed = getMqttResponse();
            if ( readed < len ) return false;
            return ( strncmp_P(pack_buf, str, len ) == 0 );
        };


        bool waitHello(unsigned long ms=MQTT_WAIT_RESPONSE ){ 
            auto res = ( getMqttResponse(ms) >= constLength(helloStr) );
            if ( res ) {
                res = ( strncmp_P( pack_buf, helloStr, constLength(helloStr) ) == 0 );
            }
            FUNC_LINE(res ? F_TRUE : F_FALSE );
            return res;
        };
        void sendGateId(){ 
            size_t sended=0;
            if( connected ) {
              sended = mqtt.println( gate_id ); 
            }
            FUNC_LINE( (String(F("Send ")) + sended + F(" bytes")).c_str() );
        };

        // bool isDigit(const char c ) const {
        //     return !(c<'0' || c>'9');
        // };
        bool magicResponce(){
            bool res = false; 
            auto receved = getMqttResponse();
            if( receved >= 15 ){
                res = !res;
                int i = 0;
                int j = 1;
                //bool valid = true;
                while(( pack_buf[i])){
                    if ( pack_buf[i] != gate_id[j]){
                        res = !res;
                        break;
                    }
                    i++; j += 2; // каждый второй знак gate_id
                }
             }
             pack_buf[0]='\0';
            debugPrintf("%s\n", res ? F("Succes") : F("Fail"));
            return res;
        };
        public:
        WiFiClient * getMqttP() { return &mqtt; };
        static void gateCallback(String& s){
            //Serial.println(s);
            FUNC_LINE( s );
        };        
        static String gateState(String& s){
            s += "on";
            FUNC_LINE(s);
            return s;
        };
 /// @brief  gate к 4duk по mqtt. 
/// @param id = id шлюза 4duk
/// @param  mdnsName mdns_name.local - имя шлюза в лосальной сети
/// @param  thisGate device для этого шлюза
/// @note Если Gate существует, то устройства общаются через него, 
    /// иначе по udp через внешний роутер 
        Gate(const char * id, const char * mdnsName, DeviceT&);
        ~Gate();
        bool hello();
        bool is_connected();
        bool connect();
        bool sendStatus( int j = -1 );
        bool sendStatus( const char *name, const char* actionName=nullptr,  const char* state=nullptr );
        bool sendStatus( DeviceT& device, const char* actionName=nullptr,  const char* state=nullptr);
        bool sendPing( unsigned long  period=110000 );
        bool tick();
        void addDevice(const DeviceT& );
        bool updateDevice(const DeviceT&  );
    };

};

