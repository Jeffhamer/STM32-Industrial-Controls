/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdbool.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define GREEN_TIME_MS	5000U
#define YELLOW_TIME_MS	2000U
#define ALL_RED_TIME_MS	1000U
#define FLASH_TIME_MS	500U

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

COM_InitTypeDef BspCOMInit;
ADC_HandleTypeDef hadc1;

TIM_HandleTypeDef htim6;

/* USER CODE BEGIN PV */
uint32_t adc_value = 0;
uint32_t green_time_ms = GREEN_TIME_MS;
uint32_t active_green_time_ms = GREEN_TIME_MS;
int32_t filtered_adc = 0;
uint32_t fault_reset_start_ms = 0;
uint8_t fault_reset_timing = 0;
//volatile uint8_t traffic_mode = 0;
volatile uint8_t mode_changed = 0;
volatile uint8_t mode_button_pressed = 0;
volatile uint8_t button_ready = 1;
volatile uint8_t traffic_fault = 0;
//uint8_t traffic_state = 0;
typedef enum
{
	A_GREEN,
	A_YELLOW,
	ALL_RED_TO_B,
	B_GREEN,
	B_YELLOW,
	ALL_RED_TO_A,
} TrafficState;

TrafficState traffic_state = A_GREEN;


volatile TrafficMode traffic_mode = MODE_AUTO;

volatile uint32_t traffic_timer = 0;
volatile uint32_t system_ms = 0;
//uint32_t old_blink_period = 0;
float adc_voltage = 0.0f;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM6_Init(void);
static void MX_ADC1_Init(void);
/* USER CODE BEGIN PFP */
void SetTrafficA(uint8_t red, uint8_t yellow, uint8_t green);
void SetTrafficB(uint8_t red, uint8_t yellow, uint8_t green);
void ProcessModeButton(void);
void InitializeTrafficMode(void);
void RunAutomaticTraffic(void);
void RunFourWayStop(void);
void RunTwoWayStop(void);
void ReadPotentiometer(void);
void CheckTrafficFaults(void);
void HandleTrafficFaults(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_TIM6_Init();
  MX_ADC1_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start_IT(&htim6);

  /* USER CODE END 2 */

  /* Initialize led */
  BSP_LED_Init(LED_GREEN);

  /* Initialize USER push-button, will be used to trigger an interrupt each time it's pressed.*/
  BSP_PB_Init(BUTTON_USER, BUTTON_MODE_EXTI);

  /* Initialize COM1 port (115200, 8 bits (7-bit data + 1 stop bit), no parity */
  BspCOMInit.BaudRate   = 115200;
  BspCOMInit.WordLength = COM_WORDLENGTH_8B;
  BspCOMInit.StopBits   = COM_STOPBITS_1;
  BspCOMInit.Parity     = COM_PARITY_NONE;
  BspCOMInit.HwFlowCtl  = COM_HWCONTROL_NONE;
  if (BSP_COM_Init(COM1, &BspCOMInit) != BSP_ERROR_NONE)
  {
    Error_Handler();
  }

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

    /* USER CODE END WHILE */


    /* USER CODE BEGIN 3 */
	  ProcessModeButton();
	  InitializeTrafficMode();
	  ReadPotentiometer();

	  switch (traffic_mode)
	  {
	  	  case MODE_AUTO:
	  		  /* Automatic traffic light sequence */
	  		  RunAutomaticTraffic();
			  break;
	  	  case MODE_FOUR_WAY_STOP:
	  		  /* 4-way stop - flash both RED */
	  		  RunFourWayStop();
			  break;
	  	  case MODE_TWO_WAY_STOP:
	  		  /* 2-way stop
	  		     Traffic A = Flashing YELLOW
	  		     Traffic B = Flashing RED */
	  		  RunTwoWayStop();
			  break;
          case MODE_FAULT:
              SetTrafficA(1, 0, 0);
              SetTrafficB(1, 0, 0);
              break;
	  	  default:
	  		traffic_mode = MODE_AUTO;
	  		mode_changed = 1;
	  		break;
	  }
	  CheckTrafficFaults();
	  HandleTrafficFaults();
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
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV4;
  RCC_OscInitStruct.PLL.PLLN = 85;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
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

  ADC_MultiModeTypeDef multimode = {0};
  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.GainCompensation = 0;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc1.Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the ADC multi-mode
  */
  multimode.Mode = ADC_MODE_INDEPENDENT;
  if (HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_2CYCLES_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief TIM6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM6_Init(void)
{

  /* USER CODE BEGIN TIM6_Init 0 */

  /* USER CODE END TIM6_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM6_Init 1 */

  /* USER CODE END TIM6_Init 1 */
  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 16999;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 9;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM6_Init 2 */

  /* USER CODE END TIM6_Init 2 */

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
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, TRAFFIC_A_YELLOW_Pin|TRAFFIC_A_GREEN_Pin|TRAFFIC_B_RED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, TRAFFIC_B_YELLOW_Pin|TRAFFIC_B_GREEN_Pin|TRAFFIC_A_RED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : TRAFFIC_A_YELLOW_Pin TRAFFIC_A_GREEN_Pin TRAFFIC_B_RED_Pin */
  GPIO_InitStruct.Pin = TRAFFIC_A_YELLOW_Pin|TRAFFIC_A_GREEN_Pin|TRAFFIC_B_RED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : TRAFFIC_B_YELLOW_Pin TRAFFIC_B_GREEN_Pin TRAFFIC_A_RED_Pin */
  GPIO_InitStruct.Pin = TRAFFIC_B_YELLOW_Pin|TRAFFIC_B_GREEN_Pin|TRAFFIC_A_RED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : MODE_BUTTON_Pin */
  GPIO_InitStruct.Pin = MODE_BUTTON_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(MODE_BUTTON_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

void HandleTrafficFaults(void)
{
	if (traffic_fault)
	{
		SetTrafficA(1, 0, 0);
		SetTrafficB(1, 0, 0);
	}
}

void CheckTrafficFaults(void)
{
	GPIO_PinState traffic_a_green;
	GPIO_PinState traffic_b_green;

	traffic_a_green = HAL_GPIO_ReadPin(TRAFFIC_A_GREEN_GPIO_Port, TRAFFIC_A_GREEN_Pin);
	traffic_b_green = HAL_GPIO_ReadPin(TRAFFIC_B_GREEN_GPIO_Port, TRAFFIC_B_GREEN_Pin);

	if ((traffic_a_green == GPIO_PIN_SET) && (traffic_b_green == GPIO_PIN_SET))
	{
		traffic_fault = 1;
		traffic_mode = MODE_FAULT;
		mode_changed = 1;
	}
}

void ReadPotentiometer(void)
{
	HAL_ADC_Start(&hadc1);

	if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK)
	{
		adc_value = HAL_ADC_GetValue(&hadc1);
		filtered_adc = filtered_adc + (((int32_t)adc_value - filtered_adc)/ 16);
		green_time_ms = 2000U + (((uint32_t)filtered_adc * 8000U) / 4095u);
	}
	HAL_ADC_Stop(&hadc1);
}
void RunTwoWayStop(void)
{
	  if (traffic_timer >= FLASH_TIME_MS)
	  {
		  HAL_GPIO_TogglePin(TRAFFIC_A_YELLOW_GPIO_Port, TRAFFIC_A_YELLOW_Pin);
		  HAL_GPIO_TogglePin(TRAFFIC_B_RED_GPIO_Port, TRAFFIC_B_RED_Pin);
		  traffic_timer = 0;
	  }
}

void RunFourWayStop(void)
{
	  if (traffic_timer >= FLASH_TIME_MS)
	  {
		  HAL_GPIO_TogglePin(TRAFFIC_A_RED_GPIO_Port, TRAFFIC_A_RED_Pin);
		  HAL_GPIO_TogglePin(TRAFFIC_B_RED_GPIO_Port, TRAFFIC_B_RED_Pin);
		  traffic_timer = 0;
	  }
}

void RunAutomaticTraffic(void)
{
	  switch (traffic_state)
	  {
	  	  case A_GREEN:
	  		/* Traffic A Green Traffic B Red*/
	  		  SetTrafficA(0, 0, 1);

	  		  SetTrafficB(1, 0, 0); //test

	  		  if (traffic_timer >= active_green_time_ms)
	  		  {
	  			  traffic_state = A_YELLOW;
	  			  traffic_timer = 0;
	  		  }
	  		  break;
	  	    case A_YELLOW:
	  	    /* Traffic A yellow Traffic B Red*/
		  		  SetTrafficA(0, 1, 0);

		  		  SetTrafficB(1, 0, 0);

	  	    	if (traffic_timer >= YELLOW_TIME_MS)
	  	    	{
	  	    		traffic_state = ALL_RED_TO_B;
	  	    		traffic_timer = 0;
	  	    	}
	  	    	break;
	  	     case ALL_RED_TO_B:
	  		 /* ALL RED clearance */
		  		  SetTrafficA(1, 0, 0);

		  		  SetTrafficB(1, 0, 0);

	  	    	 if (traffic_timer >= ALL_RED_TIME_MS)
	  	    	 {
	  	    		 active_green_time_ms = green_time_ms;
	  	    		 traffic_state = B_GREEN;
	  	    		 traffic_timer = 0;
	  	    	 }
	  	    	 break;
		  	  case B_GREEN:
		  		/* Traffic A Red Traffic B Green */
		  		  SetTrafficA(1, 0, 0);

		  		  SetTrafficB(0, 0, 1);

		  		  if (traffic_timer >= active_green_time_ms)
		  		  {
		  			  traffic_state = B_YELLOW;
		  			  traffic_timer = 0;
		  		  }
		  		  break;
		  	    case B_YELLOW:
		  	    /* Traffic A Red Traffic B Yellow */
	  		  		  SetTrafficA(1, 0, 0);

	  		  		  SetTrafficB(0, 1, 0);

		  	    	if (traffic_timer >= YELLOW_TIME_MS)
		  	    	{
		  	    		traffic_state = ALL_RED_TO_A;
		  	    		traffic_timer = 0;
		  	    	}
		  	    	break;
		  	     case ALL_RED_TO_A:
		  		 /* ALL RED clearance */
	  		  		  SetTrafficA(1, 0, 0);

	  		  		  SetTrafficB(1, 0, 0);

		  	    	 if (traffic_timer >= ALL_RED_TIME_MS)
		  	    	 {
		  	    		 active_green_time_ms = green_time_ms;
		  	    		 traffic_state = A_GREEN;
		  	    		 traffic_timer = 0;
		  	    	 }
		  	    	 break;

	  }
}

void InitializeTrafficMode(void)
{
	  if (mode_changed)
	  {
		  mode_changed = 0;
		  traffic_timer = 0;
	        switch (traffic_mode)
	        {
	            case MODE_AUTO:
	            	active_green_time_ms = green_time_ms;
	                traffic_state = A_GREEN;
	                SetTrafficA(0, 0, 1);
	                SetTrafficB(1, 0, 0);
	                break;

	            case MODE_FOUR_WAY_STOP:
	                SetTrafficA(0, 0, 0);
	                SetTrafficB(0, 0, 0);
	                break;

	            case MODE_TWO_WAY_STOP:
	                SetTrafficA(0, 0, 0);
	                SetTrafficB(0, 0, 0);
	                break;
	            case MODE_FAULT:
	                SetTrafficA(1, 0, 0);
	                SetTrafficB(1, 0, 0);
	                break;
	        }
	  }

}

void ProcessModeButton(void)
{
	if (traffic_fault)
	{
		if (HAL_GPIO_ReadPin(MODE_BUTTON_GPIO_Port, MODE_BUTTON_Pin) == GPIO_PIN_RESET)
		{
			/* Button is being held down */
			if (!fault_reset_timing)
			{
				fault_reset_start_ms = system_ms;
				fault_reset_timing = 1;
			}
			if ((system_ms - fault_reset_start_ms) >= FAULT_RESET_TIME_MS)
			{
				traffic_fault = 0;
				traffic_mode = MODE_AUTO;
				mode_changed = 1;
				mode_button_pressed = 0;
				button_ready = 0;
			}
		}
		else
		{
			/* Button was released before 2 seconds */
			fault_reset_timing = 0;
		}
		return;
	}
	/* Re-Arm after the button has been released */
	if(HAL_GPIO_ReadPin(MODE_BUTTON_GPIO_Port, MODE_BUTTON_Pin) == GPIO_PIN_SET)
	{
		button_ready = 1;
	}
	  if(mode_button_pressed && button_ready && !traffic_fault)
	  {
		  mode_button_pressed = 0;
		  button_ready = 0;
		  //traffic_mode++;
		  switch (traffic_mode)
		  {
		  case MODE_AUTO:
			  traffic_mode = MODE_FOUR_WAY_STOP;
			  break;
		  case MODE_FOUR_WAY_STOP:
			  traffic_mode = MODE_TWO_WAY_STOP;
			  break;
		  case MODE_TWO_WAY_STOP:
			  traffic_mode = MODE_AUTO;
			  break;
		  case MODE_FAULT:
			  /* Fault mode cannot be changed with a normal button press */
		  }
		  mode_changed = 1;

		  traffic_timer = 0;
	  }

}

void SetTrafficA(uint8_t red, uint8_t yellow, uint8_t green)
{
	HAL_GPIO_WritePin(TRAFFIC_A_RED_GPIO_Port, TRAFFIC_A_RED_Pin, red ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(TRAFFIC_A_YELLOW_GPIO_Port, TRAFFIC_A_YELLOW_Pin, yellow ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(TRAFFIC_A_GREEN_GPIO_Port, TRAFFIC_A_GREEN_Pin, green ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void SetTrafficB(uint8_t red, uint8_t yellow, uint8_t green)
{
	HAL_GPIO_WritePin(TRAFFIC_B_RED_GPIO_Port, TRAFFIC_B_RED_Pin, red ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(TRAFFIC_B_YELLOW_GPIO_Port, TRAFFIC_B_YELLOW_Pin, yellow ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(TRAFFIC_B_GREEN_GPIO_Port, TRAFFIC_B_GREEN_Pin, green ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void HAL_GPIO_EXT1_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == MODE_BUTTON_Pin)
	{
		HAL_GPIO_TogglePin(TRAFFIC_A_RED_GPIO_Port, TRAFFIC_A_RED_Pin);
	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim->Instance == TIM6)
	{
		traffic_timer++;
		system_ms++;
	}
}

/*void BSP_PB_Callback(Button_TypeDef Button)
{
	static uint32_t last_press = 0;
	uint32_t now = HAL_GetTick();
	if (Button == BUTTON_USER)
	{
		if ((now - last_press) > 50)
		{
			if (blink_period == 1000)
			{
				blink_period = 500;
			} else if (blink_period == 500)
			{
				blink_period = 250;
			} else
			{
				blink_period = 1000;
			}

		//__HAL_TIM_SET_AUTORELOAD(&htim6, (blink_period * 10) - 1);
		//__HAL_TIM_SET_COUNTER(&htim6, 0);
			//blink_enabled = !blink_enabled;
			last_press = now;
		}
	}
}*/
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
