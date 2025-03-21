

#include <my4duk.h>
#include <GateControl.h>
#include "rgb_led.h"
RGBled rgbLed;

#define MS_NEXT_MQTT_SEND 0
#define BARRIER_PROCESSING_TIME_MS  10000UL
#define BARRIER_OPEN_STATE_TIME_MS  20000UL
#define BARRIER_PIN_CONTROL         LED_BUILTIN
#define BARRIER_PIN_ACTIVE_LEVEL    LOW


/// Это мои настройки
#define MY_CREDENTIAL
#ifdef MY_CREDENTIAL
    #define DUK_ID_VOROTA2
    #include <my.h>
#else
/// это ваши настройки
// закомментируйте строку '#define MY_CREDENTIAL' для своих настроек
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
#define FUNC_LINE
#define ESP_MEMORY_PRINT
#endif

extern Duk::DevicesT devices;

TimedControlGate gate(BARRIER_PIN_CONTROL, BARRIER_PIN_ACTIVE_LEVEL , 
        BARRIER_PROCESSING_TIME_MS,  BARRIER_OPEN_STATE_TIME_MS );


void openGate(String& s){

   Serial.println(__PRETTY_FUNCTION__);
   Serial.println(s);
   gate.startOpen();
};
void openOnRange(String& s){
    Serial.println(__PRETTY_FUNCTION__);
    Serial.println(s);

    if( s.toInt() > 0 )
        gate.startOpen();
};

Duk::Action vorotaOpen( "on", openGate );
Duk::Action vorotaRanger( 0, 100, openOnRange );
Duk::Actions vorotaOnOff( "on_off", Duk::ActionsT{ vorotaOpen } );
Duk::Actions vorotaRange( "range", Duk::ActionsT{ vorotaRanger } );

callstat vorotaState( [](String& s){ 
    s.concat( gate.getOpenedPercent() );
    return s;
} );

Duk::DeviceT vorota( Duk::DEV_THISGATE,  //DEV_OPENABLE, 
    "thisgate", "thisgate", 
    "Home", "my", "my@mail.ru" , 
    0.0,
    vorotaRange, vorotaState ); 
    //vorotaOnOff, [](String& s){ return s+"on"; } );

Duk::Gate dukRouter( ID_4DUK, "thisgate", vorota );


void setup(){

    Serial.begin(115200);
    do { 
        delay(500);
    } while( !Serial);
    Serial.println();


    rgbLed.color(RGBled::Blue);

    if( ! myWiFiConnect() ){
        Serial.println("ERROR: wifi connection");
        ESP.restart();
        return;
    } else {
        Serial.println("Connected WiFi");
        rgbLed.color(RGBled::Cyan);
    }
    

    vorota.addCapabilities( vorotaOnOff );
    dukRouter.updateDevice( vorota );
    
    Serial.println("======== device Vorota ==================");
    vorota.printTo(Serial);


    gate.setOnChanged(  []{ 
        Serial.print("Send status");
        static auto lastState = GateBase::StatusE::Unknown;
        static bool sendOpened = false;
        switch( gate.getStatus()){
            case GateBase::StatusE::Closed:
                if ( lastState != GateBase::StatusE::Closed ){
                    lastState = GateBase::StatusE::Closed;
                    sendOpened = false;
                    vorota.sendStatus( dukRouter, "range", 0);
                    delay(MS_NEXT_MQTT_SEND);
                    vorota.sendStatus( dukRouter, "on_off", "off");
                }
                break;
            case GateBase::StatusE::Open:
                if ( lastState != GateBase::StatusE::Open ){
                    lastState = GateBase::StatusE::Open;
                    vorota.sendStatus( dukRouter, "range", 100);
                    delay(MS_NEXT_MQTT_SEND);
                    vorota.sendStatus( dukRouter , "on_off", "on");
                    
                }
                break;
            case GateBase::StatusE::Processing:
                if ( !sendOpened ){
                    vorota.sendStatus( dukRouter, "on_off", "on");
                    sendOpened = true;
                } else { 
                    vorota.sendStatus( dukRouter );
                }
                lastState = GateBase::StatusE::Processing;
        }


    }, 25 );

    if ( dukRouter.connect( ) ) rgbLed.color(RGBled::Yellow);
    if ( dukRouter.hello() ) rgbLed.color(RGBled::Blue);
    if ( dukRouter.sendStatus(vorota, "on_off", "off")) rgbLed.color(RGBled::Green);
    delay(MS_NEXT_MQTT_SEND);
    if ( dukRouter.sendStatus(vorota, "range", 0) ) rgbLed.color(RGBled::Green);
    else rgbLed.color(RGBled::Red);

    ESP_MEMORY_PRINT(0);

}

void loop()
{

    gate.tick();

    if (!dukRouter.isConnected()) {
        rgbLed.color( RGBled::Red);
        Serial.print(F("Reconnect "));
        dukRouter.connect();
        Serial.println( dukRouter.isConnected() ? F("success") : F("fail"));
        dukRouter.hello();
    } else {
        rgbLed.color( RGBled::Green);
    }
    if ( dukRouter.tick() ){      
        rgbLed.color(RGBled::Blue);
        Serial.println("Tick processed");
    }

    delay(200);
    ESP_MEMORY_PRINT(3000);
}

