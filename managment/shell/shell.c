/*
 * shell.c
 *
 *  Created on: 29 mar 2016
 *      Author: MateuszPiesta
 */

#include "shell.h"
#include "shell_conf.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "history_list.h"
#include "FreeRTOS_CLI.h"

#define SHOW_PASSWORD	0

/* Codes below are platform/device specific.*/
const char  shell_BS = 0x7F;
const char* shell_NEWLINE = "\r\n";
const char* shell_ARROW_LEFT = "D";
const char* shell_ARROW_RIGHT = "C";
const char* shell_ARROW_UP = "A";
const char* shell_ARROW_DOWN = "B";
const char* shell_start_of_escape_seq = "\e";
const char* shell_CLEAR_LINE ="2K";
/* Prompt code */
const char* shell_prompt = ">";
const char* shell_auth_prompt = "Please type password:";
const char* shell_auth_failed_prompt = "Wrong password!:";
const char* shell_logout_succes = "Logout success";

typedef enum {
	LINE_BUFF_OK = 0,
	LINE_BUFF_FULL = 1,
	LINE_BUFF_EMPTY = 2,
	LINE_BUFF_REACH_MIN = 3,
	LINE_BUFF_REACH_MAX = 4,
	LINE_BUFF_UNSPECIFIED_CMD = 5
} lbuff_e;

typedef enum {
	LINE_BUFF_ADD_CHAR = 0,
	LINE_BUFF_BS = 1,
	LINE_BUFF_CLEAR_WHOLE = 2,
	LINE_BUFF_POS_LEFT = 3,
	LINE_BUFF_POS_RIGHT = 4,
	LINE_BUFF_ADD_STR	= 5
} lbuff_action;


/* Forward declarations */
static void shell_write_clearline(shell_t* shell);
static void shell_write_escape_sequence(shell_t* shell, const char* byte);
static const char* shell_ReturnLineBuff(shell_t* shell);
static void encryptDecrypt(const char* toEncrypt,char* encrypted);
static void shell_write_prompt(shell_t* shell);

static BaseType_t password_callback(char *pcWriteBuffer, size_t xWriteBufferLen, argv arg, size_t argc  );
static BaseType_t logout_callback(char *pcWriteBuffer, size_t xWriteBufferLen, argv arg, size_t argc  );

uint8_t shell_RegisterCmd( const shell_cmd_t * const pxCommandToRegister );

static hlist_t history = {0};
static char response_output_buff[SHELL_MAX_OUTPUT_BUFFER_SIZE] = {0};


/* Password is shared between shell instances for example:
 * If you created one instance of shell for local interface(RS/UART) and one for TELNET
 * the password will be the same for both instances. */
static char shell_password[SHELL_MAX_PASSWORD_LEN];

static auth_action auth_state;

static char read_buffer[SHELL_READ_BUFFER_LEN];

static char* linebuff_temp = NULL;

/**
 Basic common commands
 */

static shell_cmd_t password_cmd = {"password","	password -> Changes password",password_callback,1};
static shell_cmd_t logout_cmd = {"logout","	logout -> Logout from console",logout_callback,0};

/* Cmd to set new password */
static BaseType_t password_callback(char *pcWriteBuffer, size_t xWriteBufferLen, argv arg, size_t argc  ){

	(void) xWriteBufferLen;
	(void) argc;
	if(strlen(&arg[1][0]) <= SHELL_MAX_PASSWORD_LEN){
		encryptDecrypt(&arg[1][0],shell_password);
	}else{
		sprintf(pcWriteBuffer,"Password too long !");
	}
	return pdFALSE;
}

/* Cmd to logout from console*/
static BaseType_t logout_callback(char *pcWriteBuffer, size_t xWriteBufferLen, argv arg, size_t argc  ){

	(void) xWriteBufferLen;
	(void) argc;
	(void) arg;

	auth_state = AUTH_IN_PROGRESS;
	strcpy(pcWriteBuffer,shell_logout_succes);

	return pdFALSE;
}

sherr_t shell_Init(shell_t* shell, size_t linebuf_len) {

	SHELL_ASSERT(shell != NULL);

	memset(shell, 0, sizeof(*shell));

	if ((shell->line_buff = malloc(linebuf_len)) == NULL) {
		return SHELL_ERR_MEM;
	}
	shell->line_buff_size = linebuf_len;
	memset(shell->line_buff, 0, linebuf_len);

	history_list_Init(&history,SHELL_CMD_HISTORY_DEPTH);

	/* These are default codes which support Putty */
	shell->line_prompt = shell_prompt;
	shell->codes.bs = &shell_BS;
	shell->codes.newline = shell_NEWLINE;
	shell->codes.arrowd = shell_ARROW_DOWN;
	shell->codes.arrowl = shell_ARROW_LEFT;
	shell->codes.arrowr = shell_ARROW_RIGHT;
	shell->codes.arrowu = shell_ARROW_UP;

	auth_state = AUTH_IN_PROGRESS;

	/* Only for tests */
	encryptDecrypt("pass",shell_password);

	/* Register common basic commands */

	shell_RegisterCmd(&password_cmd);
	shell_RegisterCmd(&logout_cmd);


	linebuff_temp = calloc(shell->line_buff_size,sizeof(char));

	if(linebuff_temp == NULL){
		return SHELL_ERR_MEM;
	}else{
		return SHELL_OK;
	}

}

