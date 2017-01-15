#ifndef WS2812_H_INCLUDED
#define WS2812_H_INCLUDED

#include "Adafruit_NeoPixel.h"

#define NEOPIXEL_LEDS
#define WS_NUM_LEDS 120
#define WS_LED_PIN 2

class WS2812
{
public:
    static Adafruit_NeoPixel ws_leds;

    static void init();
    static void workLoop();
    static void handleCommand(uint8_t command);
    static void handleTemperature();
    static void handleAlert();
    static void handleColorWipe();
    static void handleTheaterChase();

private:
    static const uint16_t tempLoopInterval = 500;
    static const uint16_t alertLoopInterval = 500;
    static const uint16_t wipeLoopInterval = 50;
    static const uint16_t theatherChaseLoopInterval = 100;
    static uint32_t previousMillis;
    
    static uint8_t currentMode;
    static uint8_t alertMode;
    static uint8_t pixelCount;
    static uint32_t stat_update;
    static uint16_t neo_r;
    static uint16_t neo_g;
    static uint16_t neo_b;
    static uint16_t c_diff;
    static uint32_t ext_c;
    static uint32_t last_ext_c;
    static float curr_ext_temp;
    static float target_ext_temp;

    static void colorFill(uint32_t c);
    static void colorWipe(uint32_t c);
};
#endif // WS2812_H_INCLUDED
