/*
 * shell.h
 *
 *  Created on: 29 mar 2016
 *      Author: MateuszPiesta
 */

#ifndef MODULES_SHELL_SHELL_H_
#define MODULES_SHELL_SHELL_H_

#include "shell_conf.h"
#include <stdint.h>
#include <stddef.h>
#include "FreeRTOS_CLI.h"

#define SHELL_CMD_VARIABLE_PARAM_NR	-1

/**
 *  Prototype for shell_write function.
 *  str - string to be written
 *  len - string length
 *  param - generic pointer to pass/ret user's data to/from function
 *  When using FreeRTOS it is needed to use some kind of thread concurrency protection etc. mutexes
 *  when accessing param. */
typedef uint8_t (*shell_write_t)(const char* str, size_t len, void* param);

/**
 * Prototype for shell_read function
 * str - string read
 * param - generic pointer to pass/ret user's data to/from function
 *  When using FreeRTOS it is needed to use some kind of thread concurrency protection etc. mutexes
 *  when accessing param.
 */
typedef uint8_t (*shell_read_t)(char* str,uint32_t* len, void* param);

typedef CLI_Command_Definition_t shell_cmd_t;

typedef enum {
	AUTH_IN_PROGRESS = 1,
	AUTH_PASSED = 2
}auth_action;

/**
 * Structure that stores special codes used by terminal
 * Arrows, backspace and so on
 */
typedef struct _codes{
	const char *bs;
	const char *newline;
	const char *arrowl;
	const char *arrowr;
	const char *arrowu;
	const char *arrowd;
	const char *clear_line;
	const char *escape_seq;
}codes_t;

/**
 * Structure to hold main parameters of shell
 */
typedef struct _shell {

	void* hist_queue;

	const char* line_prompt;
	char* line_buff;
	uint32_t line_buff_pos;
	size_t line_buff_size;

	/*
	 * Pointer to function to write string into terminal
	 */
	shell_write_t write;

	/*
	 * Pointer to function to read user's input from terminal
	 */
	shell_read_t read;

	codes_t codes;

	void* param;

} shell_t;


/*
 * Return codes
 */
typedef enum{
	SHELL_OK=0,
	SHELL_ERR_GEN=1,
	SHELL_ERR_TAB_FULL=2,
	SHELL_ERR_SPACE=3,
	SHELL_ERR_MEM=4,

}sherr_t;

/**
 * Initialize main shell structure and set linebuff length(number of characters
 * that user can input in one line)
 */
sherr_t shell_Init(shell_t* shell, size_t linebuf_len);

/**
 * Register custom set of special characters(backspace,arrows etc.).
 * User has to fill codes_t structure with valid special codes beefore passing it to the function.
 */
uint8_t shell_RegisterCodes(const shell_t* shell, codes_t* codes);

/**
 * Register shell CLI command. See shell_cmd_t definition for info how to prepare command structure and so on..
 *  */
uint8_t shell_RegisterCmd( const shell_cmd_t * const pxCommandToRegister );

/**
 * Register terminal input/output functions aka read/write.
 * Whole shell is completely separated from low-level layer. Low level layer can be TELENT or simple serial
 * protocol or whatever. */
void shell_RegisterIOFunctions(shell_t* shell,
		shell_write_t write,
		shell_read_t read);

/**
 * Perform all shell actions. There are two options for invoking this function which depends on
 *  how low-level layer is implemented. It can be non-blocking either blocking. Blocking variant is
 *  only possible when using FreeRTOS (SHELL_NO_FREERTOS is undefined).
*/
void shell_RunPeriodic(shell_t* shell);

/**
 * Pass generic pointer to params struct which will by used by I/O functions.
 * For example user can pass connections parameters in case of using TELNET layer.
 */
void shell_PassParam(shell_t* shell,void* param, size_t param_len);



#endif /* MODULES_SHELL_SHELL_H_ */
