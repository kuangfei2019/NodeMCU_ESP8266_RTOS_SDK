#ifndef OLED_H_
#define OLED_H_

// 宏定义
//=============================================================================
#define		OLED_CMD  	0		// 命令
#define		OLED_DATA 	1		// 数据

#define 	SIZE 		16		//显示字符的大小
#define 	Max_Column	128		//最大列数
#define		Max_Row		64		//最大行数
#define		X_WIDTH 	128		//X轴的宽度
#define		Y_WIDTH 	64	    //Y轴的宽度
#define		IIC_ACK		0		//应答
#define		IIC_NO_ACK	1		//不应答
//=============================================================================

// 函数声明
//=============================================================================

uint8_t oled_write_cmd(uint8_t OLED_Byte);

uint8_t oled_write_data(uint8_t OLED_Byte);

void oled_wr_byte(uint8_t OLED_Byte, uint8_t OLED_Type);

void  oled_clear(void);

void  oled_set_pos(uint8_t x, uint8_t y);

void  oled_init(void);

void  oled_showchar(uint8_t x, uint8_t y, uint8_t Show_char);

void  oled_showstring(uint8_t x, uint8_t y, uint8_t * Show_char);

//=============================================================================

#endif /* OLED_H_ */
