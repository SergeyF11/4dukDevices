#pragma once 
#define WITTY_CLOUD_RED_LED_PIN 15
#define WITTY_CLOUD_GREEN_LED_PIN 12
#define WITTY_CLOUD_BLUE_LED_PIN 13

class RGBled {
    private:
    const uint8_t pinR;
    const uint8_t pinG;
    const uint8_t pinB;
    const bool stateOff;
    //bool getColor(Color c);
    public:
    enum Color {
        Black,
        Red,
        Green,
        Yellow,
        Blue,
        Magenta,
        Cyan,
        White,
    };

    RGBled( const uint8_t r=WITTY_CLOUD_RED_LED_PIN, 
        const uint8_t g=WITTY_CLOUD_GREEN_LED_PIN, 
        const uint8_t b=WITTY_CLOUD_BLUE_LED_PIN, 
        const bool stateOn = HIGH):
        pinR(r), pinG(g), pinB(b), stateOff(!stateOn)
        {
            pinMode(pinR, OUTPUT);
            pinMode(pinG, OUTPUT);
            pinMode(pinB, OUTPUT);
            //color(Black);
            set(0,0,0);
        };
    void color(const Color c ){
        digitalWrite( pinR, (c & Red) == 0 ? stateOff : !stateOff );
        digitalWrite( pinG, (c & Green) == 0  ? stateOff : !stateOff );
        digitalWrite( pinB, (c & Blue) == 0  ? stateOff : !stateOff );
    };
    void set(const bool r, const bool g, const bool b){
        digitalWrite( pinR, r ? !stateOff : stateOff);
        digitalWrite( pinG, g ? !stateOff : stateOff);
        digitalWrite( pinB, b ? !stateOff : stateOff);
    };

    void test(uint pause){
        color(RGBled::Black);
        Serial.println("Black");
        delay(pause);
        color(RGBled::Red);
        Serial.println("Red");
        delay(pause);
        color(RGBled::Green);
        Serial.println("Green");
        delay(pause);
        color(RGBled::Blue);
        Serial.println("Blue");
        delay(pause);
        color(RGBled::White);
        Serial.println("White");
        delay(pause);
        color(RGBled::Magenta);
        Serial.println("Magenta");
        delay(pause);
        color(RGBled::Yellow);
        Serial.println("Yellow");
        delay(pause);
        color(RGBled::Black);
    };
};