#ifndef OLED_H_
#define OLED_H_

// �궨��
//=============================================================================
#define		OLED_CMD  	0		// ����
#define		OLED_DATA 	1		// ����

#define 	SIZE 		16		//��ʾ�ַ��Ĵ�С
#define 	Max_Column	128		//�������
#define		Max_Row		64		//�������
#define		X_WIDTH 	128		//X��Ŀ��
#define		Y_WIDTH 	64	    //Y��Ŀ��
#define		IIC_ACK		0		//Ӧ��
#define		IIC_NO_ACK	1		//��Ӧ��
//=============================================================================

// ��������
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
