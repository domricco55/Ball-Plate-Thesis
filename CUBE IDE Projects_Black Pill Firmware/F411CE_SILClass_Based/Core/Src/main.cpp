/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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
#include "usb_device.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

//Official Libraries
#include <iostream>
using namespace std;
#include <math.h>

//Driver Files
#include "IMUDriver.hpp"
#include "TouchscreenDriver.hpp"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

// Touch Panel Pin Definitions:
#define Pin_LL GPIO_PIN_0
#define Port_LL GPIOB

#define Pin_UL GPIO_PIN_1
#define Port_UL GPIOB

#define Pin_LR GPIO_PIN_2
#define Port_LR GPIOB

#define Pin_UR GPIO_PIN_10
#define Port_UR GPIOB

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1; //IS THIS NOT BEING USED?

I2C_HandleTypeDef hi2c1;
DMA_HandleTypeDef hdma_i2c1_rx;

TIM_HandleTypeDef htim5;
TIM_HandleTypeDef htim9;
TIM_HandleTypeDef htim10;
TIM_HandleTypeDef htim11;

UART_HandleTypeDef huart1;
DMA_HandleTypeDef hdma_usart1_tx;
DMA_HandleTypeDef hdma_usart1_rx;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_ADC1_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM11_Init(void);
static void MX_TIM9_Init(void);
static void MX_TIM5_Init(void);
static void MX_DMA_Init(void);
static void MX_TIM10_Init(void);
/* USER CODE BEGIN PFP */

// Functions used later are listed here to prevent implicit definition error.
void UserInterface(void);
void CommunicationResponse(float xposTS, float xvelTS, float yposTS, float yvelTS, float IMUx, float GyroX,
						   float IMUy, float GyroY);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

//Global variables in main go (for the most part) here:
namespace GlobalVars
{
	//Logic Flags:
	uint8_t timeconsistency = 0; //A boolean that alternates 1 and 0 to verify 5 ms timing on the Simulink data collection end.
	uint8_t Operational = 0; //Flag to let HAL_GPIO_EXTI_Callback know that it can run the MATLAB communication code

	//Motors:
	float m0tor = 0; //Motor command for axis 0 (torque)
	float m0tor_offst = 0; //Motor command offset for axis 0 (torque)
	float m1tor = 0; //Motor command for axis 1 (torque)
	float m1tor_offst = 0; //Motor command offset for axis 1 (torque)

	//Encoder Zero Position
	float enc_axis0zero = 0; //The zero position of the encoder on axis 0
	float enc_axis1zero = 0; //The zero position of the encoder on axis 1

	//IMU Reading offsets
	float IMUx_offset = 0; //The EulerX IMU reading offset
	float IMUy_offset = 0; //The EulerY IMU reading offset

	//State variables
	uint8_t UI_state = 0; //User interface task's state variable


}

// Communication variables
extern uint8_t buffer[64]; //receive buffer from USB
extern uint8_t usb_Rx_flag; //flag is set true when a USB message is received
char UARTstr[64] = {0}; //for messages to send through UART
char strUSB[100] = {0}; //for messages to send back through USB
char message[64] = {0}; //for message to break down string sent from PC

//Define array of touchscreen pins required:
static uint32_t pinarray[] = {Pin_LL, Pin_UL, Pin_LR, Pin_UR, GPIO_PIN_13}; //All pins in array, as well as pin 13 for LED on black pill
static GPIO_TypeDef* pinport[] = {Port_LL, Port_UL, Port_LR, Port_UR, GPIOC};

//Initialize IMU Class
IMUDriver imu (&hi2c1, &htim11); //Initializes the imu object, but does not fully initialize the IMU

