/*
 * referee_serial_port_protocol.h
 *
 *  Created on: 2019/11/13
 *      Author: okadatech
 */

#ifndef REFEREE_SERIAL_PORT_PROTOCOL_H_
#define REFEREE_SERIAL_PORT_PROTOCOL_H_

#include "stm32f4xx_hal.h"
#include <stdbool.h>
#include <string.h>
#include "managemet.h"



uint8_t Get_CRC8_Check_Sum(uint8_t *pchMessage,unsigned int dwLength,uint8_t ucCRC8);
unsigned int Verify_CRC8_Check_Sum(uint8_t *pchMessage, unsigned int dwLength);
void Append_CRC8_Check_Sum(uint8_t *pchMessage, unsigned int dwLength);
uint16_t Get_CRC16_Check_Sum(uint8_t *pchMessage,uint32_t dwLength,uint16_t wCRC);
uint32_t Verify_CRC16_Check_Sum(uint8_t *pchMessage, uint32_t dwLength);
void Append_CRC16_Check_Sum(uint8_t * pchMessage,uint32_t dwLength);


void makeCustomDataPacket(uint8_t packet[28]);
void setFrameHeader(uint8_t header[7]);
void setData1(float value);
void setData2(float value);
void setData3(float value);
void setMasks(uint8_t mask);
void setClientID(uint16_t ID);
void setSenderID(uint16_t ID);
void CustomData_init(uint16_t sender,uint16_t clientID);

#endif /* REFEREE_SERIAL_PORT_PROTOCOL_H_ */
