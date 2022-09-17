#include "Flash.h"


void Flash_Erase_Page(uint32_t address)
{
    HAL_FLASH_Unlock();
    FLASH_EraseInitTypeDef EraseInitStruct; //initialize a struct 
    EraseInitStruct.Banks = 1;  //STM32F103 has 1bank obtain 64pages              
    EraseInitStruct.NbPages = 1;
    EraseInitStruct.PageAddress = address;
    EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES; 
    uint32_t page_error;
    HAL_FLASHEx_Erase(&EraseInitStruct, &page_error);
    HAL_FLASH_Lock();
}

void Flash_Write_Int(uint32_t address ,int value)
{
    HAL_FLASH_Unlock();
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, address, value);
    HAL_FLASH_Lock();
}

// void Flash_Write_Float(uint32_t address ,float number, int afterpoint)
// {
//     // HAL_FLASH_Unlock();
// 	// char string[10] = {0};
// 	// int intnumber = (int)number;
// 	// float diffnumber = number - (float)intnumber;
// 	// int anotherintnumber = (int)(diffnumber * pow(10, afterpoint));
// 	// sprintf(string, "%d.%d", intnumber, anotherintnumber);
//     // len_float = strlen(string);
//     // Flash_Write_String(address, (uint8_t*)string, strlen(string));
//     // HAL_FLASH_Lock();
// }

void Flash_Write_Float(uint32_t address ,float f)
{
    HAL_FLASH_Unlock();
    uint8_t data[4];
    *(uint32_t *)data = f;
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, *(uint32_t*)data);
    HAL_FLASH_Lock();
}

void Flash_Write_String(uint32_t address, uint8_t *str, uint16_t len)
{
    HAL_FLASH_Unlock();
    uint16_t *pt = (uint16_t*)str; //value of pointer is 2 byte respectively 2 implement 
    for(uint16_t i = 0; i<(len+1)/2;i++)
    {
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, address +2*i,*pt);
        pt++;
    }
    HAL_FLASH_Lock();
}

void Flash_Write_Struct(uint32_t address ,wifi_info data)
{
    Flash_Write_String(address, (uint8_t*)&data, sizeof(data));
}


int Flash_Read_Int(uint32_t address)
{
    return *(__IO uint16_t *)(address); // convert read 4 bytes to read 2 bytes
}

float Flash_Read_Float(uint32_t address)
{
    // char str[10] = {0};
    // double f;
    // Flash_Read_String(address, (uint8_t*)str, len_float);
    // f = atof((char*)str);
    // return (float)f;
    uint32_t data = *(__IO uint32_t*)address;
    return *(float*)&data;
}

void Flash_Read_String(uint32_t address, uint8_t *str, uint16_t len)
{
    //read string from flash and store in str
    uint16_t *pt = (uint16_t*)str; //value of pointer is 2 byte respectively 2 implement 
    for(uint16_t i = 0; i<(len+1)/2;i++)
    {
        *pt = *(__IO uint16_t *)(address+2*i);
        pt++;
    }
}

// void Flash_Read_Struct(uint32_t address ,wifi_info data)
// {
//     Flash_Read_String(address, (uint8_t*)&data, sizeof(data));
// }
void Flash_Read_Struct(uint32_t address ,uint8_t *data)
{
    Flash_Read_String(address, data, sizeof(wifi_info));
}