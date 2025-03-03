#pragma once

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include "my4duk_device.h"

#define  constLength(array) (((sizeof(array))/(sizeof(array[0])))-1)

namespace Duk {

class UdpT {
    public:
    struct Sender{
        IPAddress ip;
        unsigned int port;
    };
    unsigned int port;
    static char packetBuffer[512];
    static char replyBuffer[256];
    static IPAddress bcast;
    WiFiUDP udp;
    bool started= false;
    static unsigned long pingPeriod;
    
    UdpT(WiFiUDP& udp, unsigned int port = 9009) :
        udp(udp), port(port){
            begin();
        };
    bool isStarted(){ return started; };
    bool begin(){ 
        pingPeriod = 60000;
        bcast = WiFi.broadcastIP();
        started = udp.begin(port); 
        return started;
    };
    int hasPacket(){ return udp.parsePacket(); };
    int readPacket() {
        auto size = udp.read(packetBuffer, constLength(packetBuffer));
        packetBuffer[size]='\0';
        return size;
    };
    int sendReply(Sender& s ){
        size_t size=0; 
        if ( udp.beginPacket(s.ip, s.port ) ){  
            size = udp.write(replyBuffer);
            if ( udp.endPacket() == 0 ) size=0;
        }
        return size;
    };
    static unsigned long lastPing;
    void sendPing(const char * txt="OK"){
        udp.beginPacket(bcast, port);
        udp.println(txt);
        udp.endPacket();
        lastPing= millis();
    };
    void setPingPeriod(unsigned long ms){
        pingPeriod = ms;
    };
    void restoreBuffer(const size_t size){
        for( size_t i=0; i<size; i++){
            if( packetBuffer[i] == '\0' ){
                packetBuffer[i] = ':';
            }
        }
    };
    bool tick(DevicesT devices ){
        bool res;
        int size =  hasPacket();
        if ( size ){
            size = readPacket();
            if ( size > 10 ){
                //parse 
                if ( Duk::Parser::parser(devices, packetBuffer ) == 0 ){
                    // send to mqtt if possible
                    restoreBuffer(size);
                    Serial.printf("Send to mqtt: %s\n", packetBuffer );
                }   
            }
        }
        if( ( millis() - lastPing ) >= pingPeriod ){
            sendPing();
        }

        return res;
    };
};

} //Duk