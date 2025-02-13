/*
 * motor.c
 *
 *  Created on: 2019/03/12
 *      Author: User
 */
#include "motor.h"

const int CaseMotor_MaxSpeed = 30 * 256;

void driveWheel(int16_t *u) {
	CAN_TxHeaderTypeDef header;
	uint8_t TxData[8] = { 0 };
	uint32_t TxMailbox;

	header.StdId = 0x200;
	header.RTR = CAN_RTR_DATA;
	header.IDE = CAN_ID_STD;
	header.DLC = 8;
	//header.TransmitGlobalTime = DISABLE;

	for (int i = 0; i < 2; i++) {
		if (CaseMotor_MaxSpeed < u[i]) {
			u[i] = CaseMotor_MaxSpeed - 1;
		} else if (u[i] < -CaseMotor_MaxSpeed) {
			u[i] = -CaseMotor_MaxSpeed + 1;
		}
		TxData[i * 2] = u[i] >> 8;
		TxData[i * 2 + 1] = u[i] & 0xFF;
	}

	HAL_CAN_AddTxMessage(&hcan2, &header, TxData, &TxMailbox);

}

void driveGimbalMotors(int16_t *u) {
	CAN_TxHeaderTypeDef header;
	uint8_t TxData[8] = { 0 };
	uint32_t TxMailbox;

	header.StdId = 0x1ff;
	header.RTR = CAN_RTR_DATA;
	header.IDE = CAN_ID_STD;
	header.DLC = 8;
	//header.TransmitGlobalTime = DISABLE;

	for (int i = 0; i < 4; i++) {
		if(i==2){
		if (CaseMotor_MaxSpeed < u[i]) {
			u[i] = CaseMotor_MaxSpeed - 1;
		} else if (u[i] < -CaseMotor_MaxSpeed) {
			u[i] = -CaseMotor_MaxSpeed + 1;
		}
		}
		TxData[i * 2] = u[i] >> 8;
		TxData[i * 2 + 1] = u[i] & 0xFF;
	}

	HAL_CAN_AddTxMessage(&hcan1, &header, TxData, &TxMailbox);
}




