#pragma once
#include <vector>
#include <functional>
#include <IPAddress.h>
#include <WiFiUdp.h>
#include <WiFiClient.h>

    /// @brief определение типа для функций обратного вызова действия устройства
    typedef std::function<void(String &s1)> callback;
    /// @brief определение типа для функций обратного вызова статуса устройства
    typedef std::function<String(String &s1)> callstat;

namespace Duk {

    class Gate;

    /// @brief нумератор типа устройства 4duk
    typedef enum {
        DEV_CAMERA=240,
        DEV_COOKING=250,
        DEV_COOKING_COFFEE_MAKER=251,
        DEV_COOKING_KETTLE=252,
        DEV_COOKING_MULTICOOKER=253,
        DEV_DISHWASHER=260,
        DEV_HIMIDIFIER=270,
        DEV_IRON=280,
        DEV_LIGHT=100,
        DEV_LIGHT_CEILING=101,
        DEV_LIGHT_LIGHT_STRIP=102,
        DEV_MEDIA_DEVICE=110,
        DEV_MEDIA_DEVICE_RECEIVER=111,
        DEV_MEDIA_DEVICE_TV=112,
        DEV_MEDIA_DEVICE_TV_BOX=113,
        DEV_OPENABLE=120,
        DEV_OPENABLE_CURTAIN=121,
        DEV_OTHER=130,
        DEV_PET_DRINKING_FONTAIN=140,
        DEV_PET_FEEDER=141,
        DEV_PURFIER=150,
        DEV_SENSOR=160,
        DEV_SENSOR_BUTTON=161,
        DEV_SENSOR_CLIMATE=162,
        DEV_SENSOR_GAS=163,
        DEV_SENSOR_ILLUMINATION=164,
        DEV_SENSOR_MOTION=165,
        DEV_SENSOR_OPEN=166,
        DEV_SENSOR_SMOKE=167,
        DEV_SENSOR_VIBRATION=168,
        DEV_SENSOR_WATER_LEAK=169,
        DEV_SMART_METER=170,
        DEV_SMART_METER_COLD_WATER=171,
        DEV_SMART_METER_ELECTRICITY=172,
        DEV_SMART_METER_GAS=173,
        DEV_SMART_METER_HEAT=174,
        DEV_SMART_METER_HOT_WATER=174,
        DEV_SOCKET=180,
        DEV_SWITCH=190,
        DEV_TERMOSTAT=200,
        DEV_TERMISTAT_AC=201,
        DEV_VACUUM_CLEANER=210,
        DEV_VENTILATION_FAN=220,
        DEV_WASHING_MACHINE=230,
        DEV_THISGATE=300,
    } DeviceTypeT;

    struct Creator{
        const char *name;
        const char *mail;
        Creator(const char *name, const char *mail) :
            name(strdup(name)), mail(strdup(mail))
        {};
    };

    /// @brief описание структуры функции callback для действия устройства
    struct Action {
        const char* value;
        typedef struct {
            const int minVal;
            const int maxVal;
            bool outOfRange(const int val) const {
                return val < minVal || val > maxVal;
            };
            bool inline outOfRange(const char * str) const {
                return outOfRange( atoi(str) );
            }
        } RangeVal;
        const RangeVal * rangeValues;
        callback cb;
        /// @brief конструктор функции обратного вызова для действия со значением val
        /// @param val 
        /// @param cb функции обратного вызова для значения val
        Action(const char * val, callback cb) :
            value(strdup(val)), cb(cb), rangeValues(nullptr) 
        {};
        
        Action(const int minVal, const int maxVal, callback cb) :
            value(nullptr), cb(cb), rangeValues(new RangeVal{minVal, maxVal})
        {};
        Action( const Action& src) :
            value(src.value), cb(src.cb), 
            rangeValues( src.rangeValues ? 
                new RangeVal{src.rangeValues->minVal, src.rangeValues->maxVal} : nullptr)
            {};
        ~Action(){
            // delete value;
            // value = nullptr;
            delete rangeValues;
            rangeValues = nullptr;
        };
        bool valueEquals( const char * val) const {
            if ( value ) {
                return( strcmp(value, val) == 0 );
            } else {
                return valueInRange(val);
            }
        };
        bool valueInRange( const char * val, int* valueP = nullptr) const {
            int value;
            if ( ! rangeValues ||
                 sscanf("%d", val, &value ) != 1 ||
                 rangeValues->outOfRange(value) ) return false;
            if ( valueP ) *valueP = value;
            return true;
        }; 

