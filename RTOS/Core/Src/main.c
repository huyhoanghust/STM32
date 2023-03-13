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
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "math.h"
#include "st7735.h"
#include "dht22.h"
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
 SPI_HandleTypeDef hspi2;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart3;

osThreadId Task1Handle;
osThreadId Task2Handle;
osThreadId Task3Handle;
osThreadId Task4Handle;
osSemaphoreId SensorSemHandle;
osSemaphoreId ServoSemHandle;
osSemaphoreId DataSemHandle;
osSemaphoreId IRQSemHandle;
/* USER CODE BEGIN PV */
enum
{
  Mode_auto,
  Mode_manual
};
volatile int Mode = Mode_auto;

uint8_t Rx_data;
char buffer_data[50] = {0};
volatile int Rx_index = 0;

// example
int temp;
int humi;

char buf_sensor[20] = {0};
char buf_temp[20] = {0};
char buf_humi[20] = {0};

int count = 25;
char rota[20] = {0};
char threshold[20] = {0};
char label_thres[10] = {0};
char str_thres[5] = {0};
int angle;

int PC_rota;
float PC_threshold=30;

char thres_tring[] = "Thres: 30";
uint8_t RHI, RHD, TCI, TCD, SUM;
float tCelsius = 0;
float tFahrenheit = 0;
float RH = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_SPI2_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
void Task_Sensor(void const * argument);
void Task_Servo(void const * argument);
void Task_Display(void const * argument);
void Task_Data(void const * argument);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void log_data(char *buf)
{
  HAL_UART_Transmit(&huart3, (uint8_t *)buf, strlen(buf), 100);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart == &huart3)
  {
    if (Rx_data == 13)
    {
      buffer_data[Rx_index] = '\0';
      Rx_index = 0;
      if (strcmp(buffer_data, "manual") == 0)
      {
        Mode = Mode_manual;
      }
      if (strcmp(buffer_data, "auto") == 0)
      {
        Mode = Mode_auto;
      }
      if (strncmp(buffer_data, "Rota", 4) == 0)
      {
        // Parse string
        sscanf(buffer_data, "%*[^0-9]%d", &PC_rota);
      }
      if (strncmp(buffer_data, "Threshold", strlen("Threshold")) == 0)
      {
        log_data("check data thresh\n");
        sscanf(buffer_data, "%s%s", label_thres, str_thres);
      }
      osSemaphoreRelease(IRQSemHandle);
    }
    else
    {
      buffer_data[Rx_index] = (char)Rx_data;
      Rx_index++;
      // log_data((char*)Rx_data);
    }
    HAL_UART_Receive_IT(&huart3, &Rx_data, 1);
  }
}
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
  MX_USART3_UART_Init();
  MX_SPI2_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
  HAL_UART_Receive_IT(&huart3, &Rx_data, 1);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
  ST7735_Init();
  HAL_TIM_Base_Start(&htim2);
  ST7735_FillScreen(ST7735_BLACK);
  ST7735_WriteString(25, 0, "Team 17", Font_11x18, ST7735_RED, ST7735_BLACK);
  ST7735_WriteString(0, 30, "Temp: ", Font_11x18, ST7735_RED, ST7735_BLACK);
  ST7735_WriteString(0, 3 * 10 * 2, "Humi: ", Font_11x18, ST7735_RED, ST7735_BLACK);
  ST7735_WriteString(0, 3 * 10 * 3, "Rota: ", Font_11x18, ST7735_GREEN, ST7735_BLACK);
  // sprintf(thres_tring,"Thresh: %.1f",PC_threshold);
  // strcat(thres_tring,"30");
  ST7735_WriteString(0, 3 * 10 * 4, thres_tring, Font_11x18, ST7735_GREEN, ST7735_BLACK);
  /* USER CODE END 2 */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* definition and creation of SensorSem */
  osSemaphoreDef(SensorSem);
  SensorSemHandle = osSemaphoreCreate(osSemaphore(SensorSem), 1);

  /* definition and creation of ServoSem */
  osSemaphoreDef(ServoSem);
  ServoSemHandle = osSemaphoreCreate(osSemaphore(ServoSem), 1);

  /* definition and creation of DataSem */
  osSemaphoreDef(DataSem);
  DataSemHandle = osSemaphoreCreate(osSemaphore(DataSem), 1);

  /* definition and creation of IRQSem */
  osSemaphoreDef(IRQSem);
  IRQSemHandle = osSemaphoreCreate(osSemaphore(IRQSem), 1);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of Task1 */
  osThreadDef(Task1, Task_Sensor, osPriorityHigh, 0, 128);
  Task1Handle = osThreadCreate(osThread(Task1), NULL);

  /* definition and creation of Task2 */
  osThreadDef(Task2, Task_Servo, osPriorityAboveNormal, 0, 128);
  Task2Handle = osThreadCreate(osThread(Task2), NULL);

  /* definition and creation of Task3 */
  osThreadDef(Task3, Task_Display, osPriorityNormal, 0, 128);
  Task3Handle = osThreadCreate(osThread(Task3), NULL);

  /* definition and creation of Task4 */
  osThreadDef(Task4, Task_Data, osPriorityBelowNormal, 0, 128);
  Task4Handle = osThreadCreate(osThread(Task4), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_1LINE;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 72-1;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 65535;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 1444-1;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 1000-1;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);

  /*Configure GPIO pins : PA0 PA1 PA2 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PB0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/* USER CODE BEGIN Header_Task_Sensor */
