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
#include "oledfont.h"


/**
 * TEST CODE BRIEF
 *
 * This example will show you how to use I2C module by running two tasks on i2c bus:
 *
 * - write external i2c slave, here we use an i2c oled for instance.
 * - Use one I2C port(master mode) to read or write the other I2C port(slave mode) on one ESP8266 chip.
 *
 * Pin assignment:
 *
 * - master:
 *    GPIO14 is assigned as the data signal of i2c master port
 *    GPIO2 is assigned as the clock signal of i2c master port
 *
 * Connection:
 *
 * - connect sda/scl of sensor with GPIO14/GPIO2
 * - no need to add external pull-up resistors, driver will enable internal pull-up resistors.
 *
 * Test items:
 *
 * - write data to oled, if connected.
 */

#define I2C_EXAMPLE_MASTER_SCL_IO           14                /*!< gpio number for I2C master clock */
#define I2C_EXAMPLE_MASTER_SDA_IO           2               /*!< gpio number for I2C master data  */
#define I2C_EXAMPLE_MASTER_NUM              I2C_NUM_0        /*!< I2C port number for master dev */
#define I2C_EXAMPLE_MASTER_TX_BUF_DISABLE   0                /*!< I2C master do not need buffer */
#define I2C_EXAMPLE_MASTER_RX_BUF_DISABLE   0                /*!< I2C master do not need buffer */

#define OLED_ADDR           0x78             /*!< slave address for oled */
#define WRITE_DATA          0x40             /*!< write data to oled */
#define WRITE_CMD           0x00             /*!< write cmd to cmd */
#define WRITE_BIT           I2C_MASTER_WRITE /*!< I2C master write */
#define READ_BIT            I2C_MASTER_READ  /*!< I2C master read */
#define ACK_CHECK_EN        0x1              /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS       0x0              /*!< I2C master will not check ack from slave */
#define ACK_VAL             0x0              /*!< I2C ack value */
#define NACK_VAL            0x1              /*!< I2C nack value */
#define LAST_NACK_VAL       0x2              /*!< I2C last_nack value */

/**
 * @brief i2c master initialization
 */
static esp_err_t i2c_example_master_init(void)
{
    int i2c_master_port = I2C_EXAMPLE_MASTER_NUM;
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_EXAMPLE_MASTER_SDA_IO;
    conf.sda_pullup_en = 1;
    conf.scl_io_num = I2C_EXAMPLE_MASTER_SCL_IO;
    conf.scl_pullup_en = 1;
    conf.clk_stretch_tick = 300; // 300 ticks, Clock stretch is about 210us, you can make changes according to the actual situation.
    ESP_ERROR_CHECK(i2c_driver_install(i2c_master_port, conf.mode));
    ESP_ERROR_CHECK(i2c_param_config(i2c_master_port, &conf));
    return ESP_OK;
}

/**
 * @brief test code to write cmd to oled 
 *
 * 1. send data
 * ___________________________________________________________________________________________________
 * | start | slave_addr + wr_bit + ack | write reg_address + ack | write data_len byte + ack  | stop |
 * --------|---------------------------|-------------------------|----------------------------|------|
 *
 * @param i2c_num I2C port number
 * @param data data to send
 *
 * @return
 *     - ESP_OK Success
 *     - ESP_ERR_INVALID_ARG Parameter error
 *     - ESP_FAIL Sending command error, slave doesn't ACK the transfer.
 *     - ESP_ERR_INVALID_STATE I2C driver not installed or not in master mode.
 *     - ESP_ERR_TIMEOUT Operation timeout because the bus is busy.
 */
