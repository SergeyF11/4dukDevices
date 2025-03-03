
//#include <relay.h>
//#include "gateControlTest/GateControl.h"
// #include <GateBase.h>
// #include <TimedGate.h>
#include <my4duk.h>
#include <GateControl.h>
#define MS_NEXT_MQTT_SEND 900

/// Это мои настройки
#define MY_CREDENTIAL
#ifdef MY_CREDENTIAL
    #define DUK_ID_VOROTA2
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
#define FUNC_LINE
#define ESP_MEMORY_PRINT
#endif

extern Duk::DevicesT devices;

TimedControlGate gate(LED_BUILTIN, LOW , 10000UL, 20000UL );
//Relay relay(LED_BUILTIN,0);


// String state( Relay& r ){ 
//     return String( r.getState(true) ? "on" : "off");
//  };
// void relayOn(String& s){ relay.on(); };
// String gateState(GateControl& gate ){
//     return String( gate.get()->getOpenedPercent()); 
// };

// String stateRange(TimedControlGate& gate ){
//     String s;
//     auto percent = gate.getOpenedPercent(); 
//     s+= percent;

//     Serial.printf("%s %d string(%s)\n", __PRETTY_FUNCTION__, __LINE__, s.c_str());
//     return s;
// };

void openGate(String& s){
   // gateP.get()->startOpen();
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

// callstat vorotaStateOnOff( [](String& s){ 
//     s += gate.getStatus() == GateBase::StatusE::Closed ? "off" : "on"; 
//     return s;
// } );
callstat vorotaState( [](String& s){ 
    s.concat( gate.getOpenedPercent() );//stateRange(gate));
    return s;
} );

Duk::DeviceT vorota( Duk::DEV_THISGATE,  //DEV_OPENABLE, 
    "thisgate", "thisgate", 
    "Home", "my", "my@mail.ru" , 
    0.0,
    vorotaRange, vorotaState ); 
    //vorotaOnOff, [](String& s){ return s+"on"; } );

Duk::Gate dukRouter( ID_4DUK, "thisgate", vorota );

// void Relay::onChange() const {
//     vorota.sendStatus( dukRouter );
// };
//gateP.get()->setOn

void setup(){
    //gateP.get()->setOnChanged( []{ vorota.sendStatus( dukRouter ); }, 33 );
    Serial.begin(115200);
    do { 
        delay(500);
    } while( !Serial);
    Serial.println();

    //relay.on();
    
    if( ! myWiFiConnect() ){
        Serial.println("ERROR: wifi connection");
        ESP.restart();
        return;
    } else {
        Serial.println("Connected WiFi");
    }
    

    vorota.addCapabilities( vorotaOnOff );
    dukRouter.updateDevice( vorota );
    
    //new ( &dukRouter ) Duk::Gate( ID_4DUK, "thisgate", vorota );
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
        // vorota.sendStatus( dukRouter, "on_off", ( gate.getStatus() == GateBase::StatusE::Closed) ? "off" : "on" ); 
        // vorota.sendStatus( dukRouter );

    }, 25 );

    dukRouter.connect();
    dukRouter.hello();
    dukRouter.sendStatus(vorota, "on_off", "off");
    delay(MS_NEXT_MQTT_SEND);
    dukRouter.sendStatus(vorota, "range", 0);
    //delay(1000);
    

    //devices[0].sendStatus( gate );
    ESP_MEMORY_PRINT(0);

    // delay(500);
    // dukRouter.sendStatus(vorota, "on_off", "on");
    // delay(1000);
    // dukRouter.sendStatus(vorota, "range", "50");
}

void loop()
{

    //relay.tick();
    //gateP.get()->tick();
    gate.tick();

    if (!dukRouter.is_connected()) {
        Serial.print(F("Reconnect "));
        dukRouter.connect();
        Serial.println( dukRouter.is_connected() ? F("success") : F("fail"));
        //gate.hello();
    }
    if ( dukRouter.tick() ){      
        Serial.println("Processed");
    }
    //delay(2000);
    delay(200);
    ESP_MEMORY_PRINT(3000);
}

