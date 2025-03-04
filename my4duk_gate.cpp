// my4duk.cpp

#include <WiFiUdp.h>
#include <WiFiClient.h>

#if defined(ESP32)
#include <ESPmDNS.h>
#elif defined(ESP8266)
#include <ESP8266mDNS.h>
#endif

#include "my4duk.h"

extern Duk::DevicesT devices;

// bool Duk::endStr(const char s) {
//     return ( s <= 0x20 );
// };

/// @brief  gate к 4duk по mqtt.
/// @param id = id шлюза 4duk
/// @param  mdnsName mdns_name.local - имя шлюза в лосальной сети
/// @param  thisGate device для этого шлюза
/// @note Если Gate существует, то устройства общаются через него,
/// иначе по udp через внешний роутер
Duk::Gate::Gate(const char *id, const char *mdnsName, DeviceT &thisGate) : 
    gate_id(strdup(id)), mdns_name(strdup(mdnsName))
{
    connected = false;
    devices.push_back(thisGate);
};
Duk::Gate::~Gate() {
};

void inline Duk::Gate::addDevice(const DeviceT &newDev)
{
    devices.push_back(newDev);
};


bool Duk::Gate::updateDevice(const DeviceT &newDevice)
{
    auto name = newDevice.name;
    for (DeviceT &dev : devices)
    {
        if (dev.nameEquals(newDevice.name))
        {
            debugPrintf("Update device '%s'\n", newDevice.name);
            dev.capabilities.skills.clear();
            dev.capabilities.skills.shrink_to_fit();
            dev.capabilities = newDevice.capabilities;
            //dev.callstate = newDevice.callstate;

            debugPrintf("New device capabilities:\n");
#ifdef DEBUG_PRINTF
            dev.capabilities.printTo(Serial);
#endif
            return true;
        }
    }
    return false;
};

bool Duk::Gate::connect()
{
    if (!connected)
    {
        int nreq = 10;

        FUNC_LINE(F("Connecting mqtt....."));
        while (mqtt.connect("4duk.su", 9009) == 0 && nreq > 0)
        {
            FUNC_LINE(F("Error connect to mqtt....."));
            nreq--;
        }
        if (nreq == 0)
        {
            FUNC_LINE(F("Error connect to mqtt. MQTT disable."));
        }
        else
        {
            connected = true;
        }
        mqtt.println(gate_id);
        if (MDNS.begin(mdns_name))
        {
            Serial.printf("Start MDNS %s.local on this device.\n", mdns_name);
        }
        else
        {
            FUNC_LINE(F("ERROR: mdns start"));
        }
    }
    return connected;
};

bool Duk::Gate::hello()
{
    auto res = waitHello();
    if (res)
    {
        sendGateId();
        res = magicResponce();
    }
    // connected = res;
    return res;
};

bool Duk::Gate::tick()
{
#if defined(ESP8266)
    MDNS.update();
#endif
    int pack_size = mqtt.available();
    if (pack_size)
    {
        mqtt.read(pack_buf, pack_size);
        pack_size = trimBuffer(pack_size);
        // #if defined(ESP32)
        //     String r_pack=String(pack_buf,pack_size);
        // #elif defined(ESP8266)
        //     String r_pack=String(pack_buf);
        // #endif
        FUNC_LINE(pack_buf);
        Duk::Parser::parser(devices, pack_buf);
    }
    sendPing();
    return (pack_size > 0);
};

/// @brief send `PING` for all DEV_THISGATE devices each period in ms
/// @param period in ms
/// @note calls from tick() automaticle
/// @return true if DEVskills_THISGATE devices present
bool Duk::Gate::sendPing(unsigned long period)
{
    bool res = false;
    if ((millis() - cmil) > period)
    {
        FUNC_LINE("Send PING");
        for( auto dev : devices )
        {
            if( dev.is(Duk::DEV_THISGATE) )
            {
                dev.sendPing(*this);
                resetPing(); //cmil = millis();
                res = true;
                break;
            }
        }

        if (res &&
            (getMqttResponse(1000) > 0))
        {
            FUNC_LINE(pack_buf);
            pack_buf[0] = '\0';
        }
    }
    return res;
};

bool Duk::Gate::sendStatus(const char *name, const char *actionName, const char *state)
{
    for (auto device : devices)
    {
        if (device.nameEquals(name))
        {
            device.sendStatus(*this, actionName, state );
            resetPing(); //cmil = millis();
            return true;
        }
    }
    return false;
};
bool Duk::Gate::sendStatus(DeviceT &device, const char *actionName, const char *state)
{
    device.sendStatus(*this, actionName, state);
    resetPing(); //cmil = millis();
    return true;
};

bool Duk::Gate::sendStatus(int j)
{
    bool res = false;
    if (j == -1)
    {
        FUNC_LINE("Update status all devices");
        for (auto device : devices)
        {
            device.sendStatus(*this);
        }
        // for ( unsigned int i=0; i<devices.size(); i++) {
        //     devices[i].sendStatus( *this );
        // }
        resetPing(); //cmil = millis();
        res = true;
    }
    else
    {
        if (j >= 0 && j < devices.size())
        {
            FUNC_LINE("Update status one device");
            devices[j].sendStatus(*this);
            resetPing(); //cmil = millis();
            res = true;
        }
    }
    return res;
};

bool Duk::Gate::magicResponce(){
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

int Duk::Gate::mqttReadToBuf(size_t size ) {
    int readed = 0;
    //auto size = hasMqttResponse();
    if ( size == 0 ) size = hasMqttResponse();
    if ( size != 0 ) {
        readed = mqtt.read(pack_buf, size);
        //pack_buf[readed] = '\0';
        #ifdef DEBUG_PRINTF
        debugPrintf("Readed raw %d bytes, ", readed);
        printBuf(Serial );
        #endif

        readed = trimBuffer(readed);
    }
    return readed; 
};