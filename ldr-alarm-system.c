/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
ADC_HandleTypeDef hadc1;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

osThreadId defaultTaskHandle;
osThreadId BTreadTaskHandle;
osThreadId LDRreadTaskHandle;
osThreadId LaserWriteTaskHandle;
osThreadId BuzzerTaskHandle;
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
void StartDefaultTask(void const * argument);
void StartBTreadTask(void const * argument);
void StartLDRreadTask(void const * argument);
void StartLaserWriteTask(void const * argument);
void StartBuzzerTask(void const * argument);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#include <string.h>
#include <stdint.h>
#include "core_cm4.h"

volatile uint32_t t_edge_cycles = 0;
volatile uint8_t read_ldr = 1;
volatile uint8_t arm = 0;
volatile uint8_t alarm = 0;
volatile uint8_t was_dark = 0;
osSemaphoreId ldrSem;
osSemaphoreDef(ldrSem);

static inline void dwt_init(void){
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;   //enable trace
  #ifdef DWT_LAR
  DWT->LAR = 0xC5ACCE55;
  #endif
  DWT->CYCCNT = 0;                                  //reset counter
  DWT->CTRL  |= DWT_CTRL_CYCCNTENA_Msk;             //start counter
}

ADC_ChannelConfTypeDef sConfig = {0};
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
  MX_ADC1_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);  //aprinde laserul
  SystemCoreClockUpdate();
  dwt_init();               //porneste dwt-cyccnt
  /* USER CODE END 2 */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  ldrSem = osSemaphoreCreate(osSemaphore(ldrSem), 1);
  osSemaphoreWait(ldrSem, 0);

  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityIdle, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* definition and creation of BTreadTask */
  osThreadDef(BTreadTask, StartBTreadTask, osPriorityAboveNormal, 0, 256);
  BTreadTaskHandle = osThreadCreate(osThread(BTreadTask), NULL);

  /* definition and creation of LDRreadTask */
  osThreadDef(LDRreadTask, StartLDRreadTask, osPriorityRealtime, 0, 128);
  LDRreadTaskHandle = osThreadCreate(osThread(LDRreadTask), NULL);

  /* definition and creation of LaserWriteTask */
  osThreadDef(LaserWriteTask, StartLaserWriteTask, osPriorityLow, 0, 128);
  LaserWriteTaskHandle = osThreadCreate(osThread(LaserWriteTask), NULL);

  /* definition and creation of BuzzerTask */
  osThreadDef(BuzzerTask, StartBuzzerTask, osPriorityAboveNormal, 0, 128);
  BuzzerTaskHandle = osThreadCreate(osThread(BuzzerTask), NULL);

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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
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
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = DISABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3|GPIO_PIN_5, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PB3 PB5 */
  GPIO_InitStruct.Pin = GPIO_PIN_3|GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3|GPIO_PIN_5, GPIO_PIN_RESET);

  GPIO_InitStruct.Pin = GPIO_PIN_3|GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  /* USER CODE BEGIN 5 */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_StartBTreadTask */
/**
* @brief Function implementing the BTreadTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartBTreadTask */
void StartBTreadTask(void const * argument)
{
  /* USER CODE BEGIN StartBTreadTask */
	uint8_t c;

  /* Infinite loop */
  for(;;)
  {
	  if (HAL_UART_Receive(&huart1, &c, 1, HAL_MAX_DELAY) == HAL_OK)
	      {
	        if (c == 'A') {
	        	read_ldr = 1;
	        	arm = 1;
	        	alarm = 0;
	        	was_dark = 0;
	        	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);
	        	//HAL_UART_Transmit(&huart1, (uint8_t*)"ARMED\n", 6, 20);
	        } else if (c == 'D') {
	        	read_ldr = 0;
	        	arm = 0;
	        	alarm = 0;
	        	was_dark = 0;
	        	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);
	        	//HAL_UART_Transmit(&huart1, (uint8_t*)"DISARMED\n", 9, 20);
	        }
	      }
    osDelay(1);
  }
  /* USER CODE END StartBTreadTask */
}

