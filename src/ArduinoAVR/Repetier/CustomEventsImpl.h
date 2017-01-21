/*
  WS2812b LED Strip Control.
  
  Heavily based off MTWLED contributions noted below. modified by 1ixlabs Jan 20, 2017.
  Repetier-Firmware port/rewrite by exuvo 2015-05-17
  Contributed to MTW by OhmEye October 2014
  */
#include "WS2812.h"
#define outputPin 11  // Digital output pin 
#define LEDCount 120   // Number of LEDs to drive 

#define WSLED_nochange  1 // Reserved for no change to LED Strip

#ifndef UINT8_MAX
#define UINT8_MAX 255
#endif // UINT8_MAX

// patterns               pattern R   G   B
#define WSLED_ready        0, 30,  0    // Printer Ready
#define WSLED_startup     2, 30, 30, 30    // Printer startup
#define WSLED_temphit     40, 40, 40    // Hotend is at target temp
#define WSLED_templow     40,  0, 40    // Hotend heater is slightly lower than target temp
#define WSLED_temphigh    40,  0,  0    // Hotend heater is slightly higher than target temp
#define WSLED_heateroff    0,  0, 40    // Hotend heater is off but still hot
#define WSLED_heating0     0,  0, 50    // Hotend heating up <10%
#define WSLED_heating1     0,  0,100    // Hotend heating up <20%
#define WSLED_heating2     0, 50,100    // Hotend heating up <30%
#define WSLED_heating3     0,100,100    // Hotend heating up <40%
#define WSLED_heating4     0,100, 50    // Hotend heating up <50%
#define WSLED_heating5   100,100,  0    // Hotend heating up <60%
#define WSLED_heating6   100, 50,  0    // Hotend heating up <70%
#define WSLED_heating7   100,  0,100    // Hotend heating up <80%
#define WSLED_heating8   100,  0, 50    // Hotend heating up <90%
#define WSLED_heating9   100,  0,  0    // Hotend heating up <100%

WS2812 LED(LEDCount); 
cRGB value;
uint16_t WSLED_lastpattern;

void WSLED_Init(){
  LED.setOutput(outputPin); // Digital Pin 

  /* You may uncomment one of the following three lines to switch 
  to a different data transmission sequence for your addressable LEDs.
  (These functions can be used at any point in your program as needed.)   */

  LED.setColorOrderRGB();  // Uncomment for RGB color order
  //LED.setColorOrderBRG();  // Uncomment for BRG color order
  //LED.setColorOrderGRB();  // Uncomment for GRB color order (Default; will be used if none other is defined.)

  WSLED_Write(WSLED_startup);
  return;
}
void WSLED_WaitingHeater(int8_t id){
  if(id == -1){
    waitingForHeaterIndex = NUM_TEMPERATURE_LOOPS - 1;
  } else if (id >= 0 && id < NUM_TEMPERATURE_LOOPS) {
    waitingForHeaterIndex = id;
  } else {
   //Error
   waitingForHeaterIndex = UINT8_MAX;
  }

  if(waitingForHeaterIndex < NUM_TEMPERATURE_LOOPS){
    waitingForHeaterStartC = tempController[waitingForHeaterIndex]->currentTemperatureC;
  }
}

void WSLED_HeatingFinished(int8_t id){
  waitingForHeaterIndex = UINT8_MAX;
}

