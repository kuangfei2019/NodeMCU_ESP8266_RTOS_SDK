#include "dht11.h"		// DHT11

#define GPIO_MUX_IO_0     5
#define GPIO_MUX_PIN_SEL  (1UL << GPIO_MUX_IO_0)

uint8_t DHT11_Data_Array[6] = {0};
uint8_t DHT11_Data_Char[2][10]={0};

void Dht11_delay_ms(uint32_t C_time)
{
	vTaskDelay(C_time / portTICK_RATE_MS);
}

void DHT11_Signal_Output(uint8_t Value_Vol)
{
    gpio_config_t io_conf;
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO5
    io_conf.pin_bit_mask = GPIO_MUX_PIN_SEL;
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);
}

void DHT11_Signal_Input(void)
{
    gpio_config_t io_conf;
    //interrupt of rising edge
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //bit mask of the pins, use GPIO4/5 here
    io_conf.pin_bit_mask = GPIO_MUX_PIN_SEL;
    //set as input mode
    io_conf.mode = GPIO_MODE_INPUT;
    //enable pull-up mode
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);
}

uint8_t DHT11_Start_Signal_JX(void)
{
	uint8_t C_delay_time = 0;	

	DHT11_Signal_Output(1);	
	Dht11_delay_ms(1);

        gpio_set_level(GPIO_MUX_IO_0, 0);
	Dht11_delay_ms(25);

	gpio_set_level(GPIO_MUX_IO_0,1);	// IO5 = 1
	ets_delay_us(5);
//	os_delay_us(5);		

	DHT11_Signal_Input();

	while( gpio_get_level(GPIO_MUX_IO_0)==1 && C_delay_time<50 )
	{
	        ets_delay_us(1);
	//	os_delay_us(1);		
		C_delay_time++;
	}

	if(C_delay_time >= 50)
		return 1;	


	C_delay_time = 0 ;

	while( gpio_get_level(GPIO_MUX_IO_0)==0 && C_delay_time<170 )
	{
	        ets_delay_us(1);
	//	os_delay_us(1);
		C_delay_time++;	
	}

	if(C_delay_time >= 170)
		return 2;	

	else
		return 0;	
}

uint8_t DHT11_Read_Bit(void)
{
	uint8_t C_delay_time = 0;	// ��ʱ��ʱ

	// �ȴ���Ӧ��Ϣ�ĵ͵�ƽ����ٵ�150us��
	//-------------------------------------------------------------
	while( gpio_get_level(GPIO_MUX_IO_0)==1 && C_delay_time<150 )
	{
	        ets_delay_us(1);
	//	os_delay_us(1);		// 1us��ʱ
		C_delay_time++;
	}

	C_delay_time = 0 ;		// �͵�ƽ��ʱ��ʼ

	// ����λ�ĵ͵�ƽʱ����ʱ�����200us��
	//-------------------------------------------------------------
	while( gpio_get_level(GPIO_MUX_IO_0)==0 && C_delay_time<120 )
	{
	        ets_delay_us(1);
	//	os_delay_us(1);
		C_delay_time++;	// �͵�ƽʱ��
	}

	// ����λ�ĵ͵�ƽ������������λ�ĸߵ�ƽ
	// ����"0"�ĸߵ�ƽʱ�� == [23��27us]
	// ����"1"�ĸߵ�ƽʱ�� == [68��74us]
	//------------------------------------------------
	ets_delay_us(45);
//	os_delay_us(45);	// ��������"0"�ĸߵ�ƽ����

	// ��ʱ45us�󣬼���ź��ߵ�ƽ
	// �����ʱ�ź��ߵ�ƽ==1 => ����=="1"
	// �����ʱ�ź��ߵ�ƽ==0 => ����=="0"
	//-------------------------------------
	return gpio_get_level(GPIO_MUX_IO_0);
}
//======================================================================

// ��ȡDHT11һ���ֽ�
//======================================================================
uint8_t DHT11_Read_Byte(void)
{
	uint8_t C_Bit = 0;	// λ����

	uint8_t T_DHT11_Byte_Data = 0;	// DHT11�ֽ�����

	for(; C_Bit<8; C_Bit++)		// ��ȡDHT11һ���ֽ�
	{
		T_DHT11_Byte_Data <<= 1;

		T_DHT11_Byte_Data |= DHT11_Read_Bit();	// һλһλ�Ķ�ȡ
	}

	return T_DHT11_Byte_Data;	// ���ض�ȡ�ֽ�
}
//======================================================================



