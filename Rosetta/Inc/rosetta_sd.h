#ifndef ROSETTA_SD_H
#define ROSETTA_SD_H

#include "stm32f4xx_hal.h"
#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    ROSETTA_SD_NOT_RUN = 0,
    ROSETTA_SD_NO_RESPONSE,
    ROSETTA_SD_SDIO_ERROR,
    ROSETTA_SD_FATFS_ERROR,
    ROSETTA_SD_RW_ERROR,
    ROSETTA_SD_PASS
} RosettaSDResult;

typedef struct
{
    RosettaSDResult result;

    uint8_t card_present;
    uint8_t sdio_ok;
    uint8_t fatfs_ok;
    uint8_t rw_ok;

    uint32_t size_mb;
    uint32_t logical_block_size;
    uint8_t fatfs_result;
} RosettaSDData;

/*
 * Runs an end-to-end, non-destructive acceptance test:
 *   SDIO init -> 4-bit bus -> card info -> FATFS mount
 *   -> create temporary file -> write -> read -> compare -> delete.
 *
 * The test never overwrites an existing file: it uses FA_CREATE_NEW and
 * searches for a free ROSxx.TST filename, then deletes it after verification.
 *
 * IMPORTANT:
 * - CubeMX must generate SDIO + FATFS support.
 * - MX_FATFS_Init() must have been called once before this function.
 * - Do NOT auto-call MX_SDIO_SD_Init() at boot if you want Rosetta to boot
 *   normally with the socket empty. This function initializes SDIO on demand.
 */
bool RosettaSD_RunAcceptance(SD_HandleTypeDef *hsd, RosettaSDData *out);

#endif