uint8_t shell_RegisterCodes(const shell_t* shell, codes_t* codes)
{
	memcpy((void*)&shell->codes,codes,sizeof(*codes));
	return SHELL_OK;
}

uint8_t shell_RegisterCmd( const shell_cmd_t * const pxCommandToRegister )
{
	return (uint8_t)FreeRTOS_CLIRegisterCommand(pxCommandToRegister);
}

void shell_RegisterIOFunctions(shell_t* shell,
		shell_write_t write,
		shell_read_t read) {

	SHELL_ASSERT(shell != NULL);
	SHELL_ASSERT(write != NULL);
	SHELL_ASSERT(read != NULL);

	shell->write = write;
	shell->read = read;

}

void shell_PassParam(shell_t* shell,void* param, size_t param_len)
{
#ifndef SHELL_NO_FREERTOS
	//obtain mutex
#endif

	memcpy(shell->param,param,param_len);

#ifndef SHELL_NO_FREERTOS
	//release mutex
#endif
}


 // inserts into subject[] at position pos
static void append(uint32_t buf_size,char subject[], const char insert[], uint32_t pos) {

	memset(linebuff_temp,0,buf_size);

    strncpy(linebuff_temp, subject, pos); // copy at most first pos characters
    size_t len = strlen(linebuff_temp);
    strcpy(linebuff_temp+len, insert); // copy all of insert[] at the end
    len += strlen(insert);  // increase the length by length of insert[]
    strcpy(linebuff_temp+len, subject+pos); // copy the rest

    strcpy(subject, linebuff_temp);   // copy it back to subject
}

static void encryptDecrypt(const char* toEncrypt,char* encrypted) {
    char key = 'K'; //Any char will work

    for (uint16_t i = 0; i < strlen(toEncrypt); i++)
        encrypted[i] = toEncrypt[i] ^ key;

}


static const char* shell_ReturnLineBuff(shell_t* shell)
{
	return shell->line_buff;
}

