#ifndef CUSTOM_EVENTS_H_INCLUDED
#define CUSTOM_EVENTS_H_INCLUDED

//#define EVENT_WAITING_HEATER(id) {WSLED_WaitingHeater(id);}
//#define EVENT_HEATING_FINISHED(id) {WSLED_HeatingFinished(id);}
#define EVENT_TIMER_100MS {WSLED_Loop();}
#define EVENT_INITIALIZE {WSLED_Init();}
void WSLED_Init();
void WSLED_Loop();
void WSLED_WaitingHeater(int8_t id);
void WSLED_HeatingFinished(int8_t id);

#endif