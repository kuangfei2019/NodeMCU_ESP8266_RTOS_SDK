/* I2C example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_log.h"
#include "esp_system.h"
#include "esp_err.h"

#include "driver/i2c.h"
#include "driver/hw_timer.h"
#include "oled.h"
#include "dht11.h"
#include "esp_timer.h"



static const char *TAG = "main";
#define		ProjectName			"IIC_OLED"
#define I2C_EXAMPLE_MASTER_NUM              I2C_NUM_0
#define TEST_ONE_SHOT    false        // testing will be done without auto reload (one-shot)
#define TEST_RELOAD      true
#define LED_PIN_NUM      4
#define GPIO_OUTPUT_PIN_SEL      (1UL << LED_PIN_NUM)
uint8_t led_flag = 0;
esp_timer_handle_t test_p_handle = 0;

void OS_Timer_1_cb(void)
{
	led_flag = !led_flag;
	gpio_set_level(LED_PIN_NUM, led_flag);		// LEDаа
	if(DHT11_Read_Data_Complete() == 0)		// Іах║DHT11нбй╙Іхж╣
	{
		//-------------------------------------------------
		// DHT11_Data_Array[0] == й╙Іх_уШйЩ_╡©Їж
		// DHT11_Data_Array[1] == й╙Іх_п║йЩ_╡©Їж
		// DHT11_Data_Array[2] == нбІх_уШйЩ_╡©Їж
		// DHT11_Data_Array[3] == нбІх_п║йЩ_╡©Їж
		// DHT11_Data_Array[4] == пёяИвжҐз
		// DHT11_Data_Array[5] == ║╬1:нбІх>=0║©║╬0:нбІх<0║©
		//-------------------------------------------------


		// нбІхЁ╛╧Щ30║Фё╛LEDаа
		//----------------------------------------------------
		if(DHT11_Data_Array[5]==1 && DHT11_Data_Array[2]>=13)
			gpio_set_level(LED_PIN_NUM,0);		// LEDаа
		else
			gpio_set_level(LED_PIN_NUM,1);		// LEDцП


		// Є╝©зйДЁЖнбй╙Іх
		//---------------------------------------------------------------------------------
		if(DHT11_Data_Array[5] == 1)			// нбІх >= 0║Ф
		{
                        ESP_LOGE(TAG, "Hum:\t%d.%dRH\r\n", DHT11_Data_Array[0],DHT11_Data_Array[1]);
                        ESP_LOGE(TAG, "Temp:\t %d.%d\r\n", DHT11_Data_Array[2],DHT11_Data_Array[3]);
		}
		else // if(DHT11_Data_Array[5] == 0)	// нбІх < 0║Ф
		{
                        ESP_LOGE(TAG, "Hum:\t%d.%dRH\r\n", DHT11_Data_Array[0],DHT11_Data_Array[1]);
                        ESP_LOGE(TAG, "Temp:\t %d.%d\r\n", DHT11_Data_Array[2],DHT11_Data_Array[3]);
		}

		// OLEDотй╬нбй╙Іх
		//---------------------------------------------------------------------------------
		DHT11_NUM_Char();	// DHT11йЩ╬щж╣в╙ЁивжЇШЄ╝

		oled_showstring(0, 2, DHT11_Data_Char[0]);	// DHT11_Data_Char[0] == ║╬й╙ІхвжЇШЄ╝║©
		oled_showstring(0, 6, DHT11_Data_Char[1]);	// DHT11_Data_Char[1] == ║╬нбІхвжЇШЄ╝║©
	}
}

esp_timer_create_args_t test_periodic_arg =
{
    .callback = OS_Timer_1_cb, // 设置回调函数
    .arg = NULL,                         // 不携带参数
    .name = "PeriodicTimer"          // 定时器名字
};


static void led_init(void)
{
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);
}

static void i2c_task_example(void *arg)
{
    ESP_LOGE(TAG, "\r\n=================================================\r\n");
    ESP_LOGE(TAG, "\t Project:\t%s\r\n", ProjectName);
    ESP_LOGE(TAG, "\t SDK version:\t%s", esp_get_idf_version());
    ESP_LOGE(TAG, "\r\n=================================================\r\n");

    led_init();
    oled_init();
    oled_showstring(0,0, (uint8_t *)"Hum");		// 从(0,0)开始显示
    oled_showstring(0,4, (uint8_t *)"Temp");
  //  ESP_LOGI(TAG, "Initialize hw_timer for callback1");
  //  hw_timer_init(OS_Timer_1_cb, NULL);
  //  ESP_LOGI(TAG, "Set hw_timer timing time 1s with reload");
  //  hw_timer_alarm_us(1000000, TEST_RELOAD);
    esp_timer_init(); // 使用定时器API函数，先调用接口初始化

    esp_err_t err = esp_timer_create(&test_periodic_arg, &test_p_handle);
    err = esp_timer_start_periodic(test_p_handle, 1000 * 1000);
    while (1) {
        vTaskDelay(3000 / portTICK_RATE_MS);
    }

  //  hw_timer_deinit();
    esp_timer_stop(test_p_handle);
    i2c_driver_delete(I2C_EXAMPLE_MASTER_NUM);
}

void app_main(void)
{
    //start i2c task
    xTaskCreate(i2c_task_example, "i2c_task_example", 2048, NULL, 10, NULL);
}
