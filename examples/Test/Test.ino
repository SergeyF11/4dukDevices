#include <my4duk.h>
#include <GateControl.h>


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
#define FUNC_LINE
#define ESP_MEMORY_PRINT
#endif

extern Duk::DevicesT devices;

TimedControlGate gate(LED_BUILTIN, LOW , 10000UL, 3000UL );
extern Duk::DevicesT devices;

void cmdVorotaOpen(String& s){
    gate.startOpen();
 };

 void cmdRange(String& s){
    Serial.println(s);
 };

void printBarierState(){
    Serial.println( gate.getOpenedPercent() );
};


Duk::Action vorotaOpen( "on", cmdVorotaOpen );
//Duk::ActionsT vv = { vorotaOpen };
 
Duk::Action vorotaRange( 0, 100, cmdRange );

Duk::Actions vorotaOnOff( "on_off", Duk::ActionsT{ vorotaOpen } );
Duk::Actions vorotaRanger( "range",  vorotaRange );

void setup(){
    gate.setOnChanged(printBarierState, 50 );

    Serial.begin(115200);
    delay(500);
    Serial.println();
    Serial.println("Test");

    vorotaOpen.printTo(Serial);
    vorotaRange.printTo(Serial);

   // Duk::Actions vorotaRanger( "range",  vorotaRange );
    
    gate.startOpen();

    Serial.println("Passed");
}

 void loop(){
    gate.tick();
 }