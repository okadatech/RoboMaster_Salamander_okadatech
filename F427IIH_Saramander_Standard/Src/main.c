/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "can.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "managemet.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

const int yaw_MAX=70;
const int yaw_magnification=30;
const int pich_MAX=30;
const int pich_magnification=80;
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */
void __io_putchar(uint8_t ch) {
HAL_UART_Transmit(&huart7, &ch, 1, 1);
}

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
void Gimbal_Task();
void fire_Task();
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
long map(long x, long in_min, long in_max, long out_min, long out_max) {
	  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
void timerTask() ;
uint8_t cnt_tim,cnt_tim_fire;
uint16_t cnt_tim_omega;
uint16_t sw1_cnt=1220;
uint8_t rc_SW1_temp;
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
  MX_CAN1_Init();
  MX_CAN2_Init();
  MX_SPI5_Init();
  MX_TIM1_Init();
  MX_TIM6_Init();
  MX_TIM12_Init();
  MX_UART7_Init();
  MX_UART8_Init();
  MX_USART1_UART_Init();
  MX_USART6_UART_Init();
  /* USER CODE BEGIN 2 */
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11, 0);
  HAL_GPIO_WritePin(LED_G_GPIO_Port, LED_G_Pin, 1);
  HAL_GPIO_WritePin(GPIOG, GPIO_PIN_1, 1);
  HAL_GPIO_WritePin(GPIOG, GPIO_PIN_2, 1);
  HAL_GPIO_WritePin(GPIOG, GPIO_PIN_3, 1);
  HAL_GPIO_WritePin(GPIOG, GPIO_PIN_4, 1);
  HAL_GPIO_WritePin(GPIOG, GPIO_PIN_5, 1);
  HAL_GPIO_WritePin(GPIOG, GPIO_PIN_6, 1);
  HAL_GPIO_WritePin(GPIOG, GPIO_PIN_7, 1);
  HAL_GPIO_WritePin(GPIOG, GPIO_PIN_8, 1);

  mpu_device_init();
  mpu_offset_call();
  init_quaternion();


  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1); // friction wheel
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);
  initFriction();
  initPID();
  initLoadPID();
  initCanFilter();
  initMecanum();
  HAL_UART_Receive_IT(&huart1, rcData, 18);
  HAL_TIM_Base_Start_IT(&htim6);
  setbuf(stdout, NULL);
  HAL_CAN_Start(&hcan1);
  HAL_CAN_Start(&hcan2);
  HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);
  HAL_CAN_ActivateNotification(&hcan2, CAN_IT_RX_FIFO0_MSG_PENDING);
  HAL_GPIO_WritePin(POWER_OUT1_GPIO_Port, POWER_OUT1_Pin, 1);
  HAL_GPIO_WritePin(POWER_OUT2_GPIO_Port, POWER_OUT2_Pin, 1);
  HAL_GPIO_WritePin(POWER_OUT3_GPIO_Port, POWER_OUT3_Pin, 1);
  HAL_GPIO_WritePin(POWER_OUT4_GPIO_Port, POWER_OUT4_Pin, 1);
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11, 1);

  init_quaternion();

  IMU_pich_set=((imu.pit)*(180.0/M_PI));
  IMU_yaw_set=((imu.yaw)*(180.0/M_PI));
  IMU_rol_set=((imu.rol)*(180.0/M_PI));
  PC_mouse_x=0;
  PC_mouse_y=0;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2)==1){
		  IMU_pich_set=((imu.pit)*(180.0/M_PI));
		  IMU_yaw_set=((imu.yaw)*(180.0/M_PI));
		  IMU_rol_set=((imu.rol)*(180.0/M_PI));
	  }

	   printf(" Roll:%8.3lf  Pitch:%8.3lf  Yaw:%8.3lf", IMU_rol, IMU_pich, IMU_yaw);
	   //printf(" Roll_set:%8.3lf  Pitch_set:%8.3lf  Yaw_set:%8.3lf", IMU_rol_set, IMU_pich_set, IMU_yaw_set);
	   printf(" target_Pitch:%d target_Yaw:%d", target_pich,target_yaw);
	  //printf("ch1=%d ch2=%d ch3=%d ch4=%d ch5=%d sw1=%d sw2=%d m_x=%d m_y=%d m_z=%d m_l=%d m_r=%d W=%d S=%d A=%d D=%d Q=%d E=%d Shift=%d Ctrl=%d"
	  //	 ,rc.ch1,rc.ch2,rc.ch3,rc.ch4,rc.ch5,rc.sw1,rc.sw2,rc.mouse_x, rc.mouse_y, rc.mouse_z,rc.mouse_press_l,rc.mouse_press_r
	//		 ,rc.key_W,rc.key_S,rc.key_A,rc.key_D,rc.key_Q,rc.key_E,rc.key_Shift,rc.key_Ctrl);
	  //printf("PC_mouse_x=%d PC_mouse_y=%d",PC_mouse_x,PC_mouse_y);
	  //printf("M0=%d M1=%d M2=%d M3=%d",wheelFdb[0].rpm,wheelFdb[1].rpm,wheelFdb[2].rpm,wheelFdb[3].rpm);
	  printf(" ch5=%d vw=%f cnt=%d",rc.ch5,mecanum.speed.vw,cnt_tim_omega);
	  //printf(" target_yaw=%d angle=%f",target_yaw,(float)((gimbalYawFdb.angle-4096.0)/8191.0*360.0));
	  printf("\r\n");


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
  /** Initializes the CPU, AHB and APB busses clocks 
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
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	static long unsigned int c = 0;
	c++;
	if (htim->Instance == htim6.Instance) {//500Hz
		timerTask();
		if(cnt_tim>20){
		HAL_GPIO_TogglePin(LED_G_GPIO_Port, LED_G_Pin);
		cnt_tim=0;
		}
		cnt_tim++;
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle) {
	if (UartHandle->Instance == huart1.Instance) {	//Propo-receive Interrupts
		HAL_UART_Receive_IT(&huart1, rcData, 18);
		HAL_GPIO_TogglePin(GPIOG,GPIO_PIN_1);
		rc.ch1 = (((rcData[1] & 0x07) << 8) | rcData[0]);
		rc.ch1 -= 1024;
		rc.ch2 = (((rcData[2] & 0x3F) << 5) | (rcData[1] >> 3));
		rc.ch2 -= 1024;
		rc.ch3 = (((rcData[4] & 0x01) << 10) | (rcData[3] << 2) | (rcData[2] >> 6));
		rc.ch3 -= 1024;
		rc.ch4 = (((rcData[5] & 0x0F) << 7) | (rcData[4] >> 1));
		rc.ch4 -= 1024;
		rc.ch5 = (rcData[17]<<8) | rcData[16];
		rc.ch5 = 1024-rc.ch5;
		rc.sw1 = ((rcData[5] & 0x30) >> 4);
		rc.sw2 = ((rcData[5] & 0xC0) >> 6);
		rc.mouse_x = ((int16_t)rcData[6]) | ((int16_t)rcData[7] << 8);
		rc.mouse_y = ((int16_t)rcData[8]) | ((int16_t)rcData[9] << 8);
		rc.mouse_z = ((int16_t)rcData[10]) | ((int16_t)rcData[11] << 8);
		rc.mouse_press_l = rcData[12];
		rc.mouse_press_r = rcData[13];
		rc.key_v = ((int16_t)rcData[14]);
		rc.key_W =     (0b0000000000000001 & rc.key_v);
		rc.key_S =     (0b0000000000000010 & rc.key_v)>>1;
		rc.key_A =     (0b0000000000000100 & rc.key_v)>>2;
		rc.key_D =     (0b0000000000001000 & rc.key_v)>>3;
		rc.key_Shift = (0b0000000000010000 & rc.key_v)>>4;
		rc.key_Ctrl =  (0b0000000000100000 & rc.key_v)>>5;
		rc.key_Q =     (0b0000000001000000 & rc.key_v)>>6;
		rc.key_E =     (0b0000000010000000 & rc.key_v)>>7;

		if(rc.sw2==2){
			PC_mouse_x=0;
			PC_mouse_y=0;
		}
		else{
			if(rc.mouse_press_l==1){
			PC_mouse_x=PC_mouse_x+rc.mouse_x;
			PC_mouse_y=PC_mouse_y+rc.mouse_y;
			if(PC_mouse_x > pich_MAX*pich_magnification){	PC_mouse_x = pich_MAX*pich_magnification;}
			if(PC_mouse_x < -1*pich_MAX*pich_magnification){PC_mouse_x = -1*pich_MAX*pich_magnification;}
			if(PC_mouse_y > yaw_MAX*yaw_magnification){		PC_mouse_y = pich_MAX*pich_magnification;}
			if(PC_mouse_y < -1*yaw_MAX*yaw_magnification){	PC_mouse_y = -1*yaw_MAX*yaw_magnification;}
			}
		}

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

	if(rc.sw1==1){
		if(cnt_tim_omega<200 || cnt_tim_omega>600){
			mecanum.speed.vw = -(float) (rc.ch5-100.0) / 660.0 * MAX_CHASSIS_VW_SPEED;
		}
		else{
			mecanum.speed.vw = -(float) (rc.ch5+100.0) / 660.0 * MAX_CHASSIS_VW_SPEED;
		}

		cnt_tim_omega++;
		if(cnt_tim_omega>800){cnt_tim_omega=0;}
	}
	else{
		cnt_tim_omega=0;
		mecanum.speed.vw = -(float) rc.ch5 / 660 * MAX_CHASSIS_VW_SPEED;
	}

	mecanum_calculate(&mecanum);

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
	for(int i=0;i<300;i++){
	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 1500);
	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, 1500);
	HAL_Delay(10);
	mpu_get_data();
	imu_ahrs_update();
	imu_attitude_update();
	}
	for(int i=0;i<500;i++){
	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 1220);
	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, 1220);
	HAL_Delay(10);
	mpu_get_data();
	imu_ahrs_update();
	imu_attitude_update();
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

void timerTask() { //call 500Hz
	driveWheelTask();
	Gimbal_Task();
	fire_Task();
	mpu_get_data();
	imu_ahrs_update();
	imu_attitude_update();

	IMU_pich=((imu.pit)*(180.0/M_PI))-IMU_pich_set;
		if(IMU_pich>  90.0){IMU_pich=IMU_pich-180;}
		if(IMU_pich< -90.0){IMU_pich=IMU_pich+180;}
	IMU_yaw=((imu.yaw)*(180.0/M_PI))-IMU_yaw_set;
		if(IMU_yaw>  180.0){IMU_yaw=IMU_yaw-360;}
		if(IMU_yaw< -180.0){IMU_yaw=IMU_yaw+360;}
	IMU_rol=((imu.rol)*(180.0/M_PI))-IMU_rol_set;
		if(IMU_rol>  180.0){IMU_rol=IMU_rol-360;}
		if(IMU_rol< -180.0){IMU_rol=IMU_rol+360;}
}

void Gimbal_Task(){
	int fire = 0;
	int16_t u[4];
	if (rc.mouse_press_r == 1) {
		fire = 1;
		DBUFF[1] = loadPID.error = -900.0f*fire*3 - loadMotorFdb.rpm;
		DBUFF[3] = u[2] = pidExecute(&loadPID);
	} else {
		fire = 0;
		if(rc.sw2==2){
			DBUFF[1] = loadPID.error = 900.0f*1 - loadMotorFdb.rpm;
			DBUFF[3] = u[2] = pidExecute(&loadPID);
		}
		else{
			DBUFF[1] = loadPID.error = -900.0f*fire*3 - loadMotorFdb.rpm;
			DBUFF[3] = u[2] = pidExecute(&loadPID);
		}
	}

	if(rc.sw1==2){target_yaw=0;}
	else{
		if(rc.sw1==1){
			if(rc_SW1_temp==3){IMU_yaw_set=((imu.yaw)*(180.0/M_PI));;}
		target_yaw =((float)PC_mouse_x / yaw_magnification)-IMU_yaw;
		if(target_yaw>70){target_yaw=70;}
		if(target_yaw<-70){target_yaw=-70;}
		}
		else{
			target_yaw=(float)PC_mouse_x / yaw_magnification;
			if(target_yaw>70){target_yaw=70;}
			if(target_yaw<-70){target_yaw=-70;}
		}
	}
	yaw_now=(float)((gimbalYawFdb.angle-4096.0)/8191.0*360.0);
	u[0]=map(target_yaw-yaw_now, -180, 180, -30000, 30000);

	if(rc.sw1==2){target_pich=0;}
	else{
		target_pich=((float)PC_mouse_y / pich_magnification)-IMU_pich;
		if(target_pich>20){target_pich=20;}
		if(target_pich<-30){target_pich=-30;}
	}
	pich_now=(float)((gimbalPitchFdb.angle-4096.0)/8191.0*360.0)+24;
	u[1]=map(target_pich-pich_now, -30, 20, -15000, 15000);


	u[3]=0;
	driveGimbalMotors(u);
	rc_SW1_temp=rc.sw1;
}

void fire_Task(){
	if(rc.sw2==1){
		__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, sw1_cnt);
		__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, sw1_cnt);
		if(sw1_cnt>=1400){
			sw1_cnt=1400;
		}
		else{sw1_cnt++;}
	}
	else{
		sw1_cnt=1220;
		__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, sw1_cnt);
		__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, sw1_cnt);
	}
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
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
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/