// my4duk.cpp

#include <WiFiUdp.h>
#include <WiFiClient.h>

#if defined(ESP32)
#include <ESPmDNS.h>
#elif defined(ESP8266)
#include <ESP8266mDNS.h>
#endif

#include "my4duk.h"

Duk::DevicesT devices;

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

void Duk::Gate::addDevice(const DeviceT &newDev)
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

bool Duk::Gate::is_connected()
{
    if (connected)
    {
        // Serial.print("Connected");
    }
    else
    {
        FUNC_LINE(F("NOT Connected"));
    }
    return connected;
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
        for (unsigned int i = 0; i < devices.size(); i++)
        {
            if (devices[i].type == Duk::DEV_THISGATE)
            {
                devices[i].sendPing(*this); //&(this->mqtt) );
                cmil = millis();
                res = true;
            }
        }
        if (res &&
            (getMqttResponse(1000) > 0))
        {
            FUNC_LINE(pack_buf);
            pack_buf[0] = '\0';
        }
    }
    return true;
};

bool Duk::Gate::sendStatus(const char *name, const char *actionName, const char *state)
{
    for (auto device : devices)
    {
        if (device.nameEquals(name))
        {
            device.sendStatus(*this, actionName, state );
            cmil = millis();
            return true;
        }
    }
    return false;
};
bool Duk::Gate::sendStatus(DeviceT &device, const char *actionName, const char *state)
{
    device.sendStatus(*this, actionName, state);
    cmil = millis();
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
        cmil = millis();
        res = true;
    }
    else
    {
        if (j >= 0 && j < devices.size())
        {
            FUNC_LINE("Update status one device");
            devices[j].sendStatus(*this);
            cmil = millis();
            res = true;
        }
    }
    return res;
};

String Duk::DeviceT::formStatusStr(String &inOut, const char* state )
{
    debugPrintf("String '%s' %s\n",inOut.c_str(), state ? state : "callstate");
    
    String s(F("devstate:"));
    s += name;
    s += ':';
    bool ok = false;
    if (!inOut.isEmpty())
    {
        for (auto actions : capabilities.skills)
        {
            if (actions.nameEquals(inOut.c_str()))
            {
                s += actions.name;
                ok = true;
                break;
            }
        }
    }
    if (!ok)
    {
        s += capabilities[0].name;
    }
    s += ':';
    if ( state ) s += state;
    else if (capabilities.callstate != nullptr)
        s = capabilities.callstate(s);
    else
        s += F("on");
    inOut = s;
    FUNC_LINE(s.c_str());
    return inOut;
};
void Duk::DeviceT::sendStatus(const char *actionName, const char *state)
{
    if (capabilities.callstate != nullptr)
    {
        String s1(actionName);
        formStatusStr(s1, state );
        // udp.send(s1);
        FUNC_LINE("Send to udp.");
    }
};
// void Duk::DeviceT::sendStatus( const char * actionName ){
//     for ( auto action : capabilities ){
//         if ( action.nameEquals( actionName) &&
//             callstate != nullptr ) {
//             String s1;
//             formStatusStr(s1);
//             //udp.send(s1);
//             FUNC_LINE("Send to udp.");
//         }
//         break;
//     }
// };
void Duk::DeviceT::sendPing(Duk::Gate &gate)
{
    if (gate.is_connected())
    {
        gate.getMqttP()->println(F("PING"));
        FUNC_LINE("Send to mqtt.");
    }
};
void Duk::DeviceT::sendStatus(Duk::Gate &gate, const char *actionName, const char *state)
{
    if (gate.is_connected())
    {
        String s1(actionName);
        formStatusStr(s1, state);

        auto sended = gate.getMqttP()->println(s1);
        //FUNC_LINE("Send to mqtt.");
        debugPrintf("Send mqtt: %s\n%d bytes sended\n", s1.c_str(), sended);
    }
};


bool Duk::DeviceT::nameEquals(const char *n) const
{
    return (strcmp(name, n) == 0);
};
// Duk::Actions Duk::DeviceT::getByName(const char * n) const{
// };

size_t Duk::DeviceT::printTo(Print& p) const {
    size_t out =0;
    out = p.printf("Name:'%s', alias:'%s', room:'%s', %s\n", 
                    name, alias, room, is_real ? "" : "hidden," );
    out += capabilities.printTo(p);

    return out;
};

void Duk::DeviceT::sendStatus(WiFiClient *mqtt, const char *actionName, const char *state)
{
    String s1(actionName);
    formStatusStr(s1, state );
    FUNC_LINE((String(F("Status - ")) + s1));
    if (mqtt != nullptr)
    {
        mqtt->println(s1);
        FUNC_LINE("Send to mqtt.");
    }
    else
    {
        // udp.send(s1);
        FUNC_LINE("Send to udp.");
    }
};