        size_t printTo(Print& p) const {
            size_t out = p.print("type "); 
            if ( value ){
                out += p.print("Value \'");
                out += p.print( value );
                out += p.print('\'');
            } else {
                out += p.print("Range: ");
                out += p.print( rangeValues->minVal );
                out += p.print("-");
                out += p.print( rangeValues->maxVal );
                
            }
            out += p.print(" cb:");
            out += p.println( cb ? "OK" : "null" );
            return out;
        };
    };
   
        // /// @brief описание структуры функции callback для действия устройства
        // struct Action {
        //     const char* value;
        //     callback cb;
        //     /// @brief конструктор функции обратного вызова для действия со значением val
        //     /// @param val 
        //     /// @param cb функции обратного вызова для значения val
        //     Action(const char * val, callback cb) :
        //         value(strdup(val)), cb(cb)
        //     {};
        //     ~Action(){
        //         delete value;
        //     };
        //     bool valueEquals( const char * val) const {
        //         if ( value ) {
        //             return( strcmp(value, val) == 0 );
        //         } else {
        //           //  return valueInRange(val);
        //           return false;
        //         }
        //     };
        // };
    

    /// @brief список Action
    typedef std::vector<Action> ActionsT;
    /// @brief описывает свойство actions устройства и список значений для него
    struct Actions {
        const char * name;
        ActionsT list;
        /// @brief определяет actions со списком действий устройства
        /// @param name 
        /// @param list 
        Actions(const char * name, const ActionsT& list) :
            name(strdup(name)), list(list) 
            {};
        Actions(const char * name, const Action& action ):
            Actions(name, ActionsT{ action }){};

        bool nameEquals(const char * n) const {
            return ( strcmp(name, n) == 0 );
        };
        ~Actions(){ 
            //delete[] name;
            // name = nullptr;
        };
    };

    typedef std::vector<Actions> CapabilitiesT;
    struct Capabilities {
        CapabilitiesT skills;
        callstat callstate;
        Capabilities(const Actions& actions, callstat cs = nullptr) :
            skills( CapabilitiesT{actions} ), callstate(cs)
            {};
        Capabilities(const Capabilities& src) :
            skills(src.skills), callstate(src.callstate)
        {};
        ~Capabilities() = default;
        // auto begin() const { return skills.begin(); };
        // auto end() const { return skills.end(); };
        const Actions& operator[](std::size_t idx) const { return skills[idx]; };
        const ActionsT& getActions(const char * name) const {
            for ( auto acts : skills ){
                if( acts.nameEquals( name ) ) return acts.list;
            }
            return ActionsT();
        };
        size_t printTo(Print& p) const {
            size_t out;
            for ( auto acts : skills ){
                out += p.printf("%s: ", acts.name);
                for( auto a : acts.list ){
                    out += a.printTo(p);
                }
            }
            out += p.print("callstate: ");
            out += p.println( callstate ? "OK" : "null");
            return out;
        };
    };

