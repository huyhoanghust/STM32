// #ifndef __dht11_
// #define __dht11_
// #include "stm32f1xx_hal.h"
// #include "tim.h"

// void delay(uint16_t time);
// void DHT11_Start (void);
// uint8_t DHT11_Check_Response (void);
// uint8_t DHT11_Read (void);

// #endif
#ifndef __DHT_11_H_
#define __DHT_11_H_

#include "stm32f1xx_hal.h"
#include <stdio.h>
#include "tim.h"
#define DHT11_PORT GPIOB
#define DHT11_PIN GPIO_PIN_11

uint8_t DHT11_Start (void);
uint8_t DHT11_Read (void);

#endif


    // if(DHT11_Start())
    // {
    //   RHI = DHT11_Read(); // Relative humidity integral
    //   RHD = DHT11_Read(); // Relative humidity decimal
    //   TCI = DHT11_Read(); // Celsius integral
    //   TCD = DHT11_Read(); // Celsius decimal
    //   SUM = DHT11_Read(); // Check sum
    //   if (RHI + RHD + TCI + TCD == SUM)
    //   {
    //     // Can use RHI and TCI for any purposes if whole number only needed
    //     tCelsius = (float)TCI + (float)(TCD/10.0);
    //     tFahrenheit = tCelsius * 9/5 + 32;
    //     RH = (float)RHI + (float)(RHD/10.0);
    //     // Can use tCelsius, tFahrenheit and RH for any purposes
    //   }