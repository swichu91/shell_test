/*
 * Some constants, hardware definitions and comments taken from ST's HAL driver
 * library, COPYRIGHT(c) 2015 STMicroelectronics.
 */

 /*
 * FreeRTOS+FAT Labs Build 150825 (C) 2015 Real Time Engineers ltd.
 * Authors include James Walmsley, Hein Tibosch and Richard Barry
 *
 *******************************************************************************
 ***** NOTE ******* NOTE ******* NOTE ******* NOTE ******* NOTE ******* NOTE ***
 ***                                                                         ***
 ***                                                                         ***
 ***   FREERTOS+FAT IS STILL IN THE LAB:                                     ***
 ***                                                                         ***
 ***   This product is functional and is already being used in commercial    ***
 ***   products.  Be aware however that we are still refining its design,    ***
 ***   the source code does not yet fully conform to the strict coding and   ***
 ***   style standards mandated by Real Time Engineers ltd., and the         ***
 ***   documentation and testing is not necessarily complete.                ***
 ***                                                                         ***
 ***   PLEASE REPORT EXPERIENCES USING THE SUPPORT RESOURCES FOUND ON THE    ***
 ***   URL: http://www.FreeRTOS.org/contact  Active early adopters may, at   ***
 ***   the sole discretion of Real Time Engineers Ltd., be offered versions  ***
 ***   under a license other than that described below.                      ***
 ***                                                                         ***
 ***                                                                         ***
 ***** NOTE ******* NOTE ******* NOTE ******* NOTE ******* NOTE ******* NOTE ***
 *******************************************************************************
 *
 * - Open source licensing -
 * While FreeRTOS+FAT is in the lab it is provided only under version two of the
 * GNU General Public License (GPL) (which is different to the standard FreeRTOS
 * license).  FreeRTOS+FAT is free to download, use and distribute under the
 * terms of that license provided the copyright notice and this text are not
 * altered or removed from the source files.  The GPL V2 text is available on
 * the gnu.org web site, and on the following
 * URL: http://www.FreeRTOS.org/gpl-2.0.txt.  Active early adopters may, and
 * solely at the discretion of Real Time Engineers Ltd., be offered versions
 * under a license other then the GPL.
 *
 * FreeRTOS+FAT is distributed in the hope that it will be useful.  You cannot
 * use FreeRTOS+FAT unless you agree that you use the software 'as is'.
 * FreeRTOS+FAT is provided WITHOUT ANY WARRANTY; without even the implied
 * warranties of NON-INFRINGEMENT, MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE. Real Time Engineers Ltd. disclaims all conditions and terms, be they
 * implied, expressed, or statutory.
 *
 * 1 tab == 4 spaces!
 *
 * http://www.FreeRTOS.org
 * http://www.FreeRTOS.org/plus
 * http://www.FreeRTOS.org/labs
 *
 */

/* Standard includes. */
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "portmacro.h"

/* FreeRTOS+FAT includes. */
#include "ff_sddisk.h"
#include "ff_sys.h"

/* ST HAL includes. */
#include "stm32746g_discovery_sd.h"

/* Misc definitions. */
#define sdSIGNATURE 			0x41404342UL
#define sdHUNDRED_64_BIT		( 100ull )
#define sdBYTES_PER_MB			( 1024ull * 1024ull )
#define sdSECTORS_PER_MB		( sdBYTES_PER_MB / 512ull )
#define sdIOMAN_MEM_SIZE		4096

/* TODO: needs more testing!! */
#define SDIO_USES_DMA 0

/* Define a time-out for all DMA transactions in msec. */
#ifndef sdMAX_TIME_TICKS
	#define sdMAX_TIME_TICKS	pdMS_TO_TICKS( 2000UL )
#endif

#ifndef sdCARD_DETECT_DEBOUNCE_TIME_MS
	/* Debouncing time is applied only after card gets inserted. */
	#define sdCARD_DETECT_DEBOUNCE_TIME_MS	( 5000 )
#endif

#ifndef sdARRAY_SIZE
	#define	sdARRAY_SIZE( x )	( int )( sizeof( x ) / sizeof( x )[ 0 ] )
#endif