    #define HIDDEN_DEVICE true
    ///// @brief описание устройства в 4duk
    struct DeviceT{
        const DeviceTypeT type;
        const char *name;
        const char *alias;
        const char *room;
        const Creator creator;
        const float version;
        const bool is_real=true;
        //Actions actions;
        Capabilities capabilities;
        //callstat callstate;
        /// @brief задает устройство в 4duk
        /// @param type тип enum DeviceTypeT
        /// @param name имя устройства. 
        /// В пределах одной сети все udp-устройства с одинаковыми именами выполняют команды одновременно
        /// даже если команды приходят через разные шлюзы
        /// @param alias псевдоним устройства (пока не исползуется)
        /// @param room местоположение устройства (пока не исползуется)
        /// @param creatorName имя автора (пока не исползуется)
        /// @param creatorMail e-mail автора (пока не исползуется)
        /// @param ver номер версии (пока не исползуется)
        /// @param actions список действий устройства { name, [ value, callback ] }
        /// @param callstate функция обратного вызова состояния устройства { name, callstate }
        /// @param hidden задает скрытое устройство
        DeviceT( const DeviceTypeT type, const char * name, const char * alias, const char * room, 
            const char * creatorName, const char * creatorMail, 
            const float ver,const Actions& actions, callstat callstate, const bool hidden=!HIDDEN_DEVICE ) :
          type(type), name(strdup(name)), alias(strdup(alias)), room(strdup(room)),
         creator( creatorName, creatorMail), version(ver), is_real(!hidden), 
         capabilities( actions, callstate )//,callstate(callstate)
        { 
                //addCapabilities(actions );
        };
        void inline addCapabilities( const Actions& actions, callstat cs = nullptr){
           capabilities.skills.push_back( actions );
           if ( cs )
            capabilities.callstate = cs;
        };
        String inline formStatusStr(String & s, const char *state=nullptr);
        void sendStatus(WiFiClient * mqtt, const char * = nullptr, const char *state=nullptr);

        void sendStatus( const char * = nullptr, const char * state=nullptr);
        void sendStatus( Gate& gate, const char * actionName= nullptr, const char * state=nullptr);
        void sendStatus( Gate& gate, const char * actionName, const int rangeState){
            sendStatus(gate, actionName, String(rangeState).c_str() );
        };
        void sendPing( Gate& gate);
        bool inline nameEquals(const char * n) const;
        //Actions getByName(const char * n) const;
        ~DeviceT() = default;
        size_t printTo(Print& p) const;
        
    };


    /// @brief описание типа список устройств
    /// @note сейчас ДОЛЖНА использоваться переменная
    /// @note external Duk::DevicesT device;
    typedef std::vector<DeviceT> DevicesT;

    namespace Devices {
        struct ParsedData {
            const char * name;
            const char * action;
            const char * value;
            // два конструктора 

            /// @brief пустой коструктор
            ParsedData();
            /// @brief конструктор с данными
            /// @param name 
            /// @param action 
            /// @param value 
            ParsedData( const char * name, const char *action, const char * value );

        // остальные константные свойства

            /// @brief проверяет что все данные присутствуют
            /// @return 
            bool valid() const;

            /// @brief возвращает каллбэк функцию, соответствующую action:value
            /// @param dev устройство для проверки 
            /// @note action:value значение для вызова каллбэка содержатся в this
            /// @return функцию каллбэка или 
            ///  nullptr, если нет совпадений для value
            //callback getCallback(const DeviceT& dev, const char * value ) const;
            callback getCallback(const DeviceT& dev ) const;
            /// @brief исполняет каллбэк соответствующий полям name, action, value
            /// @param devices список устройств, где были заданы описания устройств { name, { actions[], { callback[] }}
            bool callCB(const Duk::DevicesT& devices ) const;
            
            private:
            /// @brief проверяет совпадение имени устройства
            /// @param  name char *
            /// @return 
            bool nameEquals(const char * ) const;
            /// @brief проверяет совпадение имени устройства
            /// @param  device DeviceT&
            /// @return 
            bool nameEquals(const DeviceT& dev) const;
            bool actionEquals(const char *) const;
            bool actionEquals(const Actions&) const;
            //bool actionEquals(const Capabilities& ) const;
            inline std::unique_ptr<Duk::Action> getActionsEquals(const DeviceT& dev) const;
            std::unique_ptr<Duk::Action> getActionsEquals(const Capabilities& capabilities)  const;
            bool actionEquals(const DeviceT& dev) const;
            bool valueEquals( const char * val) const;
           
        };
    }
    namespace Parser {
        typedef enum  {
            _Device = 0,
            Name,
            _Action,
            Action,
            _Value,
            Value,
            Owersize, 
        } d_Ptrs;
        d_Ptrs& operator++(d_Ptrs& orig);
        bool operator--(d_Ptrs& orig);

        Duk::Devices::ParsedData parser( char * buf ); 
        uint parser(const Duk::DevicesT& devices, char * buf); 
    }
};