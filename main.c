#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_flash.h"
#include "misc.h"

#include "stm32f4xx.h"
#include "stm32f4xx_conf.h"

#include <stdio.h>
#include "main.h"

#include "stm32f4xx_it.h"
#include "spi_sd.h"
#include "delay.h"
#include "aes.h"

// Bootloader key configuration
#define BOOTLOADER_KEY_START_ADDRESS            (uint32_t)0x08004000
#define BOOTLOADER_KEY_VALUE                    0x4354524C

// Flash configuration
#define MAIN_PROGRAM_START_ADDRESS              (uint32_t)0x08008000
//-----------------------------------Definition Flash Sector--------------------------------
#define ADDR_FLASH_SECTOR_0     ((uint32_t)0x08000000) /* Base @ of Sector 0, 16 Kbytes */
#define ADDR_FLASH_SECTOR_1     ((uint32_t)0x08004000) /* Base @ of Sector 1, 16 Kbytes */
#define ADDR_FLASH_SECTOR_2     ((uint32_t)0x08008000) /* Base @ of Sector 2, 16 Kbytes */
#define ADDR_FLASH_SECTOR_3     ((uint32_t)0x0800C000) /* Base @ of Sector 3, 16 Kbytes */
#define ADDR_FLASH_SECTOR_4     ((uint32_t)0x08010000) /* Base @ of Sector 4, 64 Kbytes */
#define ADDR_FLASH_SECTOR_5     ((uint32_t)0x08020000) /* Base @ of Sector 5, 128 Kbytes */
#define ADDR_FLASH_SECTOR_6     ((uint32_t)0x08040000) /* Base @ of Sector 6, 128 Kbytes */
#define ADDR_FLASH_SECTOR_7     ((uint32_t)0x08060000) /* Base @ of Sector 7, 128 Kbytes */
#define ADDR_FLASH_SECTOR_8     ((uint32_t)0x08080000) /* Base @ of Sector 8, 128 Kbytes */
#define ADDR_FLASH_SECTOR_9     ((uint32_t)0x080A0000) /* Base @ of Sector 9, 128 Kbytes */
#define ADDR_FLASH_SECTOR_10    ((uint32_t)0x080C0000) /* Base @ of Sector 10, 128 Kbytes */
#define ADDR_FLASH_SECTOR_11    ((uint32_t)0x080E0000) /* Base @ of Sector 11, 128 Kbytes */
//---------------------------------Definition system error code------------------------------
#define FLASHERASE_ERROR        0
#define LINKDISK_ERROR          1
#define FILE_ERROR              2
#define COPY_ERROR              3
#define CLKCONF_ERROR           4

//Prototype
void CMD_init(void);
void SetKey(void);
void ResetKey(void);
uint32_t ReadKey(void);
void GPIO_Indicators_init(void);
uint32_t ReadPoint(void);

FRESULT result;
FIL firmware;

struct AES_ctx ctx;
static const uint8_t KEY[] = { 0xB4, 0xEF, 0x74, 0x56, 0x7A, 0x87, 0xC0, 0xF2, 0x5A, 0x8F, 0xE5, 0x4A, 0x88, 0xD6, 0x1C, 0x2C };
static const uint8_t IV[] = { 0xA6, 0x52, 0xB6, 0xDC, 0x92, 0x22, 0x15, 0x07, 0x25, 0x8B, 0x76, 0x14, 0x67, 0x80, 0x7D, 0xA7 };

typedef  void (*pFunction)(void);
uint8_t readBuffer[512];
uint32_t firmwareBytesToRead = 0;
uint32_t firmwareBytesCounter = 0;
uint32_t currentAddress = 0;
UINT readBytes = 0;
uint32_t jumpAddress = 0;
pFunction Jump_To_Application;

