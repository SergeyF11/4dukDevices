//#include <4duk.h>
#include <relay.h>
#include <my4duk.h>

/// Это мои настройки
#define MY_CREDENTIAL
#ifdef MY_CREDENTIAL
    #define DUK_ID_VOROTA1
    #include <my.h>
#else
/// это ваши настройки
#define ID_4DUK "Your_4duk_ID"
#define WIFI_SSID "Your  WiFi name" 
#define WIFI_PASS "Your WiFi password"
bool myWiFiConnect(){
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  auto startMs = millis();
  while (WiFi.status() != WL_CONNECTED) {
    if ( (millis() -startMs) >= 10000 ) return false;
    if ( Serial ) Serial.print('.');
    delay(500); 
  }
  return true;
};
#endif


extern Duk::DevicesT devices;

/// @brief создаем устройство для самого шлюза
Duk::Action on( "on", Duk::Gate::gateCallback);
Duk::Action off( "off", Duk::Gate::gateCallback);
Duk::Actions actions( "on_off", Duk::ActionsT{on,off} );    
Duk::DeviceT thisGate( Duk::DEV_THISGATE, 
    "thisgate", "gate", 
    "Home", "my", "my@mail.ru" , 
    0.0, 
    actions, Duk::Gate::gateState );
///=================================================

/// @brief создаём шлюз 
Duk::Gate dukRouter( ID_4DUK, "espGate", thisGate );

Relay relay(LED_BUILTIN,0);


String state( Relay& r ){ 
    return String( r.getState(true) ? "on" : "off");
};
void relayOn(String& s){ relay.on(); };

Duk::Action vorotaOpen( "on", relayOn );
Duk::Actions vorotaActions( "on_off", Duk::ActionsT{vorotaOpen } );
callstat vorotaState( [](String& s){ return s+state(relay); } );

Duk::DeviceT vorota( Duk::DEV_OPENABLE, 
    "vorota1", "opener", "Cherry Gate",
    "my", "my@mail.ru", 
    0.0, 
    vorotaActions, vorotaState );

void Relay::onChange() const {
    vorota.sendStatus( dukRouter );
};


void setup()
{
    
    Serial.begin(115200);
    do { 
        delay(300);
    } while( !Serial);
    Serial.println();

    relay.on();
    devices.push_back( vorota );

    
    if( ! myWiFiConnect() ){
        Serial.println("ERROR: wifi connection");
        return;
    } else {
        Serial.println("Connected WiFi");
    }
    relay.off();
    relay.setAutoOff(4000);
    dukRouter.connect();
    dukRouter.hello();
    dukRouter.sendStatus();
    //devices[0].sendStatus( gate );
}

void loop()
{
    relay.tick();
    if (!dukRouter.is_connected()) {
        Serial.print(F("Reconnect "));
        dukRouter.connect();
        Serial.println( dukRouter.is_connected() ? F("success") : F("fail"));
        //gate.hello();
    }
    if ( dukRouter.tick() ){      
        Serial.println("Ping");
    }
    //delay(2000);
    delay(200);
}
