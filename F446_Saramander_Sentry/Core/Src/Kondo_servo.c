/*
 * Kondo_servo.c
 *
 *  Created on: Feb 25, 2020
 *      Author: okadatech
 */


#include "Kondo_servo.h"


const uint8_t ICS_POS_CMD= 0x80;
const uint8_t ICS_PARA_WRITE_COMMND= 0xC0;
const uint8_t SUB_ST_COMMND= 0x01;
const uint8_t SUB_SP_COMMND= 0x02;


int Synchronize(uint8_t *txBuff, size_t txLength, uint8_t *rxBuff, size_t rxLength)
{
  int size=0; //受信したbyte数
  HAL_GPIO_WritePin(GPIOC,GPIO_PIN_11,1);
  HAL_UART_Transmit(&huart5,(uint8_t*)txBuff, txLength,20);
  while(HAL_UART_GetState(&huart5)==HAL_UART_STATE_BUSY_TX){}
  HAL_GPIO_WritePin(GPIOC,GPIO_PIN_11,0);
  size= HAL_UART_Receive(&huart5,(uint8_t*)rxBuff, rxLength,20);
  while(HAL_UART_GetState(&huart5)==HAL_UART_STATE_BUSY_RX){}
  if (size != HAL_OK) //指定した受信数かどうか確認
  {
    return 0;
  }
  return 1;
}

int ics_set_pos(uint8_t id,unsigned short pos)
{
  uint8_t tx_data[3];
  uint8_t rx_data[3];
  int flag;
  int rAngle;   //受信した角度データ
  /// 送信コマンドを作成
  tx_data[0] = ICS_POS_CMD + id;
  tx_data[1] = (uint8_t)((pos & 0x3F80) >> 7);
  tx_data[2] = (uint8_t)(pos & 0x007F);

  //送受信を行う
  flag = Synchronize(tx_data,3,rx_data,3);

  if(flag == 0) //失敗した場合は-1を返す
  {
    return -1;
  }

  rAngle = ((rx_data[1] << 7) & 0x3F80) + (rx_data[2] & 0x007F);

  return rAngle;
}
int ics_set_st(uint8_t id,uint8_t stData)
{
  uint8_t tx_data[3];
  uint8_t rx_data[3];
  int flag;
  /// 送信コマンドを作成
  tx_data[0] = ICS_PARA_WRITE_COMMND + id;
  tx_data[1] = SUB_ST_COMMND;
  tx_data[2] = stData;

  //送受信を行う
  flag = Synchronize(tx_data,3,rx_data,3);

  if(flag == 0) //失敗した場合は-1を返す
   {
     return -1;
   }

  return flag;
}

int ics_set_sp(uint8_t id,uint8_t spData)
{
  uint8_t tx_data[3];
  uint8_t rx_data[3];
  int flag;
  /// 送信コマンドを作成
  tx_data[0] = ICS_PARA_WRITE_COMMND + id;
  tx_data[1] = SUB_SP_COMMND;
  tx_data[2] = spData;

  //送受信を行う
  flag = Synchronize(tx_data,3,rx_data,3);

  if(flag == 0) //失敗した場合は-1を返す
    {
      return -1;
    }

  return flag;
}