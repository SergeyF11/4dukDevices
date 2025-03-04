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
    if (gate.isConnected())
    {
        gate.getMqttP()->println(F("PING"));
        FUNC_LINE("Send to mqtt.");
    }
};
void Duk::DeviceT::sendStatus(Duk::Gate &gate, const char *actionName, const char *state)
{
    if (gate.isConnected())
    {
        String s1(actionName);
        formStatusStr(s1, state);

        auto sended = gate.getMqttP()->println(s1);
        //FUNC_LINE("Send to mqtt.");
        debugPrintf("Send mqtt: %s\n%d bytes sended\n", s1.c_str(), sended);
    }
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