/**
 * @brief  Function implementing the Task1 thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_Task_Sensor */
void Task_Sensor(void const * argument)
{
  /* USER CODE BEGIN 5 */
  /* Infinite loop */
  for (;;)
  {
    log_data("task sensor\n");
    // read sensor
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
    osSemaphoreRelease(SensorSemHandle);
    osDelay(1000);
  }

  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_Task_Servo */
/**
 * @brief Function implementing the Task2 thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_Task_Servo */
void Task_Servo(void const * argument)
{
  /* USER CODE BEGIN Task_Servo */
  /* Infinite loop */
  for (;;)
  {
    if (osSemaphoreWait(SensorSemHandle, osWaitForever) == osOK || osSemaphoreWait(DataSemHandle, osWaitForever) == osOK)
    {
      log_data("task servo\n");
      if (Mode == Mode_auto)
      {
        log_data("task servo mode auto\n");
        if (temp > PC_threshold)
        {
          TIM3->CCR1 = 25;
          PC_rota = 0;
          //ST7735_WriteString(0, 3 * 10 * 3, rota, Font_11x18, ST7735_GREEN, ST7735_BLACK);
        }
        //>30C -> close curtain
        else
        {
          TIM3->CCR1 = 125;
          PC_rota = 180;
          //ST7735_WriteString(0, 3 * 10 * 3, rota, Font_11x18, ST7735_GREEN, ST7735_BLACK);
        }
      }
      if (Mode == Mode_manual)
      {
        log_data("task servo mode manual\n");
        TIM3->CCR1 = (int)((PC_rota + 45) / 1.8);
      }
      osSemaphoreRelease(ServoSemHandle);
    }
    osDelay(500);
  }
  /* USER CODE END Task_Servo */
}

/* USER CODE BEGIN Header_Task_Display */
/**
 * @brief Function implementing the Task3 thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_Task_Display */
void Task_Display(void const * argument)
{
  /* USER CODE BEGIN Task_Display */
  /* Infinite loop */
  for (;;)
  {
    osSemaphoreWait(ServoSemHandle, osWaitForever);
    log_data("Task Display\n");
    sprintf(buf_temp, "Temp: %d C", temp);
    sprintf(buf_humi, "Humi: %d ", humi);
    log_data(buf_temp);
    log_data("\n");
    log_data(buf_humi);
    log_data("\n");
    if (temp > PC_threshold)
    {
      sprintf(rota, "Rota: %d  ", 0);
      log_data(rota);
      log_data("\n");
      ST7735_WriteString(35, 145, "Warnning", Font_7x10, ST7735_WHITE, ST7735_BLACK);
    }
    else
    {
      sprintf(rota, "Rota: %d  ", 180);
      log_data(rota);
      log_data("\n");
      ST7735_WriteString(35, 145, "          ", Font_7x10, ST7735_WHITE, ST7735_BLACK);
    }
    sprintf(rota, "Rota: %d  ", PC_rota);
    ST7735_WriteString(0, 30, buf_temp, Font_11x18, ST7735_RED, ST7735_BLACK);
    ST7735_WriteString(0, 3 * 10 * 2, strcat(buf_humi, "%"), Font_11x18, ST7735_RED, ST7735_BLACK);
    ST7735_WriteString(0, 3 * 10 * 3, rota, Font_11x18, ST7735_GREEN, ST7735_BLACK);
    osDelay(500);
  }
  /* USER CODE END Task_Display */
}

/* USER CODE BEGIN Header_Task_Data */
/**
 * @brief Function implementing the Task4 thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_Task_Data */
void Task_Data(void const * argument)
{
  /* USER CODE BEGIN Task_Data */
  /* Infinite loop */
  for (;;)
  {
    osSemaphoreWait(IRQSemHandle, osWaitForever);
    log_data("task uart\n");
    if (strncmp(buffer_data, "Rota", 4) == 0)
    {
      log_data("check data rota\n");
      // Parse string
      //sscanf(buffer_data, "%*[^0-9]%d", &PC_rota);
      // control Servo
      sprintf(rota, "Rota: %d  ", PC_rota);
      log_data("\nPC control ");
      log_data(rota);
      ST7735_WriteString(0, 3 * 10 * 3, rota, Font_11x18, ST7735_GREEN, ST7735_BLACK);
    }
    if (strncmp(buffer_data, "Threshold", strlen("Threshold")) == 0)
    {
      log_data("check data thresh\n");
      char str_buf[] = "Thres: ";
      // Parse string
      //sscanf(buffer_data, "%s%s", label_thres, str_thres);
      // control Servo
      PC_threshold = atof(str_thres);
      strcat(str_buf, str_thres);
      log_data("\nPC set Threshold: ");
      log_data(str_thres);
      ST7735_WriteString(0, 3 * 10 * 4, str_buf, Font_11x18, ST7735_GREEN, ST7735_BLACK);
    }
    memset(buffer_data, '\0', sizeof(buffer_data));
    osSemaphoreRelease(DataSemHandle);
    osDelay(500);
  }
  /* USER CODE END Task_Data */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

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

#ifdef  USE_FULL_ASSERT
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
