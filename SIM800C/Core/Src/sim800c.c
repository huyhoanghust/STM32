#include "sim800c.h"

char simRxBuf[200] = {0};
int simRxIndex = 0;
char simRxData = '\0';
char simRxResponse[200] = {0};
simRxCplt_t simRxCplt = SIM_NOT_RX_CPLT;
char simNumber[20] = "";

// void sim800c_trigPwrkey(void)
// {
//     htim3.Init.Period = 15000;
//     HAL_TIM_Base_Init(&htim3);
//     HAL_GPIO_WritePin(GSM_PWRKEY_GPIO_Port, GSM_PWRKEY_Pin, ENABLE);
//     HAL_TIM_Base_Start_IT(&htim3);
// }

// void sim800c_pwrkeyCallback(void)
// {
//     HAL_TIM_Base_Stop_IT(&htim3);
//     htim3.Instance->CNT = 0;
//     HAL_GPIO_WritePin(GSM_PWRKEY_GPIO_Port, GSM_PWRKEY_Pin, DISABLE);
// }
// void sim800c_trigPwrkey()
// {
//     HAL_GPIO_WritePin(GSM_PWRKEY_GPIO_Port, GSM_PWRKEY_Pin, 1);
//     HAL_Delay(1000);
//     HAL_GPIO_WritePin(GSM_PWRKEY_GPIO_Port, GSM_PWRKEY_Pin, 0;
// }

// void sim800c_checkStatus(void)
// {
//     htim3.Init.Period = 30000;
//     HAL_TIM_Base_Init(&htim3);
//     HAL_TIM_Base_Start_IT(&htim3);
// }

void sim800c_;
//debug ttl uart1
void sim800c_debug(uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
    HAL_UART_Transmit(&huart1, pData, Size , Timeout);
}

void sim800c_init()
{
    //disable echo
    if(sim800c_sendCommand("ATE0\r\n", "OK", SIM_NOT_CHECK) != SIM_OK)
    {
        sim800c_errorHandle();
    }
    //check active
    HAL_Delay(200);
    if(sim800c_sendCommand("AT\r\n", "OK", SIM_CHECK) != SIM_OK)
    {
        sim800c_errorHandle();
    }
    //choose text mode
    if(sim800c_sendCommand("AT+CMGF=1\r\n", "OK", SIM_CHECK) != SIM_OK)
    {
        sim800c_errorHandle();
    }
    //
    if(sim800c_sendCommand("AT&W\r\n", "OK\r\n", SIM_CHECK) != SIM_OK)
    {
        sim800c_errorHandle();
    }
    //set password of phone
    if(sim800c_sendCommand("AT+CPIN?\r\n", '\0', SIM_NOT_CHECK) != SIM_OK)
    {
        sim800c_errorHandle();
    }
    if(sim800c_sendCommand("AT+CSQ\r\n", '\0', SIM_NOT_CHECK) != SIM_OK)
    {
        sim800c_errorHandle();
    }
    if(sim800c_sendCommand("AT+COPS?\r\n", '\0', SIM_NOT_CHECK) != SIM_OK)
    {
        sim800c_errorHandle();
    }    
}

simState_t sim800c_sendCommand(char *simCommand, char *trueResponse, int checkOrNot)
{   
    //declare receive interrupt function when sim send data to MCU 
    HAL_UART_Receive_IT(&huart2, (uint8_t*)&simRxData, 1);  
    sim800c_debug((uint8_t*)simCommand, strlen(simCommand), 100);
    //send command from MCU to SiM
    HAL_UART_Transmit_IT(&huart2, (uint8_t*)simCommand, strlen(simCommand));

//    HAL_Delay(2000);
    HAL_TIM_Base_Start_IT(&htim2);
    //loop until SIM receive complete data from MCU
    while(!(simRxCplt == SIM_RX_CPLT));
    //send response of SIM to USB
    sim800c_debug((uint8_t*)simRxResponse, strlen(simRxResponse), 100);
    //check response of sim 
    //NO
    if((checkOrNot == SIM_CHECK) && (strstr(simRxResponse, trueResponse) == NULL))
    {
        memset(simRxResponse, '\0', strlen(simRxResponse));
        simRxCplt = SIM_NOT_RX_CPLT;
        return SIM_NOT_OK;
    }
    //Yes
    memset(simRxResponse, '\0', strlen(simRxResponse));
    simRxCplt = SIM_NOT_RX_CPLT;
    return SIM_OK;
}
//check response from Sim and delete Buffer
void sim800c_callback()
{
    htim2.Instance->CNT = 0;
    simRxBuf[simRxIndex++] = simRxData;
    if((strstr(simRxBuf, "\r\n") != NULL) && (simRxIndex == 2))
    {
        memset(simRxBuf, '\0', strlen(simRxBuf));
        simRxIndex = 0;
    }
    HAL_UART_Receive_IT(&huart2, (uint8_t*)&simRxData, 1);
}
//wait 
void sim800c_timerCallback()
{
    HAL_TIM_Base_Stop_IT(&htim2);
    htim2.Instance->CNT = 0;
    if((strstr(simRxBuf, "\r\n") != NULL) | (strstr(simRxBuf, ">") != NULL))
    {
        memcpy(simRxResponse, simRxBuf, strlen(simRxBuf));
        memset(simRxBuf, '\0', strlen(simRxBuf));
        simRxIndex = 0;
    }
    simRxCplt = SIM_RX_CPLT;
}

void sim800c_sendSMS(char *simMessage)
{
    char smsATCommand[50] = {0};
    // char simMessage[200] = {0};
    char endSMS = 0x1A;
    sprintf(smsATCommand, "AT+CMGS=\"%s\"\r\n", simNumber);
    if(sim800c_sendCommand(smsATCommand, ">", SIM_CHECK) != SIM_OK)
    {
        sim800c_errorHandle();
    }
    if(sim800c_sendCommand(*simMessage, NULL, SIM_NOT_CHECK) != SIM_OK)
    {
        sim800c_errorHandle();
    }
    HAL_Delay(200);
    if(sim800c_sendCommand(&endSMS, NULL, SIM_NOT_CHECK) != SIM_OK)
    {
        sim800c_errorHandle();
    }
    
}

void sim800c_errorHandle()
{
    sim800c_debug((uint8_t*)"Sim error\r\n", strlen("Sim error\r\n"), 100);
    // __disable_irq();
    while(1)
    {

    }
}

