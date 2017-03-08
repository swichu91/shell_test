/*
 * console.c
 *
 *  Created on: 18 lut 2017
 *      Author: Mateusz
 */

#include "console.h"
#include "uart_drv.h"
#include "shell/shell.h"

#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <strings.h>
#include <stdlib.h>
#include "stm32f7xx_hal.h"
#include "stm32f746xx.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "cmds/cmds.h"

static shell_t local_shell;
QueueHandle_t rx_queue = NULL;
QueueHandle_t tx_queue = NULL;


static void console_task(void* param);
static void chartx_task(void* param);

static uint8_t write_uart(const char* str, size_t len,void* param);
static uint8_t read_uart(char* str,uint32_t* len, void* param);


void console_init(void)
{
	/* Disable UART interrupts during initialization */
	CLR_RXE_INTERRUPT();
	CLR_TC_INTERRUPT();
	CLR_TXE_INTERRUPT();

	rx_queue =xQueueCreate(CONSOLE_RX_QUEUE_SIZE,sizeof(char));
	if(rx_queue == NULL){
		return;
	}

	tx_queue =xQueueCreate(CONSOLE_TX_QUEUE_SIZE,sizeof(char));
	if(tx_queue == NULL){
		return;
	}

	if(xTaskCreate(console_task, "console", CONSOLE_TASK_STACK_SIZE, NULL, 0, NULL) == pdFAIL){
		return;
	}

	if(xTaskCreate(chartx_task, "char_tx", CONSOLE_TASK_STACK_SIZE, NULL, 0, NULL) == pdFAIL){
		return;
	}

	shell_Init(&local_shell,CONSOLE_LINEBUFF_SIZE);

	shell_RegisterIOFunctions(&local_shell,write_uart,read_uart);

	fs_RegisterCmds();



	/* It is safe to turn on UART as all of necessary structures are already allocated/created */
	uart_drv_init(CONSOLE_BAUDRATE);

	SET_RXE_INTERRUPT();

}

void console_send_str(const char* str)
{
	if(tx_queue != NULL){

		while(*str != '\0'){
			xQueueSend(tx_queue,str,64);
			str++;
		}
	}
}



static void console_task(void* param)
{
	(void) param;

	while(1)
	{
		shell_RunPeriodic(&local_shell);
	}
}

static void chartx_task(void* param)
{
	(void) param;
	char tx_char[] = {0,'\0'};

	while(1)
	{
		(void)param;

		//loop until there is no more chars in queue
		while(xQueueReceive(tx_queue,&tx_char[0],portMAX_DELAY)==pdTRUE){

			uart_put_string(tx_char);
			if(uxQueueMessagesWaiting(tx_queue) == 0)
				break;
		}

	}
}

static uint8_t write_uart(const char* str, size_t len,void* param)
{
	(void)len;
	(void)param;
	//usart_put_string(str);
	console_send_str(str);
	return 0;
}

static uint8_t read_uart(char* str,uint32_t* len, void* param)
{
	(void)param;
	uint32_t char_rx_count = 0;

	//loop until there is no more chars in queue
	while(xQueueReceive(rx_queue,str,portMAX_DELAY)==pdTRUE){
		char_rx_count++;
		str++;

		if(uxQueueMessagesWaiting(rx_queue) == 0)
			break;
	}
	*len = char_rx_count;
	return 0;
}