/*-----------------------------------------------------------*/


 #define DIFF_SEC_1900_1970         (2208988800UL)
time_t FreeRTOS_time( time_t *pxTime ){
	return DIFF_SEC_1900_1970;
}

/*
 * Translate a numeric code like 'SD_TX_UNDERRUN' to a printable string.
 */
static const char *prvSDCodePrintable( uint32_t ulCode );

/*
 * The following 'hook' must be provided by the user of this module.  It will be
 * called from a GPIO ISR after every change.  Note that during the ISR, the
 * value of the GPIO is not stable and it can not be used.  All you can do from
 * this hook is wake-up some task, which will call FF_SDDiskDetect().
 */
extern void vApplicationCardDetectChangeHookFromISR( BaseType_t *pxHigherPriorityTaskWoken );

/*
 * Check if the card is present, and if so, print out some info on the card.
 */
static BaseType_t prvSDMMCInit( BaseType_t xDriveNumber );

/*-----------------------------------------------------------*/

typedef struct
{
	/* Only after a card has been inserted, debouncing is necessary. */
	TickType_t xRemainingTime;
	TimeOut_t xTimeOut;
	UBaseType_t
		bLastPresent : 1,
		bStableSignal : 1;
} CardDetect_t;


/* Used to unblock the task that calls prvEventWaitFunction() after an event has
occurred. */
static SemaphoreHandle_t xSDCardSemaphore = NULL;

/* Handle of the SD card being used. */
static SD_HandleTypeDef* xSDHandle;

/* Holds parameters for the detected SD card. */
static HAL_SD_CardInfoTypedef xSDCardInfo;

/* Mutex for partition. */
static SemaphoreHandle_t xPlusFATMutex = NULL;

/* Remembers if the card is currently considered to be present. */
static BaseType_t xSDCardStatus = pdFALSE;

/* Maintains state for card detection. */
static CardDetect_t xCardDetect;

/*-----------------------------------------------------------*/

static int32_t prvFFRead( uint8_t *pucBuffer, uint32_t ulSectorNumber, uint32_t ulSectorCount, FF_Disk_t *pxDisk )
{
int32_t lReturnCode = FF_ERR_IOMAN_OUT_OF_BOUNDS_READ | FF_ERRFLAG;

	if( ( pxDisk != NULL ) &&
		( xSDCardStatus == pdPASS ) &&
		( pxDisk->ulSignature == sdSIGNATURE ) &&
		( pxDisk->xStatus.bIsInitialised != pdFALSE ) &&
		( ulSectorNumber < pxDisk->ulNumberOfSectors ) &&
		( ( pxDisk->ulNumberOfSectors - ulSectorNumber ) >= ulSectorCount ) )
	{
	uint64_t ullReadAddr;
	HAL_SD_ErrorTypedef sd_result;

		ullReadAddr = 512ull * ( uint64_t ) ulSectorNumber;

		#if( SDIO_USES_DMA == 0 )
		{
			sd_result = HAL_SD_ReadBlocks( xSDHandle, (uint32_t *) pucBuffer, ullReadAddr, 512ul, ulSectorCount );
		}
		#else
		{
			if( ( ( ( size_t )pucBuffer ) & ( sizeof( size_t ) - 1 ) ) == 0 )
			{
				/* The buffer is word-aligned, call DMA read directly. */
				sd_result = HAL_SD_ReadBlocks_DMA( xSDHandle, (uint32_t *) pucBuffer, ullReadAddr, 512ul, ulSectorCount);
				if( sd_result == SD_OK )
				{
					sd_result = HAL_SD_CheckReadOperation( xSDHandle, sdMAX_TIME_TICKS );
				}
			}
			else
			{
			uint32_t ulSector;
			uint8_t *pucDMABuffer = ffconfigMALLOC( 512ul );

				/* The buffer is NOT word-aligned, copy first to an aligned buffer. */
				if( pucDMABuffer != NULL )
				{
					sd_result = SD_OK;
					for( ulSector = 0; ulSector < ulSectorCount; ulSector++ )
					{
						ullReadAddr = 512ull * ( ( uint64_t ) ulSectorNumber + ( uint64_t ) ulSector );
						sd_result = HAL_SD_ReadBlocks_DMA( xSDHandle, ( uint32_t * )pucDMABuffer, ullReadAddr, 512ul, 1 );

						if( sd_result == SD_OK )
						{
							sd_result = HAL_SD_CheckReadOperation( xSDHandle, sdMAX_TIME_TICKS );
							if( sd_result != SD_OK )
							{
								break;
							}
							memcpy( pucBuffer + 512ul * ulSector, pucDMABuffer, 512ul );
						}
					}
					ffconfigFREE( pucDMABuffer );
				}
				else
				{
					sd_result = SD_INVALID_PARAMETER;
				}
			}
		}
		#endif	/* SDIO_USES_DMA */

		if( sd_result == SD_OK )
		{
			lReturnCode = 0L;
		}
		else
		{
			/* Some error occurred. */
			FF_PRINTF( "prvFFRead: %lu: %lu (%s)\n", ulSectorNumber, sd_result, prvSDCodePrintable( sd_result ) );
		}
	}
	else
	{
		/* Make sure no random data is in the returned buffer. */
		memset( ( void * ) pucBuffer, '\0', ulSectorCount * 512UL );

		if( pxDisk->xStatus.bIsInitialised != pdFALSE )
		{
			FF_PRINTF( "prvFFRead: warning: %lu + %lu > %lu\n", ulSectorNumber, ulSectorCount, pxDisk->ulNumberOfSectors );
		}
	}

	return lReturnCode;
}
/*-----------------------------------------------------------*/

