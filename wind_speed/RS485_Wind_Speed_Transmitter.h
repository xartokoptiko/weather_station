#ifndef __RS485_Wind_Speed_Transmitter_H__
#define __RS485_Wind_Speed_Transmitter_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/time.h>

/**
  @brief Initialize sensor serial port
  @param dev Seriial
  @return 1: Success, 0: Failure.
*/
char InitSensor(char *dev);

/**
  @brief delay function
  @param data Delay time in milliseconds
*/
void delayms(int data);

/**
  @brief  Modify address
  @param  Address1 For the current address, you can set any address with 0 address
  @param  Address2 The modified address.
  @return  A return value of 1 indicates success, and a return value of 0 indicates failure
*/
unsigned char ModifyAddress(unsigned char Address1, unsigned char Address2);

/**
  @brief  Add CRC parity bit
  @param  buf Data to which parity bits are to be added
  @param  len Check the length of the data.
*/
void addedCRC(unsigned char *buf, int len);

/**
  @brief  Read wind speed
  @param  Address The address where you want to read the data
  @return  The return value ≥0 indicates successful reading, the return value is wind speed, and the return value -1 indicates failed reading
*/
float readWindSpeed(unsigned char Address);

/**
  @brief  Calculate parity bit
  @param  buf Data to be verified
  @param  len Data length
  @return  The return value is the calculated parity bit
*/
unsigned int CRC16_2(unsigned char *buf, int len);

#endif