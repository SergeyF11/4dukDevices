#pragma once
#include <vector>
#include <functional>
#include <IPAddress.h>
#include <WiFiUdp.h>
#include <WiFiClient.h>
#include "my4duk_device.h"

#define MQTT_WAIT_RESPONSE 3000UL
#define PING_PERIOD_MS 110000UL
#define MQTT_BUFFER_SIZE 1024

//#define MY_4DUK_DEBUG
#define F_TRUE  F("True")
#define F_FALSE F("False")

#ifdef MY_4DUK_DEBUG
#define INIT_DEBUG { while( !Serial){ Serial.begin(115200); delay(500); }}
#define FUNC_LINE(...) INIT_DEBUG { Serial.print(__PRETTY_FUNCTION__); Serial.println(__LINE__); Serial.println(__VA_ARGS__); Serial.flush(); }
#else
#define FUNC_LINE

#endif

#define  constLength(array) (((sizeof(array))/(sizeof(array[0])))-1)

namespace Duk { 
        
        // PROGMEM hello
        static const char helloStr[] PROGMEM = "hello";    
        static const char _4duk_su[] = "4duk.su";

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
        unsigned long ping_period = PING_PERIOD_MS;
        int hasInBuffer = 0;
        #if defined(ESP32)
        uint8_t pack_buf[MQTT_BUFFER_SIZE];

        #elif defined(ESP8266)
        char pack_buf[MQTT_BUFFER_SIZE];
        #endif
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
        void resetBufferSize( const int newSize = 0){
            hasInBuffer = newSize;
            pack_buf[newSize] = '\0';
        };

        bool inline bufStartWith( const char * cmp, const size_t length ){
            size_t len = length ? length : (strlen(cmp)-1);
            return strncmp( pack_buf, "OK", constLength("OK") ) == 0;
        }
        bool inline bufEndsWith( const char * cmp, const size_t length = 0){
            size_t len = length ? length : (strlen(cmp)-1);
            return strncmp ( &pack_buf[hasInBuffer-len], cmp, len ) == 0;
        };

        int trimBuffer(const int size ){
            // int i = size - 1;
            // while( i >= 0 ){ 
            int i;
            for ( i = size-1; i >=0; i--){
                if ( pack_buf[i] > 0x20 ) {
                    break;
                }
                //i--;
            }
            i++;
            resetBufferSize(i);
            return i;
        };

        // bool inline pingTimeout() const { 
        //     return (millis() - cmil > MQTT_WAIT_RESPONSE); 
        // };

        int inline hasMqttResponse() { return mqtt.available(); };
        int mqttReadToBuf(size_t size = 0 );

        unsigned long cmil;
        unsigned long lastOkMqttResponseTime;
        void okResponse(const unsigned ms = 0 ){ 
            resetBufferSize();
            lastOkMqttResponseTime = ms ? ms : millis(); 
            FUNC_LINE( lastOkMqttResponseTime);
        };

        bool checkOk(unsigned long timeoutMs = MQTT_WAIT_RESPONSE ); 
        // {
        //     if ( (millis() - lastOkMqttResponseTime) >= (pingPeriod() + timeoutMs)) return false;

        //     auto size =  mqttReadToBuf();

        //     if  (constLength("OK") <= size ) {
        //         #ifdef DEBUG_PRINTF
        //         debugPrintf("Readed raw %d bytes, ", readed);
        //         printBuf(Serial );
        //         #endif

        //         if ( strncmp( pack_buf, "OK", constLength("OK") ) == 0 ) {
        //             //lastOkMqttResponseTime = millis();
        //             okResponse();
        //             pack_buf[0] = '\0';
        //         }
        //     }
        //     return true;
        // };

        ///void inline resetPing(){ cmil = millis(); };


        int waitMqttResponse( unsigned long ms=MQTT_WAIT_RESPONSE ){ 
            int pack_size=0;
            auto start = millis();
            while ( pack_size == 0 ) {
                if ( ( millis() - start) >= ms ) return pack_size;
                delay(10);
                pack_size=hasMqttResponse();
            }
            return mqttReadToBuf(pack_size);

        };

        bool checkResponse(const char * str, const size_t bufSize, const int size = -1){
            if ( str && bufSize ) {
                size_t len;
                if( size < 0) len = strlen_P(str) - 1;
                else len = size;
                //auto readed = waitMqttResponse();
                if ( bufSize >= len ) 
                    return ( strncmp(pack_buf, str, len ) == 0 );
            }
            return false;
        };
        bool checkResponseP(const char * str, const size_t bufSize, const int size = -1){
            bool res = false;
            if ( str && bufSize ) {
                size_t len;
                if( size < 0) len = strlen_P(str) - 1;
                else len = size;
                //auto readed = waitMqttResponse();
                if ( bufSize >= len ) 
                    res = ( strncmp_P(pack_buf, str, len ) == 0 );
            }
            resetBufferSize();
            return res;
        };

        bool inline waitHello(unsigned long ms=MQTT_WAIT_RESPONSE ){
            auto bufSize = waitMqttResponse(ms); 
            return checkResponseP(helloStr, bufSize, constLength(helloStr));
        };
        void inline sendGateId(){ 
            size_t sended=0;
            if( connected ) {
              sended = mqtt.println( gate_id ); 
            }
            FUNC_LINE( (String(F("Send ")) + sended + F(" bytes")).c_str() );
        };

        bool magicResponce();

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
        bool isConnected() const { return connected; };
        bool connect(const char *site=_4duk_su, const uint port=9009);
        bool inline connect(const uint port ) { return connect(_4duk_su, port); };
        bool sendStatus( int j = -1 );
        bool sendStatus( const char *name, const char* actionName=nullptr,  const char* state=nullptr );
        bool sendStatus( DeviceT& device, const char* actionName=nullptr,  const char* state=nullptr);
        void pingPeriod(const unsigned long period) { ping_period = period; };
        unsigned long pingPeriod() const { return ping_period; };
        bool sendPing();
        bool tick();
        void addDevice(const DeviceT& );
        bool updateDevice(const DeviceT&  );
    };

};

