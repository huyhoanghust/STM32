/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *    // angle = 1.8*count-45;
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "stdlib.h"
#include "math.h"
#include "string.h"
#include "st7735.h"
#include "dht22.h"
// #include "fonts.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
int count = 25;
char rota[20] = {0};
char threshold[20] = {0};
char label_thres[10] = {0};
char str_thres[5] = {0};
int angle;
char buf_temp[20] = {0};
char buf_humi[20] = {0};
enum
{
  Task_sensor,
  Task_control,
  Task_display,
  Mode_auto,
  Mode_manual
};

volatile int Type_task = Task_sensor;
volatile int Mode = Mode_auto;

uint8_t Rx_data;
char buffer_data[50] = {0};
volatile int Rx_index = 0;
volatile bool Rx_complete = false;

volatile bool perform_s = false;
volatile bool perform_c = false;
volatile bool perform_d = false;

int PC_rota;
float PC_threshold;

int temp = 30;
int humi = 90;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
// void debug()
void set_timer(long time)
{
  TIM3->CNT = 60000 - time * 10;
}
void log_data(char *buffer)
{
  HAL_UART_Transmit(&huart3, (uint8_t *)buffer, strlen(buffer), 1000);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim == &htim3)
  {
    if (Type_task == Task_sensor)
    {
      perform_s = false;
      Type_task = Task_control;
      set_timer(500);
      return;
    }
    if (Type_task == Task_control)
    {
      perform_c = false;
      Type_task = Task_display;
      set_timer(500);
      return;
    }
    if (Type_task == Task_display)
    {
      perform_d = false;
      Type_task = Task_sensor;
      set_timer(1000);
      return;
    }
  }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart == &huart3)
  {
    HAL_TIM_Base_Stop(&htim3);
    if (Rx_data == 13)
    {
      buffer_data[Rx_index] = '\0';
      Rx_index = 0;
      Rx_complete = true;
      if (strcmp(buffer_data, "manual") == 0)
      {
        Mode = Mode_manual;
      }
      if (strcmp(buffer_data, "auto") == 0)
      {
        Mode = Mode_auto;
      }

      // HAL_TIM_Base_Stop_IT(&htim3);
      // log_data(buffer_data);
    }
    else
    {
      buffer_data[Rx_index] = (char)Rx_data;
      Rx_index++;
      // log_data((char*)Rx_data);
    }
    HAL_UART_Receive_IT(&huart3, &Rx_data, 1);
    HAL_TIM_Base_Start(&htim3);
  }
}
uint8_t RHI, RHD, TCI, TCD, SUM;
float tCelsius = 0;
float tFahrenheit = 0;
float RH = 0;
/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI2_Init();
  MX_TIM1_Init();
  MX_USART3_UART_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
  // TImer 2 ->>>> generate PWM to control sg90
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
  // init LCD TFT
  ST7735_Init();
  // Timer 1 to gen delay for DHT22
  HAL_TIM_Base_Start(&htim1);
  // set screen
  ST7735_FillScreen(ST7735_BLACK);
  ST7735_WriteString(25, 0, "Team 17", Font_11x18, ST7735_RED, ST7735_BLACK);
  ST7735_WriteString(0, 30, "Temp: ", Font_11x18, ST7735_RED, ST7735_BLACK);
  ST7735_WriteString(0, 3 * 10 * 2, "Humi: ", Font_11x18, ST7735_RED, ST7735_BLACK);
  ST7735_WriteString(0, 3 * 10 * 3, "Rota: ", Font_11x18, ST7735_GREEN, ST7735_BLACK);
  ST7735_WriteString(0, 3 * 10 * 4, "Thresh: ", Font_11x18, ST7735_GREEN, ST7735_BLACK);
  ST7735_WriteString(35, 145, "Warnning", Font_7x10, ST7735_WHITE, ST7735_BLACK);
  HAL_Delay(1000);

  set_timer(1000);
  HAL_TIM_Base_Start_IT(&htim3);

  // interrupt UART
  HAL_UART_Receive_IT(&huart3, &Rx_data, 1);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    switch (Mode)
    {
    case Mode_auto:
      if (Type_task == Task_sensor)
      {
        if (!perform_s)
        {
          perform_s = true;
          log_data("task sensor\n");
          if (DHT11_Start())
          {
            RHI = DHT11_Read(); // Relative humidity integral
            RHD = DHT11_Read(); // Relative humidity decimal
            TCI = DHT11_Read(); // Celsius integral
            TCD = DHT11_Read(); // Celsius decimal
            SUM = DHT11_Read(); // Check sum
            if (RHI + RHD + TCI + TCD == SUM)
            {
              // Can use RHI and TCI for any purposes if whole number only needed
              temp = (float)TCI + (float)(TCD / 10.0);
              humi = (float)RHI + (float)(RHD / 10.0);
              // Can use tCelsius, tFahrenheit and RH for any purposes
            }
          }
          // read sensor
        }
        if (Type_task == Task_control)
        {
          if (!perform_c)
          {
            perform_c = true;
            log_data("task control\n");
            // depend Temp value
            //>30C -> close curtain
            if (temp > 30)
            {
              TIM2->CCR1 = 25;
              sprintf(rota, "Rota: %d  ", 0);
              // log_data(rota);
              ST7735_WriteString(0, 3 * 10 * 3, rota, Font_11x18, ST7735_GREEN, ST7735_BLACK);
            }
            //>30C -> close curtain
            else
            {
              TIM2->CCR1 = 125;
              sprintf(rota, "Rota: %d  ", 180);
              // log_data(rota);
              ST7735_WriteString(0, 3 * 10 * 3, rota, Font_11x18, ST7735_GREEN, ST7735_BLACK);
            }
          }
        }
        if (Type_task == Task_display)
        {
          if (!perform_d)
          {
            perform_d = true;
            log_data("task display\n");
            sprintf(buf_temp, "Temp: %d C", temp);
            sprintf(buf_humi, "Humi: %d ", humi);
            ST7735_WriteString(0, 30, buf_temp, Font_11x18, ST7735_RED, ST7735_BLACK);
            ST7735_WriteString(0, 3 * 10 * 2, strcat(buf_humi, "%"), Font_11x18, ST7735_RED, ST7735_BLACK);
          }
        }
        break;

      case Mode_manual:
        if (Rx_complete)
        {
          Rx_complete = false;
          if (strncmp(buffer_data, "Rota", 4) == 0)
          {
            // Parse string
            sscanf(buffer_data, "%*[^0-9]%d", &PC_rota);
            // control Servo
            TIM2->CCR1 = (int)((PC_rota + 45) / 1.8);
            sprintf(rota, "Rota: %d  ", PC_rota);
            log_data(rota);
            ST7735_WriteString(0, 3 * 10 * 3, rota, Font_11x18, ST7735_GREEN, ST7735_BLACK);
          }
          if (strncmp(buffer_data, "Threshold", strlen("Threshold")) == 0)
          {
            char str_buf[] = "Thres: ";
            // Parse string
            sscanf(buffer_data, "%s%s", label_thres, str_thres);
            log_data(str_thres);
            log_data(label_thres);
            // control Servo
            PC_threshold = atof(str_thres);
            strcat(str_buf, str_thres);
            log_data(str_buf);
            // sprintf(threshold, "%.1f  ", PC_threshold);
            // // strcat()
            // // strcpy(threshold,str_thres);
            // log_data(threshold);
            ST7735_WriteString(0, 3 * 10 * 4, str_buf, Font_11x18, ST7735_GREEN, ST7735_BLACK);
          }
          memset(buffer_data, '\0', sizeof(buffer_data));
          // HAL_TIM_Base_Start_IT(&htim3);
        }
      }
      HAL_Delay(100);
      // HAL_TIM_Base_Stop(&htim3);
      // HAL_Delay(3000);
      // HAL_TIM_Base_Start_IT(&htim3);
      // HAL_Delay(3000);
      // TIM2 -> CCR1 = count;
      // angle = 1.8*count-45;
      // sprintf(rota,"Rota: %d  ",angle);
      // debug(rota);
      // ST7735_WriteString(0, 3 * 10 * 2, rota, Font_11x18, ST7735_GREEN, ST7735_BLACK);
      // count =count+10;
      // if(count>125)
      // {
      //   count=25;
      // }
      // HAL_Delay(2000);
      // Check fonts
    }
  }
  /* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