static int32_t prvFFWrite( uint8_t *pucBuffer, uint32_t ulSectorNumber, uint32_t ulSectorCount, FF_Disk_t *pxDisk )
{
int32_t lReturnCode = FF_ERR_IOMAN_OUT_OF_BOUNDS_READ | FF_ERRFLAG;

	if( ( pxDisk != NULL ) &&
		( xSDCardStatus == pdPASS ) &&
		( pxDisk->ulSignature == sdSIGNATURE ) &&
		( pxDisk->xStatus.bIsInitialised != pdFALSE ) &&
		( ulSectorNumber < pxDisk->ulNumberOfSectors ) &&
		( ( pxDisk->ulNumberOfSectors - ulSectorNumber ) >= ulSectorCount ) )
	{
		HAL_SD_ErrorTypedef sd_result;
		uint64_t ullWriteAddr;
		ullWriteAddr = 512ull * ulSectorNumber;

		#if( SDIO_USES_DMA == 0 )
		{
			sd_result = HAL_SD_WriteBlocks( xSDHandle, ( uint32_t * )pucBuffer, ullWriteAddr, 512ul, ulSectorCount );
		}
		#else
		{
			if( ( ( ( size_t )pucBuffer ) & ( sizeof( size_t ) - 1 ) ) == 0 )
			{
				/* The buffer is word-aligned, call DMA reawrite directly. */
				sd_result = HAL_SD_WriteBlocks_DMA( xSDHandle, ( uint32_t * )pucBuffer, ullWriteAddr, 512ul, ulSectorCount );
				if( sd_result == SD_OK )
				{
					sd_result = HAL_SD_CheckWriteOperation( xSDHandle, sdMAX_TIME_TICKS );
				}
			}
			else
			{
			uint32_t ulSector;
			uint8_t *pucDMABuffer = ffconfigMALLOC( 512ul );

				/* The buffer is NOT word-aligned, read to an aligned buffer and then
				copy the data to the user provided buffer. */
				if( pucDMABuffer != NULL )
				{
					sd_result = SD_OK;
					for( ulSector = 0; ulSector < ulSectorCount; ulSector++ )
					{
						memcpy( pucDMABuffer, pucBuffer + 512ul * ulSector, 512ul );
						ullWriteAddr = 512ull * ( ulSectorNumber + ulSector );
						sd_result = HAL_SD_WriteBlocks_DMA( xSDHandle, ( uint32_t * )pucDMABuffer, ullWriteAddr, 512ul, 1 );
						if( sd_result == SD_OK )
						{
							sd_result = HAL_SD_CheckWriteOperation( xSDHandle, sdMAX_TIME_TICKS );
							if( sd_result != SD_OK )
							{
								break;
							}
						}
					}
					ffconfigFREE( pucDMABuffer );
				}
				else
				{
					sd_result = SD_INVALID_PARAMETER;
				}
			}
		}
		#endif	/* SDIO_USES_DMA */

		if( sd_result == SD_OK )
		{
			/* No errors. */
			lReturnCode = 0L;
		}
		else
		{
			FF_PRINTF( "prvFFWrite: %lu: %lu (%s)\n", ulSectorNumber, sd_result, prvSDCodePrintable( sd_result ) );
		}
	}
	else
	{
		if( pxDisk->xStatus.bIsInitialised != pdFALSE )
		{
			FF_PRINTF( "prvFFWrite: warning: %lu + %lu > %lu\n", ulSectorNumber, ulSectorCount, pxDisk->ulNumberOfSectors );
		}
	}

	return lReturnCode;
}
/*-----------------------------------------------------------*/

