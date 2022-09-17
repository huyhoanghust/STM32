#ifndef __FLASH_H
#define __FLASH_H

#include "stm32f1xx.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "math.h"
#pragma pack(1)
typedef struct wifi_info
{
    //uint8_t no;
    uint8_t ssid[30];
    uint8_t pass[30];
} wifi_info;
#pragma pack()

void Flash_Erase_Page(uint32_t address);
void Flash_Write_Int(uint32_t address ,int value);
//void Flash_Write_Float(uint32_t address ,float number, int afterpoint);
void Flash_Write_Float(uint32_t address ,float f);
void Flash_Write_String(uint32_t address, uint8_t *str, uint16_t len);
void Flash_Write_Struct(uint32_t address ,wifi_info data);

int Flash_Read_Int(uint32_t address);
float Flash_Read_Float(uint32_t address);
void Flash_Read_String(uint32_t address, uint8_t *str, uint16_t len);
// void Flash_Read_Struct(uint32_t address ,wifi_info data);
void Flash_Read_Struct(uint32_t address ,uint8_t *data);
#endif