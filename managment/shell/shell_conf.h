/*
 * shell_conf.h
 *
 *  Created on: 29 mar 2016
 *      Author: MateuszPiesta
 */

#ifndef MODULES_SHELL_SHELL_CONF_H_
#define MODULES_SHELL_SHELL_CONF_H_

#include <assert.h>

/**
 * Number of maximum registere commands supported by shell
 */
#define SHELL_MAX_NR_REGISTERED_COMMANDS	50

/*
 * Depth of command line history. Depth is limited because of strict memory managment requirements
 * and because of fact that embedded devices usually doesn't need that big commands history.
 */
#define SHELL_CMD_HISTORY_DEPTH	5

/**
 * Maximum number of parameters which can be passed to invoked function.
 */
#define SHELL_CMD_MAX_PARAM_NR	10

/**
 * Maximal length of each parameter.
 */
#define SHELL_CMD_MAX_PARAM_LEN	10

/**
 * Maximal length of password.
 */
#define SHELL_MAX_PASSWORD_LEN	32

/* Shell can be used with our without(U2) FreeRTOS.
 * In case of  using FreeRTOS comment define below ! */
//#define SHELL_NO_FREERTOS

/**
 * Maximal output buffer size needed for passing function's response back to terminal.
 */
#define SHELL_MAX_OUTPUT_BUFFER_SIZE	512

/**
 * Maximal number of characters passed to shell at once. When using TELNET data transmission
 * can be buffered which sometimes can cause reception of more than once characters.
 * When using USART/RS232 set it to 1 or 2.
 */
#define SHELL_READ_BUFFER_LEN	32

/**
 * Define assert function if you want to use asserts.
 */
#define SHELL_ASSERT(X)	assert(X)

#ifndef SHELL_NO_FREERTOS
#include "FreeRTOS.h"
#include "task.h"
#endif

#if (SHELL_MAX_PASSWORD_LEN > SHELL_MAX_OUTPUT_BUFFER_SIZE)
#error "Shell password musn't be longer than SHELL_MAX_OUTPUT_BUFFER_SIZE"
#endif
#endif /* MODULES_SHELL_SHELL_CONF_H_ */
