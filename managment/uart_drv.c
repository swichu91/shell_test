/*
 * uart_drv.c
 *
 *  Created on: 17 lut 2017
 *      Author: Mateusz
 */

#include "uart_drv.h"
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

#include "shell/shell.h"


extern QueueHandle_t rx_queue;

/* Forward declarations */
void USART6_IRQHandler(void);


void uart_drv_init(uint32_t baudrate)
{

	 RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;

	 RCC->APB2ENR |=RCC_APB2ENR_USART6EN;

	 GPIO_InitTypeDef Rx_pin,TX_pin;

	 Rx_pin.Pin = GPIO_PIN_7;
	 Rx_pin.Mode = GPIO_MODE_AF_OD;
	 Rx_pin.Pull = GPIO_NOPULL;
	 Rx_pin.Alternate = GPIO_AF8_USART6;
	 Rx_pin.Speed = GPIO_SPEED_MEDIUM;


	 TX_pin.Pin = GPIO_PIN_6;
	 TX_pin.Mode = GPIO_MODE_AF_PP;
	 TX_pin.Pull = GPIO_NOPULL;
	 TX_pin.Speed = GPIO_SPEED_MEDIUM;
	 TX_pin.Alternate = GPIO_AF8_USART6;

	 HAL_GPIO_Init(GPIOC,&Rx_pin);
	 HAL_GPIO_Init(GPIOC,&TX_pin);

	 CLR_RXE_INTERRUPT();
	 CLR_TXE_INTERRUPT();

	 USART6->BRR = APB2_PERPH_FREQ/baudrate;

	 USART6->CR1 |= USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;

	 NVIC_SetPriority(USART6_IRQn,10);
	 NVIC_EnableIRQ(USART6_IRQn);
}

static void uart_put_char(char byte)
{

	/*SET_TC_INTERRUPT();
	if(console_mngt_txrdy_semph && (xSemaphoreTake(console_mngt_txrdy_semph,20)==pdPASS))
	{

		taskENTER_CRITICAL();
		USART6->TDR = (uint32_t)byte;
		taskEXIT_CRITICAL();
	}
	else*/

	/* some kind of spin lock */
	while(!(USART6->ISR & USART_ISR_TXE));
		USART6->TDR = (uint32_t)byte;



}

void uart_put_string(const char* string)
{
	while(*string)
	{
		uart_put_char(*string);
		string++;

	}
}


// RX USART interrupt
void USART6_IRQHandler(void)
{
	int8_t onechar;

	long lHigherPriorityTaskWoken = pdFALSE;

	if(USART6->ISR & USART_ISR_RXNE) // RX interrupt
	{
		onechar=(char) USART6->RDR;

		xQueueSendFromISR(rx_queue,&onechar,&lHigherPriorityTaskWoken);


	}

	if(USART6->ISR & USART_ISR_ORE) // overrun interrupt
	{
		USART6->ICR |= USART_ICR_ORECF;
	}

	//if(USART6->SR & USART_SR_TXE) // data register empty
	//{
		//xSemaphoreGiveFromISR(console_mngt_txrdy_semph,&lHigherPriorityTaskWoken);
	//	CLR_TXE_INTERRUPT();

	//}
	/*
	if(USART6->ISR & USART_ISR_TC) // trasmit complete interrupt
	{
		CLR_TC_INTERRUPT(); //disable TC intterupt
		USART6->ICR |= USART_ICR_TCCF; // clear interrupt flag
		xSemaphoreGiveFromISR(console_mngt_txrdy_semph,&lHigherPriorityTaskWoken);
	}*/


	portEND_SWITCHING_ISR( lHigherPriorityTaskWoken );

}


