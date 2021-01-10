#include "ds18b20.h"

#define GPIO_MUX_IO_0     5
#define GPIO_MUX_PIN_SEL  (1UL << GPIO_MUX_IO_0)

static void DS18B20_IO_OUT(void)
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
	io_conf.pull_up_en = 1;
	//configure GPIO with the given settings
	gpio_config(&io_conf);
}

static void DS18B20_IO_IN(void)
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

static void delay_us(uint32_t u)
{
	ets_delay_us(u);
}

void DS18B20_Rst(void) 
{                 
	DS18B20_IO_OUT(); //SET PG11 OUTPUT
	gpio_set_level(GPIO_MUX_IO_0, 0); //����DQ
	delay_us(750);    //����750us
	gpio_set_level(GPIO_MUX_IO_0, 1); //DQ=1 
	delay_us(15);     //15US
}
//�ȴ�DS18B20�Ļ�Ӧ
//����1:δ��⵽DS18B20�Ĵ���
//����0:����
uint8_t DS18B20_Check(void) 	   
{   
	uint8_t retry=0;
	DS18B20_IO_IN();//SET PG11 INPUT	 
	while (gpio_get_level(GPIO_MUX_IO_0)&&retry<200)
	{
		retry++;
		delay_us(1);
	};	 
	if(retry>=200)return 1;
	else retry=0;
	while (!gpio_get_level(GPIO_MUX_IO_0)&&retry<240)
	{
		retry++;
		delay_us(1);
	};
	if(retry>=240)return 1;	    
	return 0;
}
//��DS18B20��ȡһ��λ
//����ֵ��1/0
uint8_t DS18B20_Read_Bit(void) 			 // read one bit
{
	uint8_t data;
	DS18B20_IO_OUT();//SET PG11 OUTPUT
	gpio_set_level(GPIO_MUX_IO_0, 0); 
	delay_us(2);
	gpio_set_level(GPIO_MUX_IO_0, 1); 
	DS18B20_IO_IN();//SET PG11 INPUT
	delay_us(12);
	if(gpio_get_level(GPIO_MUX_IO_0))data=1;
	else data=0;	 
	delay_us(50);           
	return data;
}
//��DS18B20��ȡһ���ֽ�
//����ֵ������������
uint8_t DS18B20_Read_Byte(void)    // read one byte
{        
	uint8_t i,j,dat;
	dat=0;
	for (i=1;i<=8;i++) 
	{
		j=DS18B20_Read_Bit();
		dat=(j<<7)|(dat>>1);
	}						    
	return dat;
}
//дһ���ֽڵ�DS18B20
//dat��Ҫд����ֽ�
void DS18B20_Write_Byte(uint8_t dat)     
{             
	uint8_t j;
	uint8_t testb;
	DS18B20_IO_OUT();//SET PG11 OUTPUT;
	for (j=1;j<=8;j++) 
	{
		testb=dat&0x01;
		dat=dat>>1;
		if (testb) 
		{
			gpio_set_level(GPIO_MUX_IO_0, 0);// Write 1
			delay_us(2);                            
			gpio_set_level(GPIO_MUX_IO_0, 1);
			delay_us(60);             
		}
		else 
		{
			gpio_set_level(GPIO_MUX_IO_0, 0);// Write 0
			delay_us(60);             
			gpio_set_level(GPIO_MUX_IO_0, 1);
			delay_us(2);                          
		}
	}
}
//��ʼ�¶�ת��
void DS18B20_Start(void)// ds1820 start convert
{   						               
	DS18B20_Rst();	   
	DS18B20_Check();	 
	DS18B20_Write_Byte(0xcc);// skip rom
	DS18B20_Write_Byte(0x44);// convert
} 
//��ʼ��DS18B20��IO�� DQ ͬʱ���DS�Ĵ���
//����1:������
//����0:����    	 
uint8_t DS18B20_Init(void)
{
	DS18B20_Rst();
	return DS18B20_Check();
}  
//��ds18b20�õ��¶�ֵ
//���ȣ�0.1C
//����ֵ���¶�ֵ ��-550~1250�� 
short DS18B20_Get_Temp(void)
{
	uint8_t temp;
	uint8_t TL,TH;
	short tem;

	DS18B20_Start();                    // ds1820 start convert
	DS18B20_Rst();
	DS18B20_Check();	 
	DS18B20_Write_Byte(0xcc);// skip rom
	DS18B20_Write_Byte(0xbe);// convert	    
	TL=DS18B20_Read_Byte(); // LSB   
	TH=DS18B20_Read_Byte(); // MSB   
	if(TH>7)
	{
		TH=~TH;
		TL=~TL; 
		temp=0;//�¶�Ϊ��  
	}else temp=1;//�¶�Ϊ��	  	  
	tem=TH; //��ø߰�λ
	tem<<=8;    
	tem+=TL;//��õװ�λ
	tem=(double)tem*0.625;//ת��     
	if (temp)
		return tem; //�����¶�ֵ
	else 
		return -tem;    
}

