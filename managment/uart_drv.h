/*
 * uart_drv.h
 *
 *  Created on: 17 lut 2017
 *      Author: Mateusz
 */

#ifndef UART_DRV_H_
#define UART_DRV_H_

#include <stdint.h>
#include <stddef.h>
#include "stm32f7xx_hal.h"
#include "stm32f746xx.h"

void uart_drv_init(uint32_t baudrate);

void uart_put_string(const char* string);

inline void SET_TC_INTERRUPT(void){
	(USART6->CR1 |= USART_CR1_TCIE);
}
inline void CLR_TC_INTERRUPT(void){
	(USART6->ICR |= USART_ICR_TCCF);
}
inline void SET_TXE_INTERRUPT(void){
	(USART6->CR1 |= USART_CR1_TXEIE);
}
inline void CLR_TXE_INTERRUPT(void){
	(USART6->CR1 &= ~USART_CR1_TXEIE);
}

inline void SET_RXE_INTERRUPT(void){
	(USART6->CR1 |= USART_CR1_RXNEIE);
}

inline void CLR_RXE_INTERRUPT(void){
	(USART6->CR1 &= ~USART_CR1_RXNEIE);
}

#endif /* UART_DRV_H_ */