void FF_SDDiskFlush( FF_Disk_t *pxDisk )
{
	if( ( pxDisk != NULL ) &&
		( pxDisk->xStatus.bIsInitialised != pdFALSE ) &&
		( pxDisk->pxIOManager != NULL ) )
	{
		FF_FlushCache( pxDisk->pxIOManager );
	}
}
/*-----------------------------------------------------------*/

FF_Disk_t *FF_SDDiskInit( const char *pcName )
{
FF_Error_t xFFError;
BaseType_t xPartitionNumber = 0;
FF_CreationParameters_t xParameters;
FF_Disk_t *pxDisk;

	xSDCardStatus = prvSDMMCInit( 0 );

	if( xSDCardStatus != pdPASS )
	{
		FF_PRINTF( "FF_SDDiskInit: prvSDMMCInit failed\n" );
		pxDisk = NULL;
	}
	else
	{
		pxDisk = (FF_Disk_t *)ffconfigMALLOC( sizeof( *pxDisk ) );
		if( pxDisk == NULL )
		{
			FF_PRINTF( "FF_SDDiskInit: Malloc failed\n" );
		}
		else
		{
			/* Initialise the created disk structure. */
			memset( pxDisk, '\0', sizeof( *pxDisk ) );

			pxDisk->ulNumberOfSectors = xSDCardInfo.CardCapacity / 512;

			if( xPlusFATMutex == NULL )
			{
				xPlusFATMutex = xSemaphoreCreateRecursiveMutex();
			}
			pxDisk->ulSignature = sdSIGNATURE;

			if( xPlusFATMutex != NULL)
			{
				memset( &xParameters, '\0', sizeof( xParameters ) );
				xParameters.ulMemorySize = sdIOMAN_MEM_SIZE;
				xParameters.ulSectorSize = 512;
				xParameters.fnWriteBlocks = prvFFWrite;
				xParameters.fnReadBlocks = prvFFRead;
				xParameters.pxDisk = pxDisk;

				/* prvFFRead()/prvFFWrite() are not re-entrant and must be
				protected with the use of a semaphore. */
				xParameters.xBlockDeviceIsReentrant = pdFALSE;

				/* The semaphore will be used to protect critical sections in
				the +FAT driver, and also to avoid concurrent calls to
				prvFFRead()/prvFFWrite() from different tasks. */
				xParameters.pvSemaphore = ( void * ) xPlusFATMutex;

				pxDisk->pxIOManager = FF_CreateIOManger( &xParameters, &xFFError );

				if( pxDisk->pxIOManager == NULL )
				{
					FF_PRINTF( "FF_SDDiskInit: FF_CreateIOManger: %s\n", (const char*)FF_GetErrMessage( xFFError ) );
					FF_SDDiskDelete( pxDisk );
					pxDisk = NULL;
				}
				else
				{
					pxDisk->xStatus.bIsInitialised = pdTRUE;
					pxDisk->xStatus.bPartitionNumber = xPartitionNumber;
					if( FF_SDDiskMount( pxDisk ) == 0 )
					{
						FF_SDDiskDelete( pxDisk );
						pxDisk = NULL;
					}
					else
					{
						if( pcName == NULL )
						{
							pcName = "/";
						}
						FF_FS_Add( pcName, pxDisk );
						FF_PRINTF( "FF_SDDiskInit: Mounted SD-card as root \"%s\"\n", pcName );
						FF_SDDiskShowPartition( pxDisk );
					}
				}	/* if( pxDisk->pxIOManager != NULL ) */
			}	/* if( xPlusFATMutex != NULL) */
		}	/* if( pxDisk != NULL ) */
	}	/* if( xSDCardStatus == pdPASS ) */

	return pxDisk;
}
/*-----------------------------------------------------------*/

