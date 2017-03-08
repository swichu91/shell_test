//
// This file is part of the GNU ARM Eclipse distribution.
// Copyright (c) 2014 Liviu Ionescu.
//

// ----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include "diag/Trace.h"
#include "stm32f7xx_hal.h"

#include "FreeRTOS.h"
#include "task.h"

#include "console.h"

#include "../bsp/lcd/stm32746g_discovery_lcd.h"
#include "../bsp/lcd/rk043fn48h.h"

#include "stm32746g_discovery_sd.h"

#include "ff_sddisk.h"
#include "ff_ramdisk.h"

void vApplicationTickHook(void);
void vApplicationIdleHook(void);
void vApplicationMallocFailedHook(void);
void vApplicationStackOverflowHook(TaskHandle_t pxTask, signed char *pcTaskName);

uint64_t u64IdleTicksCnt = 0; // Counts when the OS has no task to execute.
uint64_t tickTime = 0;        // Counts OS ticks (default = 1000Hz).


/* The MAC address array is not declared const as the MAC address will
 normally be read from an EEPROM and not hard coded (in real deployed
 applications).*/
uint8_t ucMACAddress[6] = { MAC_ADDR0, MAC_ADDR1, MAC_ADDR2,
MAC_ADDR3, MAC_ADDR4, MAC_ADDR5 };

const char* welcome_logo = "\r\nWelcome STM32F7 Discovery!\r\n"
		"Software Version : 1.00 alpha\r\n"
		"Created by: Mateusz Piesta\r\n\r\n\r\n";


void vApplicationTickHook(void) {
	++tickTime;

}

// This FreeRTOS call-back function gets when no other task is ready to execute.
// ----------------------------------------------------------------------------
void vApplicationIdleHook(void) {

	++u64IdleTicksCnt;
}

// A required FreeRTOS function.
// ----------------------------------------------------------------------------
void vApplicationMallocFailedHook(void) {

	assert_param("Malloc failed!\n");
}

void vApplicationStackOverflowHook(TaskHandle_t pxTask, signed char *pcTaskName) {
	(void) pcTaskName;
	(void) pxTask;

	assert_param("Holy FUCK! STACK OVERFLOW !!\n");
}

void vOneSecTask(void* pvp)
{
	(void)pvp;


	while(1) {

		HAL_GPIO_TogglePin(GPIOI,GPIO_PIN_1);
		vTaskDelay(1000);
	}
}
#define SECTOR_COUNT 8192
__attribute__ ((section (".sdram"))) uint8_t ramdisk[SECTOR_COUNT*512];


void Main_task(void * pvparameters) {

	console_init();
	//print welcome msg with actual software version
	console_send_str(welcome_logo);
	//console_mngt_init();
	//MX_USB_HOST_Init();

	  /* LCD Initialization */
	  BSP_LCD_Init();

	  /* LCD Initialization */
	  BSP_LCD_LayerDefaultInit(0, LCD_FB_START_ADDRESS);
	  BSP_LCD_LayerDefaultInit(1, LCD_FB_START_ADDRESS+(BSP_LCD_GetXSize()*BSP_LCD_GetYSize()*4));

	  /* Enable the LCD */
	  BSP_LCD_DisplayOn();

	  /* Select the LCD Background Layer  */
	  BSP_LCD_SelectLayer(0);

	  /* Clear the Background Layer */
	  BSP_LCD_Clear(LCD_COLOR_DARKRED);

	  /* Select the LCD Foreground Layer  */
	  BSP_LCD_SelectLayer(1);

	  /* Clear the Foreground Layer */
	  BSP_LCD_Clear(LCD_COLOR_DARKRED);

	  /* Configure the transparency for foreground and background :
	     Increase the transparency */
	  //BSP_LCD_SetTransparency(0, 0);
	  //BSP_LCD_SetTransparency(1, 100);

	  BSP_LCD_DisplayStringAtLine(8, (uint8_t*)"Kochana Edytka !!");

	  BSP_LCD_FillCircle(200,100,50);


	  FF_SDDiskInit("/sd");
	  FF_RAMDiskInit("/ram",ramdisk,SECTOR_COUNT,2048);

	xTaskCreate(vOneSecTask, "1_sec", 3*configMINIMAL_STACK_SIZE, NULL, 0, NULL);


	//prvCreateDiskAndExampleFiles(); //TODO: nie postawie fata na ramie w procku bo mam go za malo... minimalnie to 2Mb
	//sprvCreateDiskAndExampleFiles();

	/* Define the network addressing.  These parameters will be used if either
	 ipconfigUDE_DHCP is 0 or if ipconfigUSE_DHCP is 1 but DHCP auto configuration
	 failed. */
	static const uint8_t ucIPAddress[4] = { 192, 168, 1, 200 };
	static const uint8_t ucNetMask[4] = { 255, 0, 0, 0 };
	static const uint8_t ucGatewayAddress[4] = { 192, 168, 1, 1 };

	/* The following is the address of an OpenDNS server. */
	static const uint8_t ucDNSServerAddress[4] = { 208, 67, 222, 222 };

	/* Initialise the RTOS's TCP/IP stack.  The tasks that use the network
	 are created in the vApplicationIPNetworkEventHook() hook function
	 below.  The hook function is called when the network connects. */
	//FreeRTOS_IPInit(ucIPAddress, ucNetMask, ucGatewayAddress, ucDNSServerAddress, ucMACAddress);

	vTaskDelete(NULL);
}


int
main(int argc, char* argv[])
{
	(void)argc;
	(void)argv;
  // At this stage the system clock should have already been configured
  // at high speed.

	xTaskCreate(Main_task, "Main", 1096, NULL, 7, NULL);

	vTaskStartScheduler();

	//should not get here

  // Infinite loop
  while (1)
    {
       // Add your code here.
    }
}

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