//Initialize Touchscreen Class
TouchscreenDriver TS (&hadc1, &htim10, &htim9, pinarray, pinport); //Initializes the touch panel object, but does not fully initialize

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
  MX_USB_DEVICE_Init();
  MX_DMA_Init();
  MX_USART1_UART_Init();
  MX_ADC1_Init();
  MX_I2C1_Init();
  MX_TIM11_Init();
  MX_TIM9_Init();
  MX_TIM5_Init();
  MX_TIM10_Init();
  /* USER CODE BEGIN 2 */

  //**FOR THE ABOVE, MAKE SURE DMA IS INIT BEFORE THE PERIPHERALS THAT USE IT**

  HAL_Delay(5000); //Delay to ensure that debugging/flashing new software is easier.

  //Initialize the IMU for use in the zeroing process
  imu.Initialization();

  //Initialize Touchscreen Class
  TS.Initialization();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

	  //Main code is solely used for waiting to process the touch panel data at the moment, can also place controller here as well
	  if(TS.tsflg == 1)
	  {
		  TS.ProcessPositionData(); //Process the position data
		  TS.tsflg = 0; //Reset the flag for the next time the data has been collected for processing
	  }

	  //Run User Interface Task
	  UserInterface();

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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 192;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
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
	  hadc1.Init.DMAContinuousRequests = ENABLE;
	  hadc1.Init.EOCSelection = ADC_EOC_SEQ_CONV;
	  if (HAL_ADC_Init(&hadc1) != HAL_OK)
	  {
	    Error_Handler();
	  }
	  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
	  */
	  sConfig.Channel = ADC_CHANNEL_7;
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
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief TIM5 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM5_Init(void)
{

  /* USER CODE BEGIN TIM5_Init 0 */

  /* USER CODE END TIM5_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM5_Init 1 */

  /* USER CODE END TIM5_Init 1 */
  htim5.Instance = TIM5;
  htim5.Init.Prescaler = 0;
  htim5.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim5.Init.Period = 4294967295;
  htim5.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim5.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim5) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim5, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim5, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM5_Init 2 */

  /* USER CODE END TIM5_Init 2 */

}

/**
  * @brief TIM9 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM9_Init(void)
{

  /* USER CODE BEGIN TIM9_Init 0 */

  /* USER CODE END TIM9_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};

  /* USER CODE BEGIN TIM9_Init 1 */

  /* USER CODE END TIM9_Init 1 */
  htim9.Instance = TIM9;
  htim9.Init.Prescaler = 96-1;
  htim9.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim9.Init.Period = 65535;
  htim9.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim9.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim9) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim9, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM9_Init 2 */

  /* USER CODE END TIM9_Init 2 */

}

/**
  * @brief TIM10 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM10_Init(void)
{

  /* USER CODE BEGIN TIM10_Init 0 */

  /* USER CODE END TIM10_Init 0 */

  /* USER CODE BEGIN TIM10_Init 1 */

  /* USER CODE END TIM10_Init 1 */
  htim10.Instance = TIM10;
  htim10.Init.Prescaler = 96-1;
  htim10.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim10.Init.Period = 150-1;
  htim10.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim10.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim10) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM10_Init 2 */

  /* USER CODE END TIM10_Init 2 */

}

/**
  * @brief TIM11 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM11_Init(void)
{

  /* USER CODE BEGIN TIM11_Init 0 */

  /* USER CODE END TIM11_Init 0 */

  /* USER CODE BEGIN TIM11_Init 1 */

  /* USER CODE END TIM11_Init 1 */
  htim11.Instance = TIM11;
  htim11.Init.Prescaler = 96 - 1;
  htim11.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim11.Init.Period = 10000 - 1;
  htim11.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim11.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim11) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM11_Init 2 */

  /* USER CODE END TIM11_Init 2 */

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
  huart1.Init.BaudRate = 115200;
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
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

	/* DMA controller clock enable */
	__HAL_RCC_DMA2_CLK_ENABLE();
	__HAL_RCC_DMA1_CLK_ENABLE();

	/* DMA interrupt init */

	/* DMA1_Stream0_IRQn interrupt configuration */
	//I2C Stream
	HAL_NVIC_SetPriority(DMA1_Stream0_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(DMA1_Stream0_IRQn);

	/* DMA2_Stream0_IRQn interrupt configuration */
	//ADC Stream
	HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 2, 2);
	HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);

	/* DMA2_Stream7_IRQn interrupt configuration */
	//USART Stream
	HAL_NVIC_SetPriority(DMA2_Stream7_IRQn, 1, 1);
	HAL_NVIC_EnableIRQ(DMA2_Stream7_IRQn);

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
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_10, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PB0 PB1 PB2 PB10 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PB13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PA9 */
  GPIO_InitStruct.Pin = GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

}