BaseType_t FF_SDDiskFormat( FF_Disk_t *pxDisk, BaseType_t xPartitionNumber )
{
FF_Error_t xError;
BaseType_t xReturn = pdFAIL;

	xError = FF_Unmount( pxDisk );

	if( FF_isERR( xError ) != pdFALSE )
	{
		FF_PRINTF( "FF_SDDiskFormat: unmount fails: %08x\n", ( unsigned ) xError );
	}
	else
	{
		/* Format the drive - try FAT32 with large clusters. */
		xError = FF_Format( pxDisk, xPartitionNumber, pdFALSE, pdFALSE);

		if( FF_isERR( xError ) )
		{
			FF_PRINTF( "FF_SDDiskFormat: %s\n", (const char*)FF_GetErrMessage( xError ) );
		}
		else
		{
			FF_PRINTF( "FF_SDDiskFormat: OK, now remounting\n" );
			pxDisk->xStatus.bPartitionNumber = xPartitionNumber;
			xError = FF_SDDiskMount( pxDisk );
			FF_PRINTF( "FF_SDDiskFormat: rc %08x\n", ( unsigned )xError );
			if( FF_isERR( xError ) == pdFALSE )
			{
				xReturn = pdPASS;
				FF_SDDiskShowPartition( pxDisk );
			}
		}
	}

	return xReturn;
}
/*-----------------------------------------------------------*/

BaseType_t FF_SDDiskUnmount( FF_Disk_t *pxDisk )
{
FF_Error_t xFFError;
BaseType_t xReturn = pdPASS;

	if( ( pxDisk != NULL ) && ( pxDisk->xStatus.bIsMounted != pdFALSE ) )
	{
		pxDisk->xStatus.bIsMounted = pdFALSE;
		xFFError = FF_Unmount( pxDisk );

		if( FF_isERR( xFFError ) )
		{
			FF_PRINTF( "FF_SDDiskUnmount: rc %08x\n", ( unsigned )xFFError );
			xReturn = pdFAIL;
		}
		else
		{
			FF_PRINTF( "Drive unmounted\n" );
		}
	}

	return xReturn;
}
/*-----------------------------------------------------------*/

BaseType_t FF_SDDiskReinit( FF_Disk_t *pxDisk )
{
BaseType_t xStatus = prvSDMMCInit( 0 ); /* Hard coded index. */

	/*_RB_ parameter not used. */
	( void ) pxDisk;

	FF_PRINTF( "FF_SDDiskReinit: rc %08x\n", ( unsigned ) xStatus );
	return xStatus;
}
/*-----------------------------------------------------------*/

BaseType_t FF_SDDiskMount( FF_Disk_t *pxDisk )
{
FF_Error_t xFFError;
BaseType_t xReturn;

	/* Mount the partition */
	xFFError = FF_Mount( pxDisk, pxDisk->xStatus.bPartitionNumber );

	if( FF_isERR( xFFError ) )
	{
		FF_PRINTF( "FF_SDDiskMount: %08lX\n", xFFError );
		xReturn = pdFAIL;
	}
	else
	{
		pxDisk->xStatus.bIsMounted = pdTRUE;
		FF_PRINTF( "****** FreeRTOS+FAT initialized %lu sectors\n", pxDisk->pxIOManager->xPartition.ulTotalSectors );
		xReturn = pdPASS;
	}

	return xReturn;
}
/*-----------------------------------------------------------*/

FF_IOManager_t *sddisk_ioman( FF_Disk_t *pxDisk )
{
FF_IOManager_t *pxReturn;

	if( ( pxDisk != NULL ) && ( pxDisk->xStatus.bIsInitialised != pdFALSE ) )
	{
		pxReturn = pxDisk->pxIOManager;
	}
	else
	{
		pxReturn = NULL;
	}
	return pxReturn;
}
/*-----------------------------------------------------------*/

