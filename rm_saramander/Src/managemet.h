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
float DBUFF[32];


#endif /* MANAGEMET_H_ */