static lbuff_e shell_MngtLineBuff(shell_t* shell,const char* str, uint32_t str_len,
		lbuff_action action) {

	lbuff_e ret = LINE_BUFF_UNSPECIFIED_CMD;

	switch (action) {

	case LINE_BUFF_ADD_CHAR:
		if (shell->line_buff_pos >= shell->line_buff_size) {
			shell->line_buff_pos = 0;
			memset(shell->line_buff, 0, shell->line_buff_size);	// clear character buffer
			shell->line_buff[0] = '\0';
			ret = LINE_BUFF_FULL;
		}
		/* User moved cursor and now it points somewhere between beg and end of linebuff */
		else if (shell->line_buff_pos < strlen(shell->line_buff)){

			if (strlen(shell->line_buff) >= shell->line_buff_size) {
				ret = LINE_BUFF_FULL;
			}else{
				ret = LINE_BUFF_OK;

				append(shell->line_buff_size,shell->line_buff,str,shell->line_buff_pos);
				shell_write_clearline(shell);
				shell->write("\r",1,shell->param);
				shell_write_prompt(shell);

				shell->write(shell->line_buff,strlen(shell->line_buff),shell->param);

				uint32_t temp2 = strlen(shell->line_buff) - shell->line_buff_pos;
				while(temp2--){
					shell_write_escape_sequence(shell, shell->codes.arrowl);
				}
				shell->line_buff_pos++;
			}
		}
		/* Add char at the end of linebuff */
		else {
			shell->line_buff[shell->line_buff_pos] = *str;
			shell->line_buff_pos++;
			shell->line_buff[shell->line_buff_pos] = '\0';
			if (shell->line_buff_pos == strlen(shell->line_buff)) {
				shell->line_buff[shell->line_buff_pos] = '\0';
			}
			ret = LINE_BUFF_OK;
		}

		break;

	case LINE_BUFF_BS:
		if (shell->line_buff_pos > 0) {

			uint32_t curr_pos_temp = shell->line_buff_pos;
			uint32_t temp = shell->line_buff_pos - 1;
			uint32_t line_buff_len = strlen(shell->line_buff);

			/* Move characters on the right in line buffer */
			while(temp != line_buff_len){
				shell->line_buff[temp] = shell->line_buff[temp+1];
				temp++;
			}
			shell_write_clearline(shell);
			shell->write("\r",1,shell->param);
			shell_write_prompt(shell);

			shell->write(shell->line_buff,strlen(shell->line_buff),shell->param);

			uint32_t temp2 = line_buff_len - curr_pos_temp;
			while(temp2--){
				shell_write_escape_sequence(shell, shell->codes.arrowl);
			}

			shell->line_buff_pos--;

			ret = LINE_BUFF_OK;
		} else {
			ret = LINE_BUFF_EMPTY;
		}
		break;

	case LINE_BUFF_CLEAR_WHOLE:

		memset(shell->line_buff, 0, shell->line_buff_size);	// clear character buffer
		shell->line_buff[0] = '\0';
		shell->line_buff_pos = 0;
		ret = LINE_BUFF_OK;

		break;

	case LINE_BUFF_POS_LEFT:

		if (shell->line_buff_pos == 0) {
			return LINE_BUFF_REACH_MIN;
		} else {
			shell->line_buff_pos--;
			shell_write_escape_sequence(shell, shell->codes.arrowl);
			ret = LINE_BUFF_OK;
		}
		break;

	case LINE_BUFF_POS_RIGHT:

		if (shell->line_buff_pos == strlen(shell->line_buff)) {
			ret = LINE_BUFF_REACH_MAX;
		} else {
			shell->line_buff_pos++;
			shell_write_escape_sequence(shell, shell->codes.arrowr);
			ret = LINE_BUFF_OK;
		}

		break;

	case LINE_BUFF_ADD_STR:

		if(str_len <= shell->line_buff_size){

			/* Clear current terminal cmd. */
			shell_write_clearline(shell);
			shell->write("\r",1,shell->param);
			shell_write_prompt(shell);

			shell->line_buff_pos = 0;
			memset(shell->line_buff, 0, shell->line_buff_size);	// clear character buffer
			shell->line_buff[0] = '\0';

			memcpy(shell->line_buff,str,str_len);
			shell->line_buff_pos +=str_len;
			shell->write(str,str_len,shell->param);
		}

		break;


	default:
		ret = LINE_BUFF_UNSPECIFIED_CMD;
		break;

	}

	return ret;

}

static void shell_write_newline(shell_t* shell) {
	shell->write(shell->codes.newline, strlen(shell->codes.newline),shell->param);
	//shell->write(shell->line_prefix, strlen(shell->line_prefix));

}

static void shell_write_prompt(shell_t* shell) {
	shell->write(shell->line_prompt, strlen(shell->line_prompt),shell->param);

}

static void shell_write_escape_sequence(shell_t* shell,const char* byte) {

	char buff[4] = { *shell_start_of_escape_seq, '[', 0,'\0' };

	buff[2] = *byte;
	shell->write(buff, sizeof(buff),shell->param);
}
static void shell_write_clearline(shell_t* shell) {

	char buff[5] = { *shell_start_of_escape_seq, '[', *shell_CLEAR_LINE,*(shell_CLEAR_LINE+1),'\0' };

	shell->write(buff, sizeof(buff),shell->param);
}

static void shell_NL_Action(shell_t* shell)
{
	shell_write_newline(shell);

	/* If line buffer is empty it means that user typed only enter without providing command */
	if(strcmp(shell_ReturnLineBuff(shell),"\0") != 0){
		/* Add typed cmd into history list only if it is new element
		 * (we don't want to have several same commands stored in history list) */
		history_list_AddBeg(&history,shell_ReturnLineBuff(shell));

		BaseType_t ret;
		do
		{
			ret = FreeRTOS_CLIProcessCommand(shell_ReturnLineBuff(shell),response_output_buff,SHELL_MAX_OUTPUT_BUFFER_SIZE);

			/* Output cmd response if any */
			if(response_output_buff[0] != '\0'){
				shell->write(response_output_buff, strlen(response_output_buff),shell->param);
				shell_write_newline(shell);
			}
		}while(ret != pdFALSE );

	}

	shell_write_prompt(shell);
	shell_MngtLineBuff(shell, NULL,0, LINE_BUFF_CLEAR_WHOLE);
}

/* When authorization is in progress do not echo typed keys.
 *  */
