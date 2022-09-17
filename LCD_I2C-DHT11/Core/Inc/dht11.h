#ifndef __dht11_
#define __dht11_
#include "stm32f1xx_hal.h"
#include "tim.h"

void delay(uint16_t time);
void DHT11_Start (void);
uint8_t DHT11_Check_Response (void);
uint8_t DHT11_Read (void);

#endif