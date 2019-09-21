
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2019 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f4xx_hal.h"
#include "can.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* USER CODE BEGIN Includes */
#include "remote_controller.h"
#include "wheel.h"
#include "pid.h"
#include "motor.h"
#include "mecanum.h"
#include "disp.h"
#include "motor_fdb.h"
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
__uint8_t rcData[18];
struct rc_info_t rc;
wheel_fdb_t wheelFdb[4];
uint8_t canRxData[8];
CAN_RxHeaderTypeDef canRxHeader;
motor_fdb gimbalYawFdb, gimbalPitchFdb, loadMotorFdb;
struct mecanum mecanum;
_pid_t wheelPID[4], loadPID;
float DBUFF[32];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/
void timerTask();
void initPID();
void initMecanum();
void driveWheelTask();
void initFriction();
void initLoadPID();
/* USER CODE END PFP */

/* USER CODE BEGIN 0 */
void timerTask() { //call 500Hz
	int fire = 0;
	int16_t u[3];
	driveWheelTask();

	if (rc.sw2 == 1) {
		fire = 1;
	} else {
		fire = 0;
	}
	DBUFF[1] = loadPID.error = -900.0f*fire - loadMotorFdb.rpm;
	u[0] = 0;
	u[1] = 0;
	DBUFF[3] = u[2] = pidExecute(&loadPID);
	driveGimbalMotors(u);
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  *
  * @retval None
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

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
  MX_SPI5_Init();
  MX_CAN2_Init();
  MX_CAN1_Init();
  MX_TIM12_Init();
  MX_TIM1_Init();
  MX_USART1_UART_Init();
  MX_UART7_Init();
  MX_UART8_Init();
  MX_USART6_UART_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1); // friction wheel
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);
  initFriction();
  initPID();
  initLoadPID();
  initCanFilter();
  initMecanum();
  HAL_UART_Receive_IT(&huart1, rcData, 18);
  HAL_TIM_Base_Start_IT(&htim6);
  setDispUartHandler(&huart7);
  HAL_CAN_Start(&hcan1);
  HAL_CAN_Start(&hcan2);
  HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);
  HAL_CAN_ActivateNotification(&hcan2, CAN_IT_RX_FIFO0_MSG_PENDING);
  HAL_GPIO_WritePin(POWER_OUT1_GPIO_Port, POWER_OUT1_Pin, 1);
  HAL_GPIO_WritePin(POWER_OUT2_GPIO_Port, POWER_OUT2_Pin, 1);
  HAL_GPIO_WritePin(POWER_OUT3_GPIO_Port, POWER_OUT3_Pin, 1);
  HAL_GPIO_WritePin(POWER_OUT4_GPIO_Port, POWER_OUT4_Pin, 1);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */
	  DBUFF[0] = rc.sw2;
	  //DBUFF[1] = gimbalPitchFdb.rpm;
	  DBUFF[2] = loadMotorFdb.rpm;

	  for (int i = 0; i < 4; i++) {
		  advancedDisp(DBUFF[i], 5);
		  string(",");
	  }
	  dnl();

  }
  /* USER CODE END 3 */

}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

    /**Configure the main internal regulator output voltage 
    */
  __HAL_RCC_PWR_CLK_ENABLE();

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 6;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* USER CODE BEGIN 4 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {

	static int cnt = 0;
	static long unsigned int c = 0;
	c++;
	if (htim->Instance == htim6.Instance) {//500Hz
		timerTask();
		cnt++;
		if (cnt >= 500) {//1Hz
			HAL_GPIO_TogglePin(LED_G_GPIO_Port, LED_G_Pin);
			cnt = 0;
		}
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle) {
	if (UartHandle->Instance == huart1.Instance) {	//Propo-receive Interrupts
		HAL_UART_Receive_IT(&huart1, rcData, 18);
		//HAL_GPIO_TogglePin(LED_R_GPIO_Port, LED_R_Pin);
		rc.ch1 = (((rcData[1] & 0x07) << 8) | rcData[0]);
		rc.ch1 -= 1024;
		rc.ch2 = (((rcData[2] & 0x3F) << 5) | (rcData[1] >> 3));
		rc.ch2 -= 1024;
		rc.ch3 = (((rcData[4] & 0x01) << 10) | (rcData[3] << 2)
				| (rcData[2] >> 6));
		rc.ch3 -= 1024;
		rc.ch4 = (((rcData[5] & 0x0F) << 7) | (rcData[4] >> 1));
		rc.ch4 -= 1024;
		rc.sw1 = ((rcData[5] & 0x30) >> 4);
		rc.sw2 = ((rcData[5] & 0xC0) >> 6);
	}
}

//can fifo0 receive interrupt
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
	/* Get RX message */
	if (hcan->Instance == hcan2.Instance) {// can2 bus receive interrupt
		HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &canRxHeader, canRxData);
		//check ESC identifier
		int id = canRxHeader.StdId - 513;

		wheelFdb[id].angle = canRxData[0] * 256 + canRxData[1];
		wheelFdb[id].rpm = canRxData[2] * 256 + canRxData[3];
		wheelFdb[id].torque = canRxData[4] * 256 + canRxData[5];
		wheelFdb[id].temp = canRxData[6];
		wheelFdb[id].omg = 6.28318530718 * (wheelFdb[id].rpm) / 1140.0;
	}
	if (hcan->Instance == hcan1.Instance) {// can1 bus receive interrupt
		HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &canRxHeader, canRxData);
		motor_fdb *fdb = 0;
		switch (canRxHeader.StdId) {
		case 0x205:
			fdb = &gimbalYawFdb;
			break;
		case 0x206:
			fdb = &gimbalPitchFdb;
			break;
		case 0x207:
			fdb = &loadMotorFdb;
			break;
		}
		fdb->angle = canRxData[0] * 256 + canRxData[1];
		fdb->rpm = canRxData[2] * 256 + canRxData[3];
		fdb->torque = canRxData[4] * 256 + canRxData[5];
		fdb->temp = canRxData[6];
		fdb->omg = 6.28318530718 * (fdb->rpm) / 1140.0;
	}
}

