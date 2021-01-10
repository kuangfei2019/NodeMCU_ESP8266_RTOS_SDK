#ifndef __DHT11_H
#define __DHT11_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_log.h"
#include "esp_system.h"
#include "esp_err.h"
#include "driver/gpio.h"

extern uint8_t DHT11_Data_Array[6];	

extern uint8_t DHT11_Data_Char[2][10];


void Dht11_delay_ms(uint32_t C_time);			// ºÁÃëÑÓÊ±º¯Êý

void DHT11_Signal_Output(uint8_t Value_Vol);	// DHT11ÐÅºÅÏß(IO5)Êä³ö²ÎÊýµçÆ½

void DHT11_Signal_Input(void);			// DHT11ÐÅºÅÏß(IO5) ÉèÎªÊäÈë

uint8_t DHT11_Start_Signal_JX(void);			// DHT11£ºÊä³öÆðÊ¼ÐÅºÅ£­£¾½ÓÊÕÏìÓ¦ÐÅºÅ

uint8_t DHT11_Read_Bit(void);					// ¶ÁÈ¡DHT11Ò»Î»Êý¾Ý

uint8_t DHT11_Read_Byte(void);				// ¶ÁÈ¡DHT11Ò»¸ö×Ö½Ú

uint8_t DHT11_Read_Data_Complete(void);		// ÍêÕûµÄ¶ÁÈ¡DHT11Êý¾Ý²Ù×÷

void DHT11_NUM_Char(void);				// DHT11Êý¾ÝÖµ×ª³É×Ö·û´®

#endif /* __DHT11_H */