//does percentile display between start temp and target temp while a heater is heating up
bool WSLEDTemp() {
  if(waitingForHeaterIndex >= NUM_TEMPERATURE_LOOPS){
    return false;
  }

  float target = tempController[waitingForHeaterIndex]->targetTemperatureC;
  float current = tempController[waitingForHeaterIndex]->currentTemperatureC;
  float start = waitingForHeaterStartC;


  uint8_t pattern = (10.0f * (1.0f - ((target - current) / (target - start))));

  switch(pattern){
    default:
    case 0:
      WSLED_Write(WSLED_heating0);
      break;
    case 1:
      WSLED_Write(WSLED_heating1);
      break;
    case 2:
      WSLED_Write(WSLED_heating2);
      break;
    case 3:
      WSLED_Write(WSLED_heating3);
      break;
    case 4:
      WSLED_Write(WSLED_heating4);
      break;
    case 5:
      WSLED_Write(WSLED_heating5);
      break;
    case 6:
      WSLED_Write(WSLED_heating6);
      break;
    case 7:
      WSLED_Write(WSLED_heating7);
      break;
    case 8:
      WSLED_Write(WSLED_heating8);
      break;
    case 9:
      WSLED_Write(WSLED_heating9);
      break;
  }

  return true;
}
void WSLED_Write(uint8_t pattern = 1, uint8_t red, uint8_t green, uint8_t blue) {
  value.r = red;
  value.g = green;
  value.b = blue;
  //ghetto cache.
  if((pattern*red+green-blue) != WSLED_lastpattern) {
    switch(pattern) {
      case 1:
        WSLED_Full();
        break;
      case 2:
        WSLED_Alternate();
        break;
    }
    WSLED_lastpattern = (pattern*red+green-blue);
  }
}
void WSLED_Alternate() {
  while (i < LEDCount) {
    if(i % 2 )
      LED.set_crgb_at(i,value);
    i++;
  }
}
void WSLED_Full(){
  while (i < LEDCount) {
    LED.set_crgb_at(i,value);
    i++;
  }
}
void WSLED_Loop() {
  
  if(WSLEDTemp()) {
    return;
  }


  float currentTempC =  0;
  float targetTempC = 0;

  for(uint8_t i=0; i < NUM_EXTRUDER; i++){
    float target = tempController[i]->targetTemperatureC;
    float current = tempController[i]->currentTemperatureC;

    if(target > targetTempC || (targetTempC == 0 && target == 0 && current > currentTempC)){
      currentTempC = current;
      targetTempC = target;
    }
  }

  if(targetTempC == 0) {
    if(currentTempC > WSLED_cool) { // heater is off but still warm
      WSLED_Write(WSLED_heateroff);
    } else {
      WSLED_Write(2, WSLED_ready);
    }

  } else {
    uint8_t swing = abs(targetTempC - currentTempC); // how far off from target temp we are

    if(swing < WSLED_swing * 2) {                  // if within double the swing threshold
      if(swing < WSLED_swing) {
        WSLED_Write(WSLED_temphit);  // close to target temp, so consider us 'at temp'
      } else {
        if(currentTempC >= targetTempC) {
          WSLED_Write(WSLED_temphigh);   // temp high, heater is off
        } else {
          WSLED_Write(WSLED_templow);    // temp low, heater is on
        }
      }
    }
  }
}
/*



/*
  Pattern ID is the number of the pattern/animation to use
  Current patterns are:
      10 RGB	Solid color
      11 RGB 	Cylon
      12 RGB 	UFO PULSE
      13 XXX 	Color Chase
      14 XXX 	Rainbow Cycle
      15 RGB 	Color Chase Single Led
      16 RGB 	Slow fill then solid
      17 RGB	Repeating Blink
      18 XXX  Rainbow
      19 XXX  Theather Chase Rainbow
      90-99   Reserved for heating and cooling values
   R is a value from 0-127 for how red the color will be
   G is a value from 0-127 for how green the color will be
   B is a value from 0-127 for how blue the color will be
      Specifying colors is often optional, any color not given will be either 0 (none) or a default
      depending on the pattern selected.
   T is a timer in seconds for how long the pattern will override the default patterns


// Pattern Selection Table for defaults that must not be changed


union patterncode {  // access a pattern both as 32 bits and as array of 4 uint8_ts.
  uint32_t value;
  uint8_t part[4];
};

patterncode WSLED_lastpattern;
uint16_t WSLED_timer;
bool WSLED_starup;
uint8_t waitingForHeaterIndex = UINT8_MAX;
float waitingForHeaterStartC;
}

bool WSLEDEndstop(bool force) {
  uint8_t endx=0, endy=0, endz=0;

  Endstops::update();

  #if (X_MIN_PIN > -1) && MIN_HARDWARE_ENDSTOP_X
    if(Endstops::xMin()) endx |= 1;
  #endif
  #if (Y_MIN_PIN > -1) && MIN_HARDWARE_ENDSTOP_Y
    if(Endstops::yMin()) endy |= 1;
  #endif
  #if (Z_MIN_PIN > -1) && MIN_HARDWARE_ENDSTOP_Z
    if(Endstops::zMin()) endz |= 1;
  #endif
  #if (X_MAX_PIN > -1) && MAX_HARDWARE_ENDSTOP_X
    if(Endstops::xMax()) endx |= 1;
  #endif
  #if (Y_MAX_PIN > -1) && MAX_HARDWARE_ENDSTOP_Y
    if(Endstops::yMax()) endy |= 1;
  #endif
  #if (Z_MAX_PIN > -1) && MAX_HARDWARE_ENDSTOP_Z
    if(Endstops::zMax()) endz |= 1;
  #endif

  if(force || endx || endy || endz) {
    WSLED_Write(2, endx, endy, endz, WSLED_endstoptimer);

    if(endx || endy || endz) {
        return true;
    }
  }

  return false;
}


//Called every 100ms
void WSLED_Update() {
  if(!WSLED_starup) { // if this is first time display endstop status before clearing to ready
    WSLEDSetup();
    WSLEDEndstop(true);
    WSLED_starup = true;
    return;
  }

  if(WSLEDTemp() || WSLEDEndstop(false) || WSLED_lastpattern.part[0] == WSLED_nochange) {
    return;
  }

  if(WSLED_timer > 0){
    WSLED_timer--;
    return;
  }

  float currentTempC =  0;
  float targetTempC = 0;

  for(uint8_t i=0; i < NUM_EXTRUDER; i++){
    float target = tempController[i]->targetTemperatureC;
    float current = tempController[i]->currentTemperatureC;

    if(target > targetTempC || (targetTempC == 0 && target == 0 && current > currentTempC)){
      currentTempC = current;
      targetTempC = target;
    }
  }

  if(targetTempC == 0) {
    if(currentTempC > WSLED_cool) { // heater is off but still warm
      WSLED_Write(WSLED_heateroff, 0);
    } else {
      WSLED_Write(WSLED_ready, 0);
    }

  } else {
    uint8_t swing = abs(targetTempC - currentTempC); // how far off from target temp we are

    if(swing < WSLED_swing * 2) {                  // if within double the swing threshold
      if(swing < WSLED_swing) {
        WSLED_Write(WSLED_temphit, 0);  // close to target temp, so consider us 'at temp'
      } else {
        if(currentTempC >= targetTempC) {
          WSLED_Write(WSLED_temphigh, 0);   // temp high, heater is off
        } else {
          WSLED_Write(WSLED_templow, 0);    // temp low, heater is on
        }
      }
    }
  }
}
*/