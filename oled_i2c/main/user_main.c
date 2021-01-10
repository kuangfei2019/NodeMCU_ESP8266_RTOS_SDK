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
#include "oled.h"


static const char *TAG = "main";
#define		ProjectName			"IIC_OLED"
#define I2C_EXAMPLE_MASTER_NUM              I2C_NUM_0

static void i2c_task_example(void *arg)
{
    ESP_LOGE(TAG, "\r\n=================================================\r\n");
    ESP_LOGE(TAG, "\t Project:\t%s\r\n", ProjectName);
    ESP_LOGE(TAG, "\t SDK version:\t%s", esp_get_idf_version());
    ESP_LOGE(TAG, "\r\n=================================================\r\n");

    oled_init();
    oled_showstring(0,0, (uint8_t *)"Project=");		// 从(0,0)开始显示
    oled_showstring(64,0, (uint8_t *)ProjectName);
    while (1) {

        vTaskDelay(1000 / portTICK_RATE_MS);
    }

    i2c_driver_delete(I2C_EXAMPLE_MASTER_NUM);
}

void app_main(void)
{
    //start i2c task
    xTaskCreate(i2c_task_example, "i2c_task_example", 2048, NULL, 10, NULL);
}