void driveWheelTask() {

	mecanum.speed.vx = (float) rc.ch4 / 660 * MAX_CHASSIS_VX_SPEED;
	mecanum.speed.vy = -(float) rc.ch3 / 660 * MAX_CHASSIS_VX_SPEED;
	mecanum.speed.vw = -(float) rc.ch1 / 660 * MAX_CHASSIS_VW_SPEED;

	mecanum_calculate(&mecanum);

/*	DBUFF[0] = mecanum.wheel_rpm[0];
	DBUFF[1] = mecanum.wheel_rpm[1];
	DBUFF[2] = mecanum.wheel_rpm[2];
	DBUFF[3] = mecanum.wheel_rpm[3];*/

	int16_t u[4];
	for (int i = 0; i < 4; i++) {
		int error = mecanum.wheel_rpm[i] - wheelFdb[i].rpm;
		wheelPID[i].error = error;
		u[i] = (int16_t) pidExecute(&(wheelPID[i]));
	}
	driveWheel(u);
}

void initPID() {
	for (int i = 0; i < 4; i++) {
		wheelPID[i].t = 2.0f;
		wheelPID[i].p = 6.5f;
		wheelPID[i].i = 50.0f;
		wheelPID[i].d = 0.0f;
		wheelPID[i].outLimit = 15000.0f;
		wheelPID[i].integralOutLimit = 500.0f;
		wheelPID[i].differentialFilterRate = 0.9f;
	}
}

void initMecanum() {
	mecanum.param.wheel_perimeter = PERIMETER;
	mecanum.param.wheeltrack = WHEELTRACK;
	mecanum.param.wheelbase = WHEELBASE;
	mecanum.param.rotate_x_offset = 0;
	mecanum.param.rotate_y_offset = 0;
}

void initFriction() {
	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 1500);
	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, 1500);
	HAL_Delay(3000);
	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 1220);
	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, 1220);
	HAL_Delay(5000);
	for (int i = 1220; i < 1400; i++) {
		__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, i);
		__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, i);
		HAL_Delay(30);
	}
}

void initLoadPID() {
	loadPID.t = 2.0f;
	loadPID.p = 10.0f;
	loadPID.i = 0.1f*500;
	loadPID.d = 0.07f;
	loadPID.outLimit = 30000.0f;
	loadPID.integralOutLimit = 10000.0f;
	loadPID.differentialFilterRate = 0.9f;
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  file: The file name as string.
  * @param  line: The line in file as a number.
  * @retval None
  */
void _Error_Handler(char *file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1)
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
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/