static void shell_auth_inprogress(shell_t* shell,char byte)
{
	static char password_buff[SHELL_MAX_PASSWORD_LEN] ={0};
	static uint16_t buff_pos = 0;
	char encrypted_pass[SHELL_MAX_PASSWORD_LEN] ={0};
	/* During login user pressed enter */
	if(byte == *shell->codes.newline){
		/* Check if password stored in internal buffer match one saved in NVM/RAM memory */
		encryptDecrypt(password_buff,encrypted_pass);

		/* password is already encrypted */
		if(memcmp(encrypted_pass,shell_password,strlen(shell_password)) == 0){
			/* User passed correct password */
			shell_write_newline(shell);
			shell_write_prompt(shell);
			auth_state = AUTH_PASSED;
		}
		else{

		 	/* Send prompt if passwords didn't match */
			shell_write_newline(shell);
			shell->write(shell_auth_prompt,strlen(shell_auth_prompt),shell->param);
		}

	}else{

		if ((byte >= 0x20) && (byte <= 0x7E))
		{
			if(buff_pos < SHELL_MAX_PASSWORD_LEN){
#if SHOW_PASSWORD == 1
				shell->write(&byte,1,shell->param);
#endif
				password_buff[buff_pos++] = byte;
				return;
			}else{

			 	/* Send prompt if passwords didn't match */
				shell_write_newline(shell);
				shell->write(shell_auth_prompt,strlen(shell_auth_prompt),shell->param);
			}
		}
	}

	/* Clear temporary buffers*/
	buff_pos = 0;
	memset(password_buff,0,sizeof(password_buff));
	memset(encrypted_pass,0,sizeof(encrypted_pass));

}

static void shell_auth_passed(shell_t* shell,char byte)
{
	static char escape_sequence_start = 0;
	/*
	 * Start of escape sequence
	 */
	if (byte == *shell_start_of_escape_seq){
		escape_sequence_start = 1;
	}
	else if ((escape_sequence_start == 1) && (byte == '[')) {
		// omit this byte
	}
	/*
	 * New Line
	 */
	else if (*shell->codes.newline == byte) {

		shell_NL_Action(shell);
	}
	/*
	 * Backspace
	 */
	else if (*shell->codes.bs == byte) {
		shell_MngtLineBuff(shell, NULL,0, LINE_BUFF_BS);
	}
	/*
	 * Arrow UP
	 */
	else if ((*shell->codes.arrowu == byte) && (escape_sequence_start == 1)) {
		escape_sequence_start = 0;

		const char* dptr = history_list_TraverseUp(&history);
		if(dptr != NULL){
			shell_MngtLineBuff(shell,dptr,strlen(dptr),LINE_BUFF_ADD_STR);
		}

	}
	/*
	 * Arrow DOWN
	 */
	else if ((*shell->codes.arrowd == byte) && (escape_sequence_start == 1)) {
		escape_sequence_start = 0;

		const char* dptr = history_list_TraverseDown(&history);
		if(dptr != NULL){
			shell_MngtLineBuff(shell,dptr,strlen(dptr),LINE_BUFF_ADD_STR);
		}

	}
	/*
	 * Arrow LEFT
	 */
	else if ((*shell->codes.arrowl == byte) && (escape_sequence_start == 1)) {
		escape_sequence_start = 0;

		shell_MngtLineBuff(shell, NULL,0, LINE_BUFF_POS_LEFT);
	}
	/*
	 * Arrow RIGHT
	 */
	else if ((*shell->codes.arrowr == byte) && (escape_sequence_start == 1)) {
		escape_sequence_start = 0;

		shell_MngtLineBuff(shell, NULL,0, LINE_BUFF_POS_RIGHT);
	}
	/*
	 * All ascii characters available from standard keyboard
	 */
	else if ((byte >= 0x20) && (byte <= 0x7E))
	{
		if (shell_MngtLineBuff(shell, &byte,1, LINE_BUFF_ADD_CHAR) == LINE_BUFF_OK) {

			shell->write(&byte, sizeof(byte),shell->param);

		} else {
			// line buffer full, clear it and then proceed to next line
			shell_MngtLineBuff(shell, NULL,0, LINE_BUFF_CLEAR_WHOLE); // clears line buff
			shell_write_newline(shell);
			shell_write_prompt(shell);
		}
	}

}

void shell_RunPeriodic(shell_t* shell) {

	SHELL_ASSERT(shell != NULL);

	uint32_t len=0;

	/* Read method can be either blocking or non-blocking */
	shell->read(read_buffer,&len,shell->param);

	uint32_t i;

	/* Received block of data can be longer than 1 byte but no bigger than SHELL_READ_BUFFER_LEN.
	 * In this case split entire buffer into separate characters and process them one by one. */
	for(i=0;i<len;++i){

		if (read_buffer[i] != '\0') {

			switch(auth_state)
			{
			case AUTH_IN_PROGRESS:
				shell_auth_inprogress(shell,read_buffer[i]);
				break;
			case AUTH_PASSED:
				shell_auth_passed(shell,read_buffer[i]);
				break;
			}

		}
	}



}