/* Release all resources */
BaseType_t FF_SDDiskDelete( FF_Disk_t *pxDisk )
{
	if( pxDisk != NULL )
	{
		pxDisk->ulSignature = 0;
		pxDisk->xStatus.bIsInitialised = 0;
		if( pxDisk->pxIOManager != NULL )
		{
			if( FF_Mounted( pxDisk->pxIOManager ) != pdFALSE )
			{
				FF_Unmount( pxDisk );
			}
			FF_DeleteIOManager( pxDisk->pxIOManager );
		}

		vPortFree( pxDisk );
	}
	return 1;
}
/*-----------------------------------------------------------*/

BaseType_t FF_SDDiskShowPartition( FF_Disk_t *pxDisk )
{
FF_Error_t xError;
uint64_t ullFreeSectors;
uint32_t ulTotalSizeMB, ulFreeSizeMB;
int iPercentageFree;
FF_IOManager_t *pxIOManager;
const char *pcTypeName = "unknown type";
BaseType_t xReturn = pdPASS;

	if( pxDisk == NULL )
	{
		xReturn = pdFAIL;
	}
	else
	{
		pxIOManager = pxDisk->pxIOManager;

		FF_PRINTF( "Reading FAT and calculating Free Space\n" );

		switch( pxIOManager->xPartition.ucType )
		{
			case FF_T_FAT12:
				pcTypeName = "FAT12";
				break;

			case FF_T_FAT16:
				pcTypeName = "FAT16";
				break;

			case FF_T_FAT32:
				pcTypeName = "FAT32";
				break;

			default:
				pcTypeName = "UNKOWN";
				break;
		}

		FF_GetFreeSize( pxIOManager, &xError );

		ullFreeSectors = pxIOManager->xPartition.ulFreeClusterCount * pxIOManager->xPartition.ulSectorsPerCluster;
		iPercentageFree = ( int ) ( ( sdHUNDRED_64_BIT * ullFreeSectors + pxIOManager->xPartition.ulDataSectors / 2 ) /
			( ( uint64_t )pxIOManager->xPartition.ulDataSectors ) );

		ulTotalSizeMB = pxIOManager->xPartition.ulDataSectors / sdSECTORS_PER_MB;
		ulFreeSizeMB = ( uint32_t ) ( ullFreeSectors / sdSECTORS_PER_MB );

		/* It is better not to use the 64-bit format such as %Lu because it
		might not be implemented. */
		FF_PRINTF( "Partition Nr   %8u\n", pxDisk->xStatus.bPartitionNumber );
		FF_PRINTF( "Type           %8u (%s)\n", pxIOManager->xPartition.ucType, pcTypeName );
		FF_PRINTF( "VolLabel       '%8s' \n", pxIOManager->xPartition.pcVolumeLabel );
		FF_PRINTF( "TotalSectors   %8lu\n", pxIOManager->xPartition.ulTotalSectors );
		FF_PRINTF( "SecsPerCluster %8lu\n", pxIOManager->xPartition.ulSectorsPerCluster );
		FF_PRINTF( "Size           %8lu MB\n", ulTotalSizeMB );
		FF_PRINTF( "FreeSize       %8lu MB ( %d perc free )\n", ulFreeSizeMB, iPercentageFree );
	}

	return xReturn;
}
/*-----------------------------------------------------------*/

/* This routine returns true if the SD-card is inserted.  After insertion, it
will wait for sdCARD_DETECT_DEBOUNCE_TIME_MS before returning pdTRUE. */
BaseType_t FF_SDDiskDetect( FF_Disk_t *pxDisk )
{
int xReturn;
(void)pxDisk;

	xReturn = BSP_SD_IsDetected();

	if( xReturn != pdFALSE )
	{
		if( xCardDetect.bStableSignal == pdFALSE )
		{
			/* The card seems to be present. */
			if( xCardDetect.bLastPresent == pdFALSE )
			{
				xCardDetect.bLastPresent = pdTRUE;
				xCardDetect.xRemainingTime = pdMS_TO_TICKS( ( TickType_t ) sdCARD_DETECT_DEBOUNCE_TIME_MS );
				/* Fetch the current time. */
				vTaskSetTimeOutState( &xCardDetect.xTimeOut );
			}
			/* Has the timeout been reached? */
			if( xTaskCheckForTimeOut( &xCardDetect.xTimeOut, &xCardDetect.xRemainingTime ) != pdFALSE )
			{
				xCardDetect.bStableSignal = pdTRUE;
			}
			else
			{
				/* keep returning false until de time-out is reached. */
				xReturn = pdFALSE;
			}
		}
	}
	else
	{
		xCardDetect.bLastPresent = pdFALSE;
		xCardDetect.bStableSignal = pdFALSE;
	}

	return xReturn;
}
/*-----------------------------------------------------------*/