// �����Ķ�ȡDHT11����
//-----------------------------------------------
// ����ֵ��		0		DHT11���ݶ�ȡ�ɹ�
//				1		�����źŵĵ͵�ƽʱ����ʱ
//				2		����DHT11����_ʧ��
//				3		У�����
//==============================================================================
uint8_t DHT11_Read_Data_Complete(void)
{
	uint8_t C_delay_time = 0;	// ��ʱ��ʱ

	// ����DHT11����_�ɹ�
	//------------------------------------------------------------------------
	if(DHT11_Start_Signal_JX() == 0)	// DHT11�������ʼ�źţ���������Ӧ�ź�
	{
		DHT11_Data_Array[0] = DHT11_Read_Byte();	// ʪ��_����_����
		DHT11_Data_Array[1] = DHT11_Read_Byte();	// ʪ��_С��_����
		DHT11_Data_Array[2] = DHT11_Read_Byte();	// �¶�_����_����
		DHT11_Data_Array[3] = DHT11_Read_Byte();	// �¶�_С��_����
		DHT11_Data_Array[4] = DHT11_Read_Byte();	// У���ֽ�


		while(gpio_get_level(GPIO_MUX_IO_0)==1 && C_delay_time<100)
		{
	                ets_delay_us(1);
		//	os_delay_us(1);		// 1us��ʱ
			C_delay_time++;
		}

		C_delay_time = 0 ;		// �͵�ƽ��ʱ��ʼ


		while(gpio_get_level(GPIO_MUX_IO_0)==0 && C_delay_time<100)
		{
	                ets_delay_us(1);
		//	os_delay_us(1);		// 1us��ʱ
			C_delay_time++;
		}

		if(C_delay_time >= 100)
			return 1;		// ����1����ʾ�������źŵĵ͵�ƽʱ����ʱ


		if(	DHT11_Data_Array[4] ==
			DHT11_Data_Array[0] + DHT11_Data_Array[1] +
			DHT11_Data_Array[2] + DHT11_Data_Array[3] )
		{

			if((DHT11_Data_Array[3]&0x80) == 0)
			{
				DHT11_Data_Array[5] = 1;		// >=0��
			}
			else
			{
				DHT11_Data_Array[5] = 0;		// <0��

				DHT11_Data_Array[3] &= 0x7F;	// �����¶�С������
			}


			return 0;	// ����0����ʾ����ʪ�ȶ�ȡ�ɹ�
		}

		else return 3;		// ����3����ʾ��У�����
	}

	else return 2;		// ����2����ʾ������DHT11���䣬ʧ��
}


void DHT11_NUM_Char(void)
{
	uint8_t C_char = 0;		// �ַ�����

	if(DHT11_Data_Array[0]/100)			// ʪ�������İ�λ
		DHT11_Data_Char[0][C_char++] = DHT11_Data_Array[0]/100 + 48;

	if((DHT11_Data_Array[0]%100)/10)	// ʪ��������ʮλ
		DHT11_Data_Char[0][C_char++] = (DHT11_Data_Array[0]%100)/10 + 48;

	DHT11_Data_Char[0][C_char++] = DHT11_Data_Array[0]%10 + 48;

	DHT11_Data_Char[0][C_char++] = '.';		// С����

	DHT11_Data_Char[0][C_char++] = DHT11_Data_Array[1]%10 + 48;

	DHT11_Data_Char[0][C_char++] = ' ';		// ' '
	DHT11_Data_Char[0][C_char++] = '%';		// '%'
	DHT11_Data_Char[0][C_char++] = 'R';		// 'R'
	DHT11_Data_Char[0][C_char++] = 'H';		// 'H'
	DHT11_Data_Char[0][C_char] 	 =  0 ;		// ��0

	C_char = 0;		// ����

	if(DHT11_Data_Array[5]==0)			// �¶� < 0��
		DHT11_Data_Char[1][C_char++] = '-';

	if(DHT11_Data_Array[2]/100)			// ʪ�������İ�λ
		DHT11_Data_Char[1][C_char++] = DHT11_Data_Array[2]/100 + 48;

	if((DHT11_Data_Array[2]%100)/10)	// ʪ��������ʮλ
		DHT11_Data_Char[1][C_char++] = (DHT11_Data_Array[2]%100)/10 + 48;

	DHT11_Data_Char[1][C_char++] = DHT11_Data_Array[2]%10 + 48;

	DHT11_Data_Char[1][C_char++] = '.';		// С����

	DHT11_Data_Char[1][C_char++] = DHT11_Data_Array[3]%10 + 48;

	DHT11_Data_Char[1][C_char++] = ' ';		// ' '
	DHT11_Data_Char[1][C_char++] = 'C';		// 'C'
	DHT11_Data_Char[1][C_char]   =  0 ;		// ��0
}
