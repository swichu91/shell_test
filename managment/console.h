/*
 * console.h
 *
 *  Created on: 18 lut 2017
 *      Author: Mateusz
 */

#ifndef CONSOLE_H_
#define CONSOLE_H_

#define CONSOLE_TX_QUEUE_SIZE 	128
#define CONSOLE_RX_QUEUE_SIZE 	64
#define CONSOLE_TASK_STACK_SIZE	256
#define CONSOLE_BAUDRATE	3000000
#define CONSOLE_LINEBUFF_SIZE	64

void console_init(void);
void console_send_str(const char* str);


#endif /* CONSOLE_H_ */
