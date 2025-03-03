
#include <relay.h>
#include <my4duk.h>

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

Relay relay(LED_BUILTIN,0);
String state( Relay& r ){ 
    Serial.printf("Send state '%s'\n", r.getState(true) ? "on" : "off" );
    return String( r.getState(true) ? "on" : "off");
};
void relayOn(String& s){ relay.on(); };

Duk::Action vorotaOpen( "on", relayOn );
Duk::Action vorotaClose( "off", [](String& s){ Serial.println("Vorota close command"); } );

Duk::Actions vorotaActions( "on_off", Duk::ActionsT{ vorotaOpen, vorotaClose } );
callstat vorotaState( [](String& s){ return s+state(relay); } );
Duk::DeviceT vorota( Duk::DEV_THISGATE, 
    "thisgate", "thisgate", 
    "Home", "my", "my@mail.ru" , 
    0.0, 
    vorotaActions, vorotaState );

Duk::Gate gate( ID_4DUK, "thisGate", vorota );

void Relay::onChange() const {
    Serial.println("Status changed");
    vorota.sendStatus( gate );
};

void setup(){

    Serial.begin(115200);
    do { 
        delay(300);
    } while( !Serial);
    Serial.println();

    relay.on();
    
    if( ! myWiFiConnect() ){
        Serial.println("ERROR: wifi connection");
        return;
    } else {
        Serial.println("Connected WiFi");
    }
    relay.off();

    vorotaOpen.printTo(Serial);
    vorotaClose.printTo(Serial);

    relay.setAutoOff(4000);
    gate.connect();
    gate.hello();
    gate.sendStatus();
    //devices[0].sendStatus( gate );
    ESP_MEMORY_PRINT(0);

    Serial.println("======== device Vorota ==================");
    vorota.printTo(Serial);

}

void loop()
{
    relay.tick();
    if (!gate.is_connected()) {
        Serial.print(F("Reconnect "));
        gate.connect();
        Serial.println( gate.is_connected() ? F("success") : F("fail"));
        //gate.hello();
    }
    if ( gate.tick() ){      
        Serial.println("Processed");
    }
    //delay(2000);
    delay(200);
    ESP_MEMORY_PRINT(3000);
}

