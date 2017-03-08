/*
 * fs.c
 *
 *  Created on: 8 mar 2017
 *      Author: Mateusz
 */

#include "fs.h"
#include <string.h>
#include <stdio.h>
#include "ff_headers.h"
#include "ff_stdio.h"
#include "shell/shell.h"

#include "FreeRTOS.h"
#include "task.h"

static BaseType_t cwd_cmd_callback( char *pcWriteBuffer, size_t xWriteBufferLen, argv arg, size_t argc );
static BaseType_t ls_cmd_callback( char *pcWriteBuffer, size_t xWriteBufferLen, argv arg, size_t argc );
static BaseType_t cat_cmd_callback( char *pcWriteBuffer, size_t xWriteBufferLen, argv arg, size_t argc );
static BaseType_t cd_cmd_callback( char *pcWriteBuffer, size_t xWriteBufferLen, argv arg, size_t argc );

BaseType_t ls_cmd_callback( char *pcWriteBuffer, size_t xWriteBufferLen, argv arg, size_t argc ) {

	(void)xWriteBufferLen;
	(void) argc;
	(void) arg;

	FF_FindData_t *pxFindStruct;
	const char *pcAttrib;
	const char *pcWritableFile = "writable file";
	const char *pcReadOnlyFile = "read only file";
	const char *pcDirectory = "directory";
	char cwd_buffer[64]; // to jest slabe !

	ff_getcwd(cwd_buffer,sizeof(cwd_buffer));

	/* FF_FindData_t can be large, so it is best to allocate the structure
	 dynamically, rather than declare it as a stack variable. */
	pxFindStruct = (FF_FindData_t *) pvPortMalloc(sizeof(FF_FindData_t));

	/* FF_FindData_t must be cleared to 0. */
	memset(pxFindStruct, 0x00, sizeof(FF_FindData_t));

	strcat(pcWriteBuffer,cwd_buffer);
	strcat(pcWriteBuffer,"\r\n");

	/* The first parameter to ff_findfist() is the directory being searched.  Do
	 not add wildcards to the end of the directory name. */
	if (ff_findfirst(cwd_buffer, pxFindStruct) == 0) {
		do {
			/* Point pcAttrib to a string that describes the file. */
			if ((pxFindStruct->ucAttributes & FF_FAT_ATTR_DIR) != 0) {
				pcAttrib = pcDirectory;
			}
			else if (pxFindStruct->ucAttributes & FF_FAT_ATTR_READONLY) {
				pcAttrib = pcReadOnlyFile;
			}
			else {
				pcAttrib = pcWritableFile;
			}
			char buff[128];
			/* Print the files name, size, and attribute string. */
			sprintf(buff,
					 "%s [%s] [size=%d]\r\n", pxFindStruct->pcFileName, pcAttrib, (int)pxFindStruct->ulFileSize );
			strcat(pcWriteBuffer,buff);
		}
		while (ff_findnext(pxFindStruct) == 0);
	}

	/* Free the allocated FF_FindData_t structure. */
	vPortFree(pxFindStruct);

	return pdFALSE;
}



BaseType_t cat_cmd_callback( char *pcWriteBuffer, size_t xWriteBufferLen, argv arg, size_t argc ) {

	(void)xWriteBufferLen;
	(void) argc;

	FF_FILE* fd;
	char buff[64]= "/a/";

	strcat(buff,&arg[1][0]);

	fd = ff_fopen(buff,"r");

	if(fd != NULL){

		ff_fread(pcWriteBuffer,1,512,fd);
		strcat(pcWriteBuffer,"\r\n");

		ff_fclose(fd);
	}
	return pdFALSE;
}

BaseType_t cd_cmd_callback( char *pcWriteBuffer, size_t xWriteBufferLen, argv arg, size_t argc ) {

	(void)xWriteBufferLen;
	(void) argc;
	(void)pcWriteBuffer;

	char cwd_buffer[64]; // to jest slabe !

	ff_getcwd(cwd_buffer,sizeof(cwd_buffer));

	if(strcmp(&arg[1][0],"..") == 0){
		ff_chdir("..");
	}else if(strcmp(&arg[1][0],"/") == 0){
		ff_chdir("/");
	}
	else{

		if(strcmp(cwd_buffer,"/") !=0){
			strcat(cwd_buffer,"/");
		}
		strcat(cwd_buffer,&arg[1][0]);
		ff_chdir(cwd_buffer);
	}

	return pdFALSE;
}

BaseType_t cwd_cmd_callback( char *pcWriteBuffer, size_t xWriteBufferLen, argv arg, size_t argc ) {

	(void)xWriteBufferLen;
	(void) argc;
	(void) arg;

	ff_getcwd(pcWriteBuffer,sizeof(pcWriteBuffer));
	return pdFALSE;
}

shell_cmd_t fs_dir_cmd ={"ls","	ls ->  Prints file list in current directory",ls_cmd_callback,0};
shell_cmd_t fs_cat_cmd ={"cat","	cat ->  Prints file content",cat_cmd_callback,1};
shell_cmd_t fs_cd_cmd ={"cd","	cd ->  Changes directory",cd_cmd_callback,1};
shell_cmd_t fs_cwd_cmd ={"cwd","	cwd ->  Current working dir",cwd_cmd_callback,0};




void fs_RegisterCmds(void)
{
	shell_RegisterCmd(&fs_dir_cmd);
	shell_RegisterCmd(&fs_cat_cmd);
	shell_RegisterCmd(&fs_cd_cmd);
	shell_RegisterCmd(&fs_cwd_cmd);
}