/* USER CODE BEGIN 4 */

//==============Callbacks=================//

//Timer channel callbacks for Touchscreen Settling Time Delay (100-150 us) and IMU Refresh Rate Delay (10 ms)
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	//Touchscreen Switching Interrupt
	if(htim == &htim10)
	{
		TS.TIM_Callback();
	}
	else if(htim == &htim11) //IMU Timing Interrupt
	{
		imu.PollOrientation();
	}

}

// Called when ADC buffer is completely filled
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {

	if(hadc == &hadc1)
	{
		//Toggle the touchscreen to read the opposite direction (x->y or y->x)
		TS.ADC_Callback();
	}

}

//Called when USB buffer receives value from MATLAB script
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if(GlobalVars::Operational) //If operational send the sensor data over USB to computer running Simulink
	{
		CommunicationResponse(TS.xpos, TS.xvel, TS.ypos, TS.yvel, imu.EulerX, imu.AngularVelX, imu.EulerY, imu.AngularVelY);

	}

}

//Called when I2C completed.
void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c)
{

	imu.ConvertOrientation(); //Attempt this

}



//===================Additional Functions======================//

void CommunicationResponse(float xposTS, float xvelTS, float yposTS, float yvelTS, float IMUx, float GyroX,
						   float IMUy, float GyroY)
{
	//Incorporate the IMU offsets and alter the signs of the readings to match the kinematics used in the
	//controller design
	//(want simulink to read 0 when the plate is in the position the user zeroed to)
	IMUx = -(IMUx - GlobalVars::IMUx_offset); //Sign on this axis needs to be switched
	IMUy = IMUy - GlobalVars::IMUy_offset;

	//Fix sign on gyro as well
	GyroX = -GyroX;

	// For processing string
	static char delim[] = " ";
	char *ptr;

	char ODrivemessage[100] = {0}; //Messages that need to be sent to the ODrive


	//============USB SEND FIRST===============//

	if (GlobalVars::timeconsistency == 1) //Verify that MATLAB model and STM32 have full parity (Check to ensure new messages are getting through)
	{
		GlobalVars::timeconsistency = 0;
	} else {
		GlobalVars::timeconsistency = 1;
	}

	//Create the message to send back to the Simulink model
	sprintf(strUSB,"%.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %u \n",
			xposTS, xvelTS, IMUy, GyroY, yposTS, yvelTS,  IMUx,
			GyroX, GlobalVars::timeconsistency);

	strUSB[strlen(strUSB)+1] = '\0'; //ASCII NULL to terminate string

	CDC_Transmit_FS((uint8_t *) strUSB, strlen(strUSB));



	//============UART SEND============//
	if (buffer[0] != '\0') //Double-Check if USB buffer has received message from Host PC
	{

		memcpy(message,buffer,sizeof(message)); //Copy buffer into message for string processing

		//memset(buffer, '\0', 64);  // clear the buffer for next message

		//Now, process message:

		ptr = strtok(message, delim); //Initial cut, isolates first float of string

		GlobalVars::m1tor = atof(ptr) + GlobalVars::m1tor_offst; //Convert string of motor torque input to float for sprintf construction

		ptr = strtok(NULL,delim); //Iterate to next section of message

		//Here, the sign of the motor 0 torque command from Simulink is inverted to produce
		//the correct torque direction
		GlobalVars::m0tor = -atof(ptr) + GlobalVars::m0tor_offst; //Save second space-separated part of string to double of torque value for processing

		//-------------Now, send message to O-Drive and confirmation to PC:-------------

		//Turn message into torque command (v for velocity, c for current/torque, q for position)
		sprintf(UARTstr, "c 0 %.4g\nc 1 %.4g\n", GlobalVars::m0tor, GlobalVars::m1tor);

		UARTstr[strlen(UARTstr)+1] = '\0'; //truncate string

		HAL_UART_Transmit_DMA(&huart1,(uint8_t *) UARTstr, strlen(UARTstr)); //Transmit Message over UART
	}
}

