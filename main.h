/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

//#include <stdio.h>
#include "spi_sd.h"

uint8_t		SD_State = 0x00;

FATFS FATFS_Obj;

/* Exported functions ------------------------------------------------------- */
FRESULT scan_files (
    char* path,        /* Start node to be scanned (also used as work area) */
    int* numFiles,
    int pos
);

#endif /* __MAIN_H */
