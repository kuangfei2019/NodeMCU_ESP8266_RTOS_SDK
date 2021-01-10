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
#include "esp_timer.h"
#include "ds18b20.h"


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
	short temp;

	temp = DS18B20_Get_Temp();
        ESP_LOGE(TAG, "Temp:\t%d\r\n", temp);
	led_flag = !led_flag;
	gpio_set_level(LED_PIN_NUM, led_flag);		// LEDаа

	oled_showstring(0, 6, (uint8_t *)&temp);	// DHT11_Data_Char[1] == ║╬нбІхвжЇШЄ╝║©
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

    esp_timer_init(); // 使用定时器API函数，先调用接口初始化
    esp_err_t err = esp_timer_create(&test_periodic_arg, &test_p_handle);
    err = esp_timer_start_periodic(test_p_handle, 1000 * 1000);

    DS18B20_Init();

    while (1) {
        vTaskDelay(3000 / portTICK_RATE_MS);
    }

    esp_timer_stop(test_p_handle);
    i2c_driver_delete(I2C_EXAMPLE_MASTER_NUM);
}

void app_main(void)
{
    //start i2c task
    xTaskCreate(i2c_task_example, "i2c_task_example", 2048, NULL, 10, NULL);
}
