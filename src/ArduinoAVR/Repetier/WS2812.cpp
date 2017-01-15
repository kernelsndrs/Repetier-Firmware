#include "WS2812.h"
#include "Repetier.h"

Adafruit_NeoPixel WS2812::ws_leds = Adafruit_NeoPixel(WS_NUM_LEDS, WS_LED_PIN, NEO_GRB + NEO_KHZ800);

uint8_t WS2812::currentMode = 1;
uint8_t WS2812::alertMode = 0;
uint8_t WS2812::pixelCount = 0;

uint32_t WS2812::previousMillis = 0;

uint16_t WS2812::neo_r = 0;
uint16_t WS2812::neo_g = 0;
uint16_t WS2812::neo_b = 255;
uint16_t WS2812::c_diff = 0;
uint32_t WS2812::ext_c;
uint32_t WS2812::last_ext_c;
float WS2812::curr_ext_temp = 0.0;
float WS2812::target_ext_temp = 0.0;

void WS2812::init() {
    ws_leds.begin();
    ws_leds.show();
    
    currentMode = 1;
}

void WS2812::workLoop() {
    switch (currentMode) {
        case 1:
            handleTemperature();
        break;
        case 2:
            handleAlert();
        break;
        case 3:
            handleColorWipe();
        break;
        case 4:
            handleTheaterChase();
        break;
    }
}

// gcode M707 - available commands:
// 0 - reset LED strip
// 1 - display temperature
// 2 - simple alert mode (flashing red and white)
// 3 - color wipe
// 4 - theater chase (flashing every second pixel)
void WS2812::handleCommand(uint8_t command) {
    previousMillis = 0;
    alertMode = 0;
    ext_c = 0;
    
    ws_leds.clear();
    
    switch (command) {
        case 0:
            init();
            Com::printFLN(PSTR("WS2812 - reseting leds"));
        break;
        case 1:
            currentMode = 1;
            Com::printFLN(PSTR("WS2812 - display temperature mode"));
        break;
        case 2:
            currentMode = 2;
            Com::printFLN(PSTR("WS2812 - alert mode"));
        break;
        case 3:
            currentMode = 3;
            pixelCount = 1;
            Com::printFLN(PSTR("WS2812 - color wipe"));
        break;
        case 4:
            currentMode = 4;
            Com::printFLN(PSTR("WS2812 - theater chase"));
        break;
    }
}

void WS2812::handleTemperature() {
    if(HAL::timeInMilliseconds() - previousMillis >= tempLoopInterval) {
        previousMillis = HAL::timeInMilliseconds();

        curr_ext_temp = Extruder::current->tempControl.currentTemperatureC;
        target_ext_temp = tempController[Extruder::current->id]->targetTemperatureC;

        //  SET Extruder LED   
        if(curr_ext_temp > 0) 
            if(curr_ext_temp > target_ext_temp && target_ext_temp > 0) 
                curr_ext_temp = target_ext_temp;

        if(curr_ext_temp > 30.0) {
            if(target_ext_temp > 0) {
                c_diff = (curr_ext_temp - 30)*255/(target_ext_temp-30);
                if(c_diff>255) {c_diff = 255;}
            } else {
                c_diff = (curr_ext_temp-30)*2;
                if(c_diff>255) {c_diff = 255;}
            }
            neo_r = c_diff;
            neo_b = 255 - c_diff;
        } else {
            neo_r = 0;
            neo_b = 255;
        }
        
        last_ext_c = ext_c;
        ext_c = ws_leds.Color(neo_r, neo_g, neo_b);

        if(ext_c != last_ext_c) 
            colorFill(ext_c);
    }
} 

void WS2812::handleAlert() {
    if(HAL::timeInMilliseconds() - previousMillis >= alertLoopInterval) {
        previousMillis = HAL::timeInMilliseconds();
        
        if (alertMode == 0)
            colorFill(ws_leds.Color(255, 255, 255)); // White
        else 
            colorFill(ws_leds.Color(255, 0, 0)); // Red
        
        alertMode += 1;
        
        if (alertMode > 1)
            alertMode = 0;
    }
}

void WS2812::handleColorWipe() {
    if(HAL::timeInMilliseconds() - previousMillis >= wipeLoopInterval) {
        previousMillis = HAL::timeInMilliseconds();

        if (alertMode == 0)
            colorWipe(ws_leds.Color(255, 0, 0)); // Red
        else if (alertMode == 1)
            colorWipe(ws_leds.Color(0, 255, 0)); // Green
        else 
            colorWipe(ws_leds.Color(0, 0, 255)); // Blue
        
        pixelCount += 1;
        
        if (pixelCount >= WS_NUM_LEDS) {
            pixelCount = 0;

            alertMode += 1;
            if (alertMode > 2)
                alertMode = 0;
        }
    }
}

void WS2812::handleTheaterChase() {
    if(HAL::timeInMilliseconds() - previousMillis >= theatherChaseLoopInterval) {
        previousMillis = HAL::timeInMilliseconds();
        
        uint32_t c1 = (alertMode == 0) ? ws_leds.Color(255, 0, 0) : 0;
        uint32_t c2 = (alertMode == 0) ? 0 : ws_leds.Color(255, 0, 0);
        
        for (uint16_t i=0; i < WS_NUM_LEDS; i+=2) {
            ws_leds.setPixelColor(i, c1);
            ws_leds.setPixelColor(i+1, c2);
        }
        ws_leds.show();

        alertMode += 1;
        
        if (alertMode > 1)
            alertMode = 0;
    }
}


void WS2812::colorFill(uint32_t c) {
    for(uint16_t i=0; i < WS_NUM_LEDS; i++) {
        ws_leds.setPixelColor(i, c);
    }
    
    ws_leds.show();
}

void WS2812::colorWipe(uint32_t c) {
    ws_leds.setPixelColor(pixelCount, c);
    ws_leds.show();
}
