/*
 * print_macros.h
 *
 *  Created on: 26 paü 2015
 *      Author: Mateusz
 */

#ifndef PRINT_MACROS_H_
#define PRINT_MACROS_H_

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "uart_drv.h"


char print_buffer[512];


//wyswietlanie aktualnej daty
#define PRINT_SYSTIME	do{	usart_put_string("Current time:\n");	\
					sprintf(print_buffer,"%d-%02d-%02d\n%02d:%02d:%02d\n",sys_time.year,sys_time.month,sys_time.day,sys_time.hour,sys_time.min,sys_time.sec);	\
					usart_put_string(print_buffer);	\
					usart_put_string("\n");	\
}while(0)


//printf ogolnego przeznaczenia
#define print_gen(...) do{	\
		usart_put_string("\n");	\
	sprintf(print_buffer,##__VA_ARGS__);	\
	usart_put_string(print_buffer);\
	usart_put_string("\n");\
}while(0)




//wewnetrzny printf dla systemu FAT
#define print_FAT(...) do{	\
		usart_put_string("\n");	\
	sprintf(print_buffer,##__VA_ARGS__);	\
	usart_put_string(print_buffer);\
}while(0)


//wewnetrzny printf m.in dla stosu TCP/IP, uzywac tylko z podwojnymi nawiasami np. FreeRTOS_printf(("%d",test))
#define FreeRTOS_printfx(...) do{	\
	sprintf(print_buffer,##__VA_ARGS__);	\
	usart_put_string(print_buffer);\
}while(0)




#endif /* PRINT_MACROS_H_ */
