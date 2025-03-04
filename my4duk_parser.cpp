// my4duk.cpp

#include <WiFiUdp.h>
#include <WiFiClient.h>

#if defined(ESP32)
#include <ESPmDNS.h>
#elif defined(ESP8266)
#include <ESP8266mDNS.h>
#endif

#include "my4duk.h"


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