/* USER CODE BEGIN Header_StartLDRreadTask */
/**
* @brief Function implementing the LDRreadTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartLDRreadTask */
void StartLDRreadTask(void const * argument)
{
  /* USER CODE BEGIN StartLDRreadTask */
	char msg[32];   //buffer pentru afisare
	uint32_t adc_value = 0;
	const uint32_t threshold_hi = 2650;    //daca trece peste, porneste buzzer-ul
	const uint32_t threshold_lo = 2600;    //daca trece sub, fasciculul loveste iar ldr-ul

  /* Infinite loop */
  for(;;)
  {
	  if (arm && read_ldr)
	      {
	        HAL_ADC_Start(&hadc1);
	        if (HAL_ADC_PollForConversion(&hadc1, 2) == HAL_OK) {
	          adc_value = HAL_ADC_GetValue(&hadc1);

	          //se trimite valoarea ADC pe UART2 la 115200
//	          int n = sprintf(msg, "LDR=%lu\r\n", adc_value);
//	          HAL_UART_Transmit(&huart2, (uint8_t*)msg, n, 10);

	          uint8_t is_dark = (!was_dark) ? (adc_value > threshold_hi)
	                                        : (adc_value > threshold_lo);

	          if (!was_dark && is_dark) {
	            was_dark = 1;
	            alarm = 1;

	            //sprintf(msg, "DEBUG: SEM RELEASE LDR=%lu\r\n", adc_value);
	            //HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 20);
	            t_edge_cycles = DWT->CYCCNT;        // t0: cand s-a detectat edge in ldr
	            osSemaphoreRelease(ldrSem);
	          } else if (was_dark && !is_dark) {
	            was_dark = 0;
	            alarm = 0;
	            //sprintf(msg, "DEBUG: BACK TO LIGHT LDR=%lu\r\n", adc_value);
	            //HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 20);
	          }
	        }
	        HAL_ADC_Stop(&hadc1);
	      } else {
	        was_dark = 0;
	        alarm = 0;
	      }
    osDelay(10);
  }
}

  /* USER CODE END StartLDRreadTask */


/* USER CODE BEGIN Header_StartLaserWriteTask */
/**
* @brief Function implementing the LaserWriteTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartLaserWriteTask */
void StartLaserWriteTask(void const * argument)
{
  /* USER CODE BEGIN StartLaserWriteTask */

  /* Infinite loop */
  for(;;)
  {
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET); // mentine pornit laserul
	  osDelay(10);
  }
}
  /* USER CODE END StartLaserWriteTask */


/* USER CODE BEGIN Header_StartBuzzerTask */
/**
* @brief Function implementing the BuzzerTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartBuzzerTask */
void StartBuzzerTask(void const * argument)
{
  /* USER CODE BEGIN StartBuzzerTask */
  /* Infinite loop */
  for(;;)
  {
	  if (osSemaphoreWait(ldrSem, osWaitForever) >= 0) {
		  // t1 cand buzzer-ul task a primit semaforul
		  uint32_t t1 = DWT->CYCCNT;
		  uint32_t dt_cycles = t1 - t_edge_cycles;
		  // conversie cicluri in ms
		  uint32_t dt_us = (uint32_t)((uint64_t)dt_cycles * 1000000ULL / SystemCoreClock);

		  //debug pe usart2 (115200)
		  char msg[48];
		  int n = sprintf(msg, "time-response=%lu cycles (%lu us)\r\n", dt_cycles, dt_us);
		  HAL_UART_Transmit(&huart2, (uint8_t*)msg, n, 20);

		  //HAL_UART_Transmit(&huart2, (uint8_t*)"DEBUG: BUZZER GOT SEM\r\n", 23, 20);
	        if (arm) {
	          HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_SET);
	          //HAL_UART_Transmit(&huart1, (uint8_t*)"ALARM: edge detected\n", 21, 20);
	        }
	      }

	      if (!arm) {
	        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);
	        while (osSemaphoreWait(ldrSem, 0) >= 0) { }
	      }
  }
}
  /* USER CODE END StartBuzzerTask */


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
