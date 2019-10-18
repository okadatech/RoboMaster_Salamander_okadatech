/*
 * managemet.h
 *
 *  Created on: 2019/09/23
 *      Author: okadatech
 */

#ifndef MANAGEMET_H_
#define MANAGEMET_H_
#include "main.h"
#include "stm32f4xx_hal.h"
#include "can.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"


#include "math.h"
#include "remote_controller.h"
#include "wheel.h"
#include "pid.h"
#include "motor.h"
#include "mecanum.h"
//#include "disp.h"
//#include "xprintf.h"
#include "motor_fdb.h"
#include "bsp_imu.h"

#include "stdio.h"
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

__uint8_t rcData[18];
struct rc_info_t rc;
wheel_fdb_t wheelFdb[4];
uint8_t canRxData[8];
CAN_RxHeaderTypeDef canRxHeader;
motor_fdb gimbalYawFdb, gimbalPitchFdb, loadMotorFdb;
struct mecanum mecanum;
_pid_t wheelPID[4], loadPID;
int16_t target_yaw,target_pich;
int16_t yaw_now,pich_now;

TIM_MasterConfigTypeDef sMasterConfig;
TIM_OC_InitTypeDef sConfigOC;
TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig;

float DBUFF[32];


float IMU_pich;
float IMU_yaw;
float IMU_rol;

float IMU_pich_set;
float IMU_yaw_set;
float IMU_rol_set;


int PC_mouse_x,PC_mouse_y;



#endif /* MANAGEMET_H_ */
