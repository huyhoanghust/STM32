#ifndef _INC_SIM800C_H__
#define _INC_SIM800C_H__

#include "main.h"
#include "stm32f1xx_hal.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

#define SIM_NOT_CHECK           0
#define SIM_CHECK               1

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
// extern TIM_HandleTypeDef htim2;
// extern TIM_HandleTypeDef htim3;

char simRxBuf[200];
int simRxIndex;
char simRxData;
char simRxResponse[200];
volatile simRxCplt_t simRxCplt;
char simNumber[20];

typedef enum
{
    SIM_NOT_OK,
    SIM_OK
} simState_t;

typedef enum
{
    SIM_NOT_RX_CPLT,
    SIM_RX_CPLT
} simRxCplt_t;


void sim800c_debug(uint8_t *pData, uint16_t Size, uint32_t Timeout);
void sim800c_init(void);
simState_t sim800c_sendCommand(char *simCommand, char *trueResponse, int checkOrNot);
void sim800c_callback(void);
void sim800c_timerCallback(void);
void sim800c_sendSMS(char *simMessage);
void sim800c_errorHandle(void);
void sim800c_trigPwrkey(void);
void sim800c_pwrkeyCallback(void);

#endif