int main(void)
{
	GPIO_Indicators_init();
	if (ReadKey() == BOOTLOADER_KEY_VALUE)
	{
		GPIOG->ODR |= GPIO_ODR_ODR_2;
		GPIOG->ODR &= ~GPIO_ODR_ODR_0;
		ResetKey();
	    __disable_irq();
	    NVIC_SetVectorTable(NVIC_VectTab_FLASH, MAIN_PROGRAM_START_ADDRESS);

	    jumpAddress = *(__IO uint32_t*) (MAIN_PROGRAM_START_ADDRESS + 4);
	    Jump_To_Application = (pFunction) jumpAddress;
	    __set_MSP(*(__IO uint32_t*) MAIN_PROGRAM_START_ADDRESS);
	    GPIOG->ODR &= ~GPIO_ODR_ODR_2;
	    Jump_To_Application();
	}
	else
	{
		GPIOG->ODR |= GPIO_ODR_ODR_0;
		SystemInit();
		CMD_init();
		AES_init_ctx_iv(&ctx, KEY, IV);
		if (f_mount(0, &FATFS_Obj) == FR_OK)	//Монтируем модуль FatFs
		{
			uint8_t path[13]="firmware.bin";
			path[12] = '\0';

			result = f_open(&firmware, (char*)path, FA_READ);

			if(result == FR_OK)
			{
				GPIOG->ODR |= GPIO_ODR_ODR_1;
				GPIOG->ODR &= ~GPIO_ODR_ODR_0;
				FLASH_Unlock();
				FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

				FLASH_EraseSector(FLASH_Sector_2, VoltageRange_3);
				FLASH_EraseSector(FLASH_Sector_3, VoltageRange_3);
				FLASH_EraseSector(FLASH_Sector_4, VoltageRange_3);
				FLASH_EraseSector(FLASH_Sector_5, VoltageRange_3);
				FLASH_EraseSector(FLASH_Sector_6, VoltageRange_3);
				FLASH_EraseSector(FLASH_Sector_7, VoltageRange_3);
				FLASH_EraseSector(FLASH_Sector_8, VoltageRange_3);
				FLASH_EraseSector(FLASH_Sector_9, VoltageRange_3);
				FLASH_EraseSector(FLASH_Sector_10, VoltageRange_3);
				FLASH_EraseSector(FLASH_Sector_11, VoltageRange_3);

				firmwareBytesToRead = firmware.fsize;
				firmwareBytesCounter = 0;
				currentAddress = MAIN_PROGRAM_START_ADDRESS;

				while ((firmwareBytesToRead - firmwareBytesCounter) >= 512)
			    {
			       	f_read(&firmware, readBuffer, 512, &readBytes);
			       	firmwareBytesCounter += 512;

			       	AES_CBC_decrypt_buffer(&ctx, readBuffer, 512);

			       	for (uint32_t i = 0; i < 512; i += 4)
			       	{
			       		FLASH_ProgramWord(currentAddress, *(uint32_t*)&readBuffer[i]);
			       		currentAddress += 4;
			       	}
			    }
			    if (firmwareBytesToRead != firmwareBytesCounter)
			    {
		        	f_read(&firmware, readBuffer, (firmwareBytesToRead - firmwareBytesCounter), &readBytes);

			       	AES_CBC_decrypt_buffer(&ctx, readBuffer, (firmwareBytesToRead - firmwareBytesCounter));

		        	for (uint32_t i = 0; i < (firmwareBytesToRead - firmwareBytesCounter); i += 4)
		        	{
			       		FLASH_ProgramWord(currentAddress, *(uint32_t*)&readBuffer[i]);
			       		currentAddress += 4;
		        	}
		        	firmwareBytesCounter = firmwareBytesToRead;
		        }

			    GPIOG->ODR &= ~GPIO_ODR_ODR_1;

			    FLASH_Lock();
			    f_close(&firmware);
			    f_unlink((char*)path);

			    if(ReadPoint() != 0x20003CD0)
			    {
				    ResetKey();
				    NVIC_SystemReset();
			    }
			    else
			    {
			    	ResetKey();
				    SetKey();
				    NVIC_SystemReset();
			    }
			}
			else
			{
				//Jump
				if (((*(uint32_t*)MAIN_PROGRAM_START_ADDRESS) & 0x2FFF0000 ) == 0x20000000)
				{
					ResetKey();
				    SetKey();
				    NVIC_SystemReset();
				}
				else
				{
					NVIC_SystemReset();
				}
			}
		}
	}
    while(1)
    {
    	;
    }
}
void CMD_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);	//Enable Port B

	// === Configure PB11 as SD Inserted Input
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}
void SetKey(void)
{
  FLASH_Unlock();
  FLASH_ProgramWord(BOOTLOADER_KEY_START_ADDRESS, BOOTLOADER_KEY_VALUE);
  FLASH_Lock();
}
void ResetKey(void)
{
	FLASH_Unlock();
	FLASH_EraseSector(FLASH_Sector_1, VoltageRange_3);
	FLASH_Lock();
}
uint32_t ReadKey(void)
{
	return (*(__IO uint32_t*) BOOTLOADER_KEY_START_ADDRESS);
}
uint32_t ReadPoint(void)
{
	return (*(__IO uint32_t*) MAIN_PROGRAM_START_ADDRESS);
}
void GPIO_Indicators_init(void)
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);
	GPIO_InitTypeDef GPIO_G;

	GPIO_G.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2;		//Red | Yellow | Green
	GPIO_G.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_G.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_G.GPIO_OType = GPIO_OType_PP;
	GPIO_G.GPIO_PuPd = GPIO_PuPd_DOWN;

	GPIO_Init(GPIOG, &GPIO_G);

	GPIOG->ODR &= ~(GPIO_ODR_ODR_0 | GPIO_ODR_ODR_1 | GPIO_ODR_ODR_2);
}