static esp_err_t i2c_write_cmd(i2c_port_t i2c_num, uint8_t OLED_Byte)
{
    int ret;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    i2c_master_start(cmd);		// 发送起始信号
    i2c_master_write_byte(cmd, OLED_ADDR, ACK_CHECK_EN);
    // [0x00]表示下一字节写入的是[指令]
    i2c_master_write_byte(cmd, WRITE_CMD, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, OLED_Byte, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    return ret;
}

static esp_err_t i2c_write_data(i2c_port_t i2c_num, uint8_t OLED_Byte)
{
    int ret;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    i2c_master_start(cmd);		// 发送起始信号
    i2c_master_write_byte(cmd, OLED_ADDR, ACK_CHECK_EN);
    // [0x40]表示下一字节写入的是[数据]
    i2c_master_write_byte(cmd, WRITE_DATA, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, OLED_Byte, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    return ret;
}

void i2c_wr_byte(uint8_t OLED_Byte, uint8_t OLED_Type)
{
	if (OLED_Type)
		i2c_write_data(I2C_EXAMPLE_MASTER_NUM, OLED_Byte); // 写入数据
	else
		i2c_write_cmd(I2C_EXAMPLE_MASTER_NUM, OLED_Byte); // 写入指令
}

void oled_clear(void)
{
	uint8_t N_Page, N_row;

	for (N_Page = 0; N_Page < 8; N_Page++) {
		i2c_wr_byte(0xb0 + N_Page, OLED_CMD);	// 从0～7页依次写入
		i2c_wr_byte(0x00, OLED_CMD);      	// 列低地址
		i2c_wr_byte(0x10, OLED_CMD);      	// 列高地址

		for(N_row=0; N_row<128; N_row++)
			i2c_wr_byte(0x00, OLED_DATA);
	}
}

void oled_set_pos(uint8_t x, uint8_t y)
{
	i2c_wr_byte(0xb0 + y, OLED_CMD); // 写入页地址
	i2c_wr_byte((x & 0x0f), OLED_CMD);	// 写入列的地址(低半字节)
	i2c_wr_byte(((x & 0xf0) >> 4) | 0x10, OLED_CMD); // 写入列的地址(高半字节)
}

void oled_init(void)
{
	i2c_example_master_init();	// 初始化IIC

	vTaskDelay(100 / portTICK_RATE_MS);	// 延迟(必须要有)

	i2c_wr_byte(0xAE,OLED_CMD);	// 关闭显示

	i2c_wr_byte(0x00,OLED_CMD);	// 设置低列地址
	i2c_wr_byte(0x10,OLED_CMD);	// 设置高列地址
	i2c_wr_byte(0x40,OLED_CMD);	// 设置起始行地址
	i2c_wr_byte(0xB0,OLED_CMD);	// 设置页地址

	i2c_wr_byte(0x81,OLED_CMD); 	// 对比度设置，可设置亮度
	i2c_wr_byte(0xFF,OLED_CMD);	// 265

	i2c_wr_byte(0xA1,OLED_CMD);	// 设置段(SEG)的起始映射地址
	i2c_wr_byte(0xA6,OLED_CMD);	// 正常显示；0xa7逆显示

	i2c_wr_byte(0xA8,OLED_CMD);	// 设置驱动路数（16~64）
	i2c_wr_byte(0x3F,OLED_CMD);	// 64duty

	i2c_wr_byte(0xC8,OLED_CMD);	// 重映射模式，COM[N-1]~COM0扫描

	i2c_wr_byte(0xD3,OLED_CMD);	// 设置显示偏移
	i2c_wr_byte(0x00,OLED_CMD);	// 无偏移

	i2c_wr_byte(0xD5,OLED_CMD);	// 设置震荡器分频
	i2c_wr_byte(0x80,OLED_CMD);	// 使用默认值

	i2c_wr_byte(0xD9,OLED_CMD);	// 设置 Pre-Charge Period
	i2c_wr_byte(0xF1,OLED_CMD);	// 使用官方推荐值

	i2c_wr_byte(0xDA,OLED_CMD);	// 设置 com pin configuartion
	i2c_wr_byte(0x12,OLED_CMD);	// 使用默认值

	i2c_wr_byte(0xDB,OLED_CMD);	// 设置 Vcomh，可调节亮度（默认）
	i2c_wr_byte(0x40,OLED_CMD);	// 使用官方推荐值

	i2c_wr_byte(0x8D,OLED_CMD);	// 设置OLED电荷泵
	i2c_wr_byte(0x14,OLED_CMD);	// 开显示

	i2c_wr_byte(0xAF,OLED_CMD);	// 开启OLED面板显示

	oled_clear();        			// 清屏

	oled_set_pos(0,0); 				// 设置数据写入的起始行、列
}

void oled_showchar(uint8_t x, uint8_t y, uint8_t Show_char)
{
	uint8_t c=0,i=0;

	c = Show_char - ' '; 				// 获取字符的偏移量

	if (x > Max_Column - 1) {
		x = 0; 
		y = y + 2;
	}	// 当列数超出范围，则另起2页

	if(SIZE == 16) 					// 字符大小为[8*16]：一个字符分两页
	{
		// 画第一页
		//-------------------------------------------------------
		oled_set_pos(x,y);						// 设置画点起始处
		for(i=0;i<8;i++)  						// 循环8次(8列)
		i2c_wr_byte(F8X16[c*16+i],OLED_DATA); 	// 找到字模

		// 画第二页
		//-------------------------------------------------------
		oled_set_pos(x,y+1); 					// 页数加1
		for(i=0;i<8;i++)  						// 循环8次
		i2c_wr_byte(F8X16[c*16+i+8],OLED_DATA);// 把第二页画完
	}
}

void oled_showstring(uint8_t x, uint8_t y, uint8_t *Show_char)
{
	uint8_t N_Char = 0;		// 字符序号

	while (Show_char[N_Char] != '\0') 	// 如果不是最后一个字符
	{
		oled_showchar(x,y,Show_char[N_Char]); 	// 显示一个字符

		x += 8;					// 列数加8，一个字符占8列

		if(x>=128){x=0;y+=2;} 	// 当x>=128，另起一页

		N_Char++; 				// 指向下一个字符
	}
}

void oled_showip(uint8_t x, uint8_t y, uint8_t *Array_IP)
{
	uint8_t N_IP_Byte = 0;		// IP字节序号

	// 循环显示4个IP字节(由高到低字节显示)
	//----------------------------------------------------------
	for(; N_IP_Byte < 4; N_IP_Byte++)
	{
		// 显示百位/十位
		//------------------------------------------------------
		if (Array_IP[N_IP_Byte] / 100)		// 判断百位?=0
		{
			oled_showchar(x, y, 48 + Array_IP[N_IP_Byte] / 100); 
			x += 8;
			// 显示十位（百位!=0）
			//---------------------------------------------------------
			//if(Array_IP[N_IP_Byte]%100/10)
			{ oled_showchar(x,y,48+Array_IP[N_IP_Byte]%100/10); x+=8; }

		}

		// 显示十位（百位==0）
		//---------------------------------------------------------
		else if(Array_IP[N_IP_Byte]%100/10)		// 判断十位?=0
		{ oled_showchar(x,y,48+Array_IP[N_IP_Byte]%100/10); x+=8; }


		// 显示个位
		//---------------------------------------------------------
		//if(Array_IP[C_IP_Byte]%100%10)
		{ oled_showchar(x,y,48+Array_IP[N_IP_Byte]%100%10); x+=8; }

		// 显示【.】
		if (N_IP_Byte < 3) { 
			oled_showchar(x, y, '.'); 
			x += 8; 
		}
	}
}

