#ifndef KT_SPI_H_
#define KT_SPI_H_

#include "stm32f1xx_hal.h"
extern SPI_HandleTypeDef hspi2;
// GPIO_InitTypeDef gpioInit;
// #define MFRC522_CS_LOW					HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET)
// #define MFRC522_CS_HIGH					HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET)

// void TM_SPI_Init(void);
// extern void TM_MFRC522_InitPins(void);
uint8_t TM_SPI_Send(uint8_t data);


#endif