void UserInterface(void)
{

	//Variables accessible to all states of the UI task:
	//Messages
	static char ODrivemessage[100] = {0}; //Messages that need to be sent to the ODrive
	static char str[200] = {0}; //for messages to send through USB
	static char ODriveReceive[50] = {0}; //For receiving messages from O-Drive
	static uint8_t UIprint = 1; //Flag to let the state know to print its USB message

	//encoder setpoints for zeroing procedure
	static float axis0encstpt = 0;
	static float axis1encstpt = 0;

	//encoder readings for zeroing procedure
	static float axis0encpos = 0;
	static float axis1encpos = 0;

	static const float axis0torqueconstant = 0.365; // N-m/A
	static const float axis1torqueconstant = 0.35; // N-m/A

	// For processing string
	static char delim[] = " ";
	static char *ptr;

	//Run the user interface state machine
	switch(GlobalVars::UI_state)
	{

		case 0 : //Wait for the user to press any key while in the serial terminal
		{
			if(usb_Rx_flag)
			{
				usb_Rx_flag = 0; //clear the usb message received flag
				GlobalVars::UI_state = 1;

			}


			break;
		}

		case 1: //Main menu
		{
			//Print UI
			if (UIprint)
			{
				HAL_Delay(5); //Just in case something tried to print after "operational" state ends
				sprintf(str,"Main Menu. What would you like to do? \r\n"
							"1. Calibrate the motors \r\n"
							"2. Conduct plate zeroing procedure \r\n"
							"3. Reset ODrive faults \r\n"
							"4. Go operational \r\n"
							"\n");
				CDC_Transmit_FS((uint8_t *) str, strlen(str));

				UIprint = 0;

			}

			//If user input was received
			if (usb_Rx_flag)
			{
				UIprint = 1; //Lets the next state print its UI message
				usb_Rx_flag = 0; //clear the usb message received flag
				//Interpret the user input
				switch(buffer[0])
				{
				case '1': //User selected "Commutate the Motors"
				{
					GlobalVars::UI_state = 2; //Go to state for displaying motor calibration prep prompt
					break;
				}

				case '2': //User selected "Conduct Plate Zeroing Procedure"
				{

					//Prepare the ODrive for the zeroing procedure

					//=====Remove errors on each axis for endstop
					sprintf(ODrivemessage, "w axis0.error 0\nw axis1.error 0\n"); //Clear ODrive errors that could have occurred from endstop.
					ODrivemessage[strlen(ODrivemessage)+1] = '\0';
					HAL_UART_Transmit_DMA(&huart1,(uint8_t *) ODrivemessage, strlen(ODrivemessage)); //Transmit Message over UART

					HAL_Delay(20); //Give time to process commands

					//======Get encoder position for both axes, set the position (in turns) to variables

					//Encoder Axis 0 position
					sprintf(ODrivemessage, "f 0\n"); //request motor parameters from axis 0
					ODrivemessage[strlen(ODrivemessage)+1] = '\0';
					HAL_UART_Transmit_DMA(&huart1,(uint8_t *) ODrivemessage, strlen(ODrivemessage)); //Transmit Message over UART

					HAL_UART_Receive(&huart1,(uint8_t *) ODriveReceive, 16, 50);

					ptr = strtok(ODriveReceive, delim); //Initial cut, should isolate for first

					axis0encpos = atof(ptr);
					axis0encstpt = axis0encpos; //Setpoint will be the current position

					HAL_Delay(5);

					//Encoder Axis 1 position
					sprintf(ODrivemessage, "f 1\n"); //request motor parameters from axis 0
					ODrivemessage[strlen(ODrivemessage)+1] = '\0';
					HAL_UART_Transmit_DMA(&huart1,(uint8_t *) ODrivemessage, strlen(ODrivemessage)); //Transmit Message over UART

					HAL_UART_Receive(&huart1,(uint8_t *) ODriveReceive, 16, 10);

					ptr = strtok(ODriveReceive, delim); //Initial cut, should isolate for first

					axis1encpos = atof(ptr);
					axis1encstpt = axis1encpos; //Setpoint will be the current position

					HAL_Delay(5);

					//Send command to change position setpoint to existing position
					sprintf(ODrivemessage, "q 0 %.4f\nq 1 %.4f\n", axis0encstpt, axis1encstpt); //request motor parameters from axis 0
					ODrivemessage[strlen(ODrivemessage)+1] = '\0';
					HAL_UART_Transmit_DMA(&huart1,(uint8_t *) ODrivemessage, strlen(ODrivemessage)); //Transmit Message over UART

					HAL_Delay(5);

					//Start closed-loop control
					sprintf(ODrivemessage, "w axis0.requested_state 8\nw axis1.requested_state 8\n"); //request motor parameters from axis 0
					ODrivemessage[strlen(ODrivemessage)+1] = '\0';
					HAL_UART_Transmit_DMA(&huart1,(uint8_t *) ODrivemessage, strlen(ODrivemessage)); //Transmit Message over UART

					HAL_Delay(5);

					GlobalVars::UI_state = 4; //Go to the state for zeroing the platform
					break;
				}

				case '3': //User selected "Reset ODrive Faults"
				{
					GlobalVars::UI_state = 1; //Self transition

					//=====Remove errors on each axis for endstop
					sprintf(ODrivemessage, "w axis0.error 0\nw axis1.error 0\n"); //Clear ODrive errors that could have occurred from endstop.
					ODrivemessage[strlen(ODrivemessage)+1] = '\0';
					HAL_UART_Transmit_DMA(&huart1,(uint8_t *) ODrivemessage, strlen(ODrivemessage)); //Transmit Message over UART

					//Print confirmation message to UI
					sprintf(str,"ODrive reset command sent. \r\n\n");
					CDC_Transmit_FS((uint8_t *) str, strlen(str));

					HAL_Delay(5);
					break;
				}

				case '4': //User selected "Go operational"
				{
					GlobalVars::Operational = 1; //Set the operational flag so the MATLAB USB code can run
					GlobalVars::UI_state = 5; //Go to the "Operational" state - Simulink model can now run
					break;
				}

				default:
					sprintf(str,"Invalid input. \r\n\n");
					CDC_Transmit_FS((uint8_t *) str, strlen(str));

					HAL_Delay(5);
					//UIprint = 0; //Was set true before switch case, sets false again if input was invalid
					break;

				}
			}
			break;
		}

		case 2: //Calibration preparation. Wait for user to press C indicating they have disconnected the motor arms
		{
			if(UIprint)
			{
				sprintf(str,"Ensure that the platform arms are disconnected. Press c when complete.\r\n\n");
				CDC_Transmit_FS((uint8_t *) str, strlen(str)); //Transmit Message over UART. NOTE: SWITCH TO CDC TRANSMIT FOR REAL SYSTEM
				UIprint = 0;
			}

			//If user input was received
			if (usb_Rx_flag)
			{
				UIprint = 1; //Lets the next state print its UI message
				usb_Rx_flag = 0; //clear the usb message received flag
				if(buffer[0] == 'c') //If user pressed 'c'
				{
					GlobalVars::UI_state = 3; //Go to the "Commutate motors" state.
				}

				else
				{
					sprintf(str,"Invalid input. \r\n\n");
					CDC_Transmit_FS((uint8_t *) str, strlen(str));
					UIprint = 0; //Was set true before switch case, sets false again if input was invalid
				}

			}

			break;
		}

		case 3: //Calibrating the motors
		{
			if(UIprint)
			{
			sprintf(str,"Calibrating Motors. When motor calibration is complete, attach the motor arms and press 'c'."
						"You will be redirected to the main menu. \r\n\n");
			CDC_Transmit_FS((uint8_t *) str, strlen(str));

			//=====Remove errors on each axis for endstop (Motors won't calibrate otherwise)
			sprintf(ODrivemessage, "w axis0.error 0\nw axis1.error 0\n"); //Clear ODrive errors that could have occurred from endstop.
			ODrivemessage[strlen(ODrivemessage)+1] = '\0';
			HAL_UART_Transmit_DMA(&huart1,(uint8_t *) ODrivemessage, strlen(ODrivemessage)); //Transmit Message over UART

			HAL_Delay(20); //Give time to process error reset

			//ODrive message to put motors into calibration mode. "requested_state" 7 is for calibration process.
			sprintf(ODrivemessage, "w axis0.requested_state 7\nw axis1.requested_state 7\n");
			ODrivemessage[strlen(ODrivemessage)+1] = '\0'; //Want ODrive number 0? Device address?

			//Transmit Message to ODrive over UART
			HAL_UART_Transmit_DMA(&huart1,(uint8_t *) ODrivemessage, strlen(ODrivemessage));
			UIprint = 0;
			}

			//If user input was received
			if (usb_Rx_flag)
			{
				UIprint = 1; //Lets the next state print its UI message
				usb_Rx_flag = 0; //clear the usb message received flag
				if(buffer[0] == 'c') //If user pressed 'c'
				{
					GlobalVars::UI_state = 1; //Go to the "Main menu" state.

					//=====Remove errors on each axis for endstop
					//(In case user tripped end stop while placing motor arms)
					sprintf(ODrivemessage, "w axis0.error 0\nw axis1.error 0\n");
					ODrivemessage[strlen(ODrivemessage)+1] = '\0';
					HAL_UART_Transmit_DMA(&huart1,(uint8_t *) ODrivemessage, strlen(ODrivemessage)); //Transmit Message over UART

				}

				else
				{
					sprintf(str,"Invalid input. \r\n\n");
					CDC_Transmit_FS((uint8_t *) str, strlen(str));
					UIprint = 0; //Was set true before switch case, sets false again if input was invalid
				}
			}
			break;
		}

		case 4: //Plate zeroing procedure
		{
			if (UIprint)
			{
				sprintf(str,"Platform is ready to zero. Use the WASD keys"
							" to position the top plate and c to complete zeroing process.\r\n\n");
				CDC_Transmit_FS((uint8_t *) str, strlen(str)); //Transmit Message over USB
				UIprint = 0;
			}

			//If user input was received
			if (usb_Rx_flag)
			{
				usb_Rx_flag = 0; //clear the usb message received flag

				//Interpret the user input (WASD zeroing)
				switch(buffer[0])
				{

					case 'w':
					{
						//Tilt platform +y
						sprintf(str,"Tilt platform +y\r\n");
						CDC_Transmit_FS((uint8_t *) str, strlen(str));

						axis0encstpt -= 0.001; //Increment the motor setpoint

						sprintf(ODrivemessage, "q 0 %.4f\n", axis0encstpt); //Send position control command
						ODrivemessage[strlen(ODrivemessage)+1] = '\0';
						HAL_UART_Transmit_DMA(&huart1,(uint8_t *) ODrivemessage, strlen(ODrivemessage)); //Transmit Message over UART

						HAL_Delay(5); //Give time for ODrive to implement the tilt command
						break;
					}

					case 'a':
					{
						//Tilt platform -x
						sprintf(str,"Tilt platform -x\r\n");
						CDC_Transmit_FS((uint8_t *) str, strlen(str));

						axis1encstpt += 0.001; //Increment the motor setpoint

						sprintf(ODrivemessage, "q 1 %.4f\n", axis1encstpt); //Send position control command
						ODrivemessage[strlen(ODrivemessage)+1] = '\0';
						HAL_UART_Transmit_DMA(&huart1,(uint8_t *) ODrivemessage, strlen(ODrivemessage)); //Transmit Message over UART

						HAL_Delay(5); //Give time for ODrive to implement the tilt command
						break;
					}

					case 's':
					{
						//Tilt platform -y
						sprintf(str,"Tilt platform -y\r\n");
						CDC_Transmit_FS((uint8_t *) str, strlen(str));

						axis0encstpt += 0.001; //Increment the motor setpoint

						sprintf(ODrivemessage, "q 0 %.4f\n", axis0encstpt); //Send position control command
						ODrivemessage[strlen(ODrivemessage)+1] = '\0';
						HAL_UART_Transmit_DMA(&huart1,(uint8_t *) ODrivemessage, strlen(ODrivemessage)); //Transmit Message over UART

						HAL_Delay(5); //Give time for ODrive to implement the tilt command
						break;
					}

					case 'd':
					{
						//Tilt platform +x
						sprintf(str,"Tilt platform +x\r\n");
						CDC_Transmit_FS((uint8_t *) str, strlen(str));

						axis1encstpt -= 0.001; //Increment the motor setpoint

						sprintf(ODrivemessage, "q 1 %.4f\n", axis1encstpt); //Send position control command
						ODrivemessage[strlen(ODrivemessage)+1] = '\0';
						HAL_UART_Transmit_DMA(&huart1,(uint8_t *) ODrivemessage, strlen(ODrivemessage)); //Transmit Message over UART

						HAL_Delay(5); //Give time for ODrive to implement the tilt command
						break;
					}

					case 'c': //User is done zeroing
					{
						GlobalVars::UI_state = 1; //Go to the "main menu" state
						UIprint = 1; //Reset UIprint so the main menu prompt will print

						/*Code to run when zeroing is complete:
						 * 	1) Save the torque offsets associated with the zeroing (uses torque constants from experiment)
						 * 	2) Save the current encoder readings - these will be the zero offsets
						 * 	3) Save the current IMU angle readings - these will be the zero offsets
						 * 	4) Display the above information*/

						//Get current torque output:

						//Axis 0 Torque Measurement
						sprintf(ODrivemessage, "r axis0.motor.current_control.Iq_measured\n"); //request motor parameters from axis 0
						ODrivemessage[strlen(ODrivemessage)+1] = '\0';
						HAL_UART_Transmit(&huart1,(uint8_t *) ODrivemessage, strlen(ODrivemessage),10); //Transmit Message over UART

						HAL_UART_Receive(&huart1,(uint8_t *) ODriveReceive, 16, 10); //Receive message of current measurement

						GlobalVars::m0tor_offst = atof(ODriveReceive)*axis0torqueconstant; //Calculate torque offset based off of experimental torque constant

						HAL_Delay(10);

						//Encoder Axis 0 position
						sprintf(ODrivemessage, "f 0\n"); //request motor parameters from axis 0
						ODrivemessage[strlen(ODrivemessage)+1] = '\0';
						HAL_UART_Transmit_DMA(&huart1,(uint8_t *) ODrivemessage, strlen(ODrivemessage)); //Transmit Message over UART

						HAL_UART_Receive(&huart1,(uint8_t *) ODriveReceive, 16, 50);

						ptr = strtok(ODriveReceive, delim); //Initial cut, should isolate for first

						axis0encpos = atof(ptr); //Double check this is correct due to motor axis change

						HAL_Delay(5);

						//Axis 1 Torque Measurement
						sprintf(ODrivemessage, "r axis1.motor.current_control.Iq_measured\n"); //request motor parameters from axis 1
						ODrivemessage[strlen(ODrivemessage)+1] = '\0';
						HAL_UART_Transmit(&huart1,(uint8_t *) ODrivemessage, strlen(ODrivemessage),10); //Transmit Message over UART

						HAL_UART_Receive(&huart1,(uint8_t *) ODriveReceive, 16, 10); //Receive message of current measurement

						GlobalVars::m1tor_offst = atof(ODriveReceive)*axis1torqueconstant; //Calculate torque based off of experimental torque constant

						//Encoder Axis 1 position
						sprintf(ODrivemessage, "f 1\n"); //request motor parameters from axis 1
						ODrivemessage[strlen(ODrivemessage)+1] = '\0';
						HAL_UART_Transmit_DMA(&huart1,(uint8_t *) ODrivemessage, strlen(ODrivemessage)); //Transmit Message over UART

						HAL_UART_Receive(&huart1,(uint8_t *) ODriveReceive, 16, 10);

						ptr = strtok(ODriveReceive, delim); //Initial cut, should isolate for first

						axis1encpos = atof(ptr); //Double check this is correct due to motor axis change

						//Process final information for user before starting program.
						sprintf(str,"Platform is now ready to go operational.\r\n");
						str[strlen(str)+1] = '\0';
						CDC_Transmit_FS((uint8_t *) str, strlen(str)); //Transmit Message over UART

						HAL_Delay(5);


						//Read the IMU Euler angle offsets
						GlobalVars::IMUx_offset = imu.EulerX;
						GlobalVars::IMUy_offset = imu.EulerY;

						//Print the offset values to the serial port
						sprintf(str,"IMUx offset: %.2f\r\nIMUy offset: %.2f\r\nAxis0EncoderPos: %.4f\r\nAxis0TorqueOffset: %.4f\r\n",
								GlobalVars::IMUx_offset, GlobalVars::IMUy_offset, axis0encpos, GlobalVars::m0tor_offst);
						str[strlen(str)+1] = '\0';
						CDC_Transmit_FS((uint8_t *) str, strlen(str)); //Transmit Message over UART

						HAL_Delay(5);

						sprintf(str,"Axis1EncoderPos: %.4f\r\nAxis1TorqueOffset: %.4f\r\n",
								axis1encpos, GlobalVars::m1tor_offst);
						str[strlen(str)+1] = '\0';
						CDC_Transmit_FS((uint8_t *) str, strlen(str)); //Transmit Message over UART

						HAL_Delay(5);

						sprintf(str,"Please select the 'go operational' option in the main menu, close the serial port, and run the Simulink model.\r\n\n");
						str[strlen(str)+1] = '\0';
						CDC_Transmit_FS((uint8_t *) str, strlen(str)); //Transmit Message over UART

						HAL_Delay(5);

						// Store encoder position to global variable for reset state
						GlobalVars::enc_axis0zero = axis0encpos;
						GlobalVars::enc_axis1zero = axis1encpos;


						break;
					}

					default:
						sprintf(str,"Invalid input, please select option through valid digits only.\r\n\n");
						CDC_Transmit_FS((uint8_t *) str, strlen(str));
						break;

				}
			}
			break;
		}

		case 5: //Operational state
		{
			if (UIprint)
				{
					sprintf(str,"Platform is now operational. Shut off the serial port and run your Simulink model."
							" To leave the operational state and go"
							"back to the main menu, send 'r' over serial. \r\n\n");
					CDC_Transmit_FS((uint8_t *) str, strlen(str)); //Transmit Message over USB
					UIprint = 0;
				}

				/*If user input was received and a 'r' message was sent POTENTIAL SPOT FOR A BUG
			    WHAT IF SIMULINK DATA HAD 'r' IN THE FIRST SPOT IN THE BUFFER? MAY NEED TO CHANGE THIS
			    */
				if (usb_Rx_flag && (buffer[0] == 'r'))
				{

/*					sprintf(str,(const char*) &buffer[0]);
					CDC_Transmit_FS((uint8_t *) str, strlen(str)); //Transmit Message over USB

					HAL_Delay(5);*/

					usb_Rx_flag = 0; //clear the usb message received flag
					UIprint = 1; //Lets the main menu message print initially
					GlobalVars::UI_state = 0; //Go back to the "wait for any key pressed" state
					GlobalVars::Operational = 0;

				}

				//If a message was sent that was not 'r' in the first spot in the buffer
				else if(usb_Rx_flag)
				{
					usb_Rx_flag = 0; //reset the flag only
				}

				break;
		}

	}

} //End of UserInterface function

//======================END===========================//

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

	  //Try checking ErrorFlg if the system hard faults

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