/*-----------------------------------------------------------*/

static BaseType_t prvSDMMCInit( BaseType_t xDriveNumber )
{
	/* 'xDriveNumber' not yet in use. */
	( void )xDriveNumber;

	int SD_state = SD_OK;
	HAL_SD_CardInfoTypedef SDCardInfo;

	if( xSDCardSemaphore == NULL )
	{
		xSDCardSemaphore = xSemaphoreCreateBinary();
	}
	SD_state = BSP_SD_Init(&xSDHandle);

	/* Check if the SD card is plugged in the slot */
	if(BSP_SD_IsDetected() == SD_PRESENT){
	  BSP_SD_GetCardInfo(&SDCardInfo);
	}else{
		FF_PRINTF( "No SD card detected\n" );
		return 0;
	}

	/* When starting up, skip debouncing of the Card Detect signal. */
	xCardDetect.bLastPresent = pdTRUE;
	xCardDetect.bStableSignal = pdTRUE;
	xSDCardInfo.CardCapacity = SDCardInfo.CardCapacity;

	FF_PRINTF( "HAL_SD_Init: %d: %s type: %s Capacity: %lu MB\n",
		SD_state, prvSDCodePrintable( ( uint32_t )SD_state ),
		SDCardInfo.CardType == HIGH_CAPACITY_SD_CARD ? "SDHC" : "SD",
		SDCardInfo.CardCapacity / ( 1024 * 1024 ) );

	return SD_state == SD_OK ? 1 : 0;
}
/*-----------------------------------------------------------*/

struct xCODE_NAME
{
	uint32_t ulValue;
	const char *pcName;
};