namespace Duk
{
    namespace Devices
    {
        ParsedData::ParsedData() : name(nullptr), action(nullptr), value(nullptr) {};
        bool ParsedData::valueEquals(const char *n) const
        {
            return (strcmp(value, n) == 0);
        };
        bool ParsedData::nameEquals(const char *n) const
        {
            return (strcmp(name, n) == 0);
        };
        bool ParsedData::nameEquals(const DeviceT &dev) const
        {
            if (!name || !dev.name)
                return false;
            // debugPrintf("compare names '%s' == '%s'\n", name, dev.name );
            return (strcmp(name, dev.name) == 0);
        };
        bool inline ParsedData::actionEquals(const char *actionName) const
        {
            debugPrintf("Check '%s'\n", actionName);
            return (strcmp(action, actionName) == 0);
        };
        bool inline ParsedData::actionEquals(const Actions &acts) const
        {
            return actionEquals(acts.name); //( strcmp(action, acts.name) == 0 );
        };

        // bool inline ParsedData::actionEquals(const DeviceT& dev) const {
        //     return actionEquals( dev.capabilities );
        // };

        std::unique_ptr<Duk::Action> ParsedData::getActionsEquals(const DeviceT &dev) const
        {
            return getActionsEquals(dev.capabilities);
        };
        std::unique_ptr<Duk::Action> ParsedData::getActionsEquals(const Capabilities &capabilities) const
        {

            for (auto cap : capabilities.skills)
            {
                if (actionEquals(cap))
                {
                    debugPrintf("Ok. return list '%s'\n", action);
                    auto t = std::make_unique<Action>(*cap.list.data());

                    t->printTo(Serial);

                    return t; //&(cap.list);
                }
            }
            debugPrintf("Not found '%s'\n", action);
            return nullptr;
        };

        callback ParsedData::getCallback(const DeviceT &dev) const
        {
            debugPrintf("+++++++++++++ Capabilities ++++++++++++++\n");
            dev.capabilities.printTo(Serial);
            if (action && value)
            {
                debugPrintf("search action '%s'\n", action);

                auto devAction = getActionsEquals(dev.capabilities);
                if (devAction != nullptr)
                {
                    // for ( auto devActions : dev.capabilities.skills ){
                    //     debugPrintf("\t'%s'\n", devActions.name );
                    //     if ( actionEquals( devActions )) {
                    debugPrintf("Action '%s': OK\n", action);
                    devAction->printTo(Serial);
                    // debugPrintf("search callback for '%s' in %d records\n", value, (*list).size() );
                    //        for( auto devAction : devActions.list ){
                    //    for( auto devAction : (*list) ){
                    if (devAction->value)
                    {
                        if (valueEquals(devAction->value))
                        {
                            // debugPrintf("Value %s: OK\n", value );
                            debugPrintf("Callback %s\n", devAction->cb ? "Ok" : "null");
                            return devAction->cb;
                        }
                    }
                    else
                    {
                        if (!devAction->rangeValues->outOfRange(value))
                        {
                            debugPrintf("Callback %s\n", devAction->cb ? "Ok" : "null");
                            return devAction->cb;
                        }
                    }
                    // }
                    //}
                }
                else
                {
                    debugPrintf("Nullptr returned");
                }
            }
            return nullptr;
        };

        bool ParsedData::valid() const
        {
            return !(value == nullptr || action == nullptr || name == nullptr);
        };
        bool ParsedData::callCB(const Duk::DevicesT &devices) const
        {
            // for( int i=0; i<devices.size(); i++){
            for (auto device : devices)
            {
                debugPrintf("check device '%s'\n", device.name);
                if (nameEquals(device) /* && actionEquals(device) */)
                {
                    callback cb = getCallback(device);
                    String v(value);
                    if (cb != nullptr)
                    {
                        cb(v);
                        return true;
                    }
                }
            }
            return false;
        };
        ParsedData::ParsedData(const char *name, const char *action, const char *value) : name(name), action(action), value(value) {};

    }
}
namespace Duk
{
    namespace Parser
    {

        d_Ptrs &operator++(d_Ptrs &orig)
        {
            if (orig < Owersize)
                orig = static_cast<d_Ptrs>(orig + 1);
            return orig;
        };

        bool operator--(d_Ptrs &orig)
        {
            if (orig > _Device)
            {
                orig = static_cast<d_Ptrs>(orig - 1);
                return true;
            }
            return false;
        };
    }
};

// уничтожает исходный buf операцией strtok_r
Duk::Devices::ParsedData Duk::Parser::parser(char *buf)
{
    auto size = strlen(buf);
    if (size > 10)
    {
        char *tokens[d_Ptrs::Owersize] = {nullptr};
        char *rest = buf;
        auto ptr = d_Ptrs::_Device;
        while ((tokens[ptr] = strtok_r(rest, ":", &rest)))
        {
            ++ptr;
            if (ptr == d_Ptrs::Owersize)
                break;
        }
        if (tokens[d_Ptrs::Value] != nullptr)
        {
            FUNC_LINE(String(tokens[d_Ptrs::Name]) + ':' +
                      tokens[d_Ptrs::Action] + ':' +
                      tokens[d_Ptrs::Value]);
            return Duk::Devices::ParsedData(
                tokens[d_Ptrs::Name],
                tokens[d_Ptrs::Action],
                tokens[d_Ptrs::Value]);
        }
    }
    return Duk::Devices::ParsedData();
};

uint Duk::Parser::parser(const Duk::DevicesT &devices, char *buf)
{
    uint count = 0;
    auto parsedData = parser(buf);
    if (parsedData.valid() &&
        parsedData.callCB(devices))
    {
        count++;
    }
    return count;
};