const struct xCODE_NAME xSD_CODES[] =
{
	{ SD_CMD_CRC_FAIL,          "CMD_CRC_FAIL: Command response received (but CRC check failed)" },
	{ SD_DATA_CRC_FAIL,         "DATA_CRC_FAIL: Data block sent/received (CRC check failed)" },
	{ SD_CMD_RSP_TIMEOUT,       "CMD_RSP_TIMEOUT: Command response timeout" },
	{ SD_DATA_TIMEOUT,          "DATA_TIMEOUT: Data timeout" },
	{ SD_TX_UNDERRUN,           "TX_UNDERRUN: Transmit FIFO underrun" },
	{ SD_RX_OVERRUN,            "RX_OVERRUN: Receive FIFO overrun" },
	{ SD_START_BIT_ERR,         "START_BIT_ERR: Start bit not detected on all data signals in wide bus mode" },
	{ SD_CMD_OUT_OF_RANGE,      "CMD_OUT_OF_RANGE: Command's argument was out of range" },
	{ SD_ADDR_MISALIGNED,       "ADDR_MISALIGNED: Misaligned address" },
	{ SD_BLOCK_LEN_ERR,         "BLOCK_LEN_ERR: Transferred block length is not allowed for the card or the number of transferred bytes does not match the block length" },
	{ SD_ERASE_SEQ_ERR,         "ERASE_SEQ_ERR: An error in the sequence of erase command occurs." },
	{ SD_BAD_ERASE_PARAM,       "BAD_ERASE_PARAM: An invalid selection for erase groups" },
	{ SD_WRITE_PROT_VIOLATION,  "WRITE_PROT_VIOLATION: Attempt to program a write protect block" },
	{ SD_LOCK_UNLOCK_FAILED,    "LOCK_UNLOCK_FAILED: Sequence or password error has been detected in unlock command or if there was an attempt to access a locked card" },
	{ SD_COM_CRC_FAILED,        "COM_CRC_FAILED: CRC check of the previous command failed" },
	{ SD_ILLEGAL_CMD,           "ILLEGAL_CMD: Command is not legal for the card state" },
	{ SD_CARD_ECC_FAILED,       "CARD_ECC_FAILED: Card internal ECC was applied but failed to correct the data" },
	{ SD_CC_ERROR,              "CC_ERROR: Internal card controller error" },
	{ SD_GENERAL_UNKNOWN_ERROR, "GENERAL_UNKNOWN_ERROR: General or unknown error" },
	{ SD_STREAM_READ_UNDERRUN,  "STREAM_READ_UNDERRUN: The card could not sustain data transfer in stream read operation" },
	{ SD_STREAM_WRITE_OVERRUN,  "STREAM_WRITE_OVERRUN: The card could not sustain data programming in stream mode" },
	{ SD_CID_CSD_OVERWRITE,     "CID_CSD_OVERWRITE: CID/CSD overwrite error" },
	{ SD_WP_ERASE_SKIP,         "WP_ERASE_SKIP: Only partial address space was erased" },
	{ SD_CARD_ECC_DISABLED,     "CARD_ECC_DISABLED: Command has been executed without using internal ECC" },
	{ SD_ERASE_RESET,           "ERASE_RESET: Erase sequence was cleared before executing because an out of erase sequence command was received" },
	{ SD_AKE_SEQ_ERROR,         "AKE_SEQ_ERROR: Error in sequence of authentication" },
	{ SD_INVALID_VOLTRANGE,     "INVALID_VOLTRANGE" },
	{ SD_ADDR_OUT_OF_RANGE,     "ADDR_OUT_OF_RANGE" },
	{ SD_SWITCH_ERROR,          "SWITCH_ERROR" },
	{ SD_SDIO_DISABLED,         "SDIO_DISABLED" },
	{ SD_SDIO_FUNCTION_BUSY,    "SDIO_FUNCTION_BUSY" },
	{ SD_SDIO_FUNCTION_FAILED,  "SDIO_FUNCTION_FAILED" },
	{ SD_SDIO_UNKNOWN_FUNCTION, "SDIO_UNKNOWN_FUNCTION" },

	/**
	* @brief  Standard error defines
	*/
	{ SD_INTERNAL_ERROR,        "INTERNAL_ERROR" },
	{ SD_NOT_CONFIGURED,        "NOT_CONFIGURED" },
	{ SD_REQUEST_PENDING,       "REQUEST_PENDING" },
	{ SD_REQUEST_NOT_APPLICABLE,"REQUEST_NOT_APPLICABLE" },
	{ SD_INVALID_PARAMETER,     "INVALID_PARAMETER" },
	{ SD_UNSUPPORTED_FEATURE,   "UNSUPPORTED_FEATURE" },
	{ SD_UNSUPPORTED_HW,        "UNSUPPORTED_HW" },
	{ SD_ERROR,                 "ERROR" },
	{ SD_OK,                    "OK" },
};
/*-----------------------------------------------------------*/

static const char *prvSDCodePrintable( uint32_t ulCode )
{
static char retString[32];
const struct xCODE_NAME *pxCode;

	for( pxCode = xSD_CODES; pxCode <= xSD_CODES + sdARRAY_SIZE( xSD_CODES ) - 1; pxCode++ )
	{
		if( pxCode->ulValue == ulCode )
		{
			return pxCode->pcName;
		}
	}
	snprintf( retString, sizeof( retString ), "SD code %lu\n", ulCode );
	return retString;
}
/*
void HAL_GPIO_EXTI_Callback( uint16_t GPIO_Pin )
{
BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	if( GPIO_Pin == configSD_DETECT_PIN )
	{
		vApplicationCardDetectChangeHookFromISR( &xHigherPriorityTaskWoken );
		portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
	}
}
-----------------------------------------------------------

void EXTI15_10_IRQHandler( void )
{
	HAL_GPIO_EXTI_IRQHandler( configSD_DETECT_PIN );	 GPIO PIN H.13
}*/
/*-----------------------------------------------------------*/
