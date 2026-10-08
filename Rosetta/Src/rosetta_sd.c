#include "rosetta_sd.h"
#include "fatfs.h"
#include "ff.h"
#include <stdio.h>
#include <string.h>

#define ROSETTA_SD_TEST_BYTES 64u

static void clear_result(RosettaSDData *out)
{
    memset(out, 0, sizeof(*out));
    out->result = ROSETTA_SD_NOT_RUN;
}

static void configure_handle(SD_HandleTypeDef *hsd)
{
    /*
     * STM32F446 + SDIO.
     * HAL_SD_Init() performs the card-identification phase at low speed
     * internally. ClockDiv = 0 gives a conservative transfer clock once the
     * card is initialized when SDIO receives a 48 MHz kernel clock.
     */
    hsd->Instance = SDIO;
    hsd->Init.ClockEdge = SDIO_CLOCK_EDGE_RISING;
    hsd->Init.ClockBypass = SDIO_CLOCK_BYPASS_DISABLE;
    hsd->Init.ClockPowerSave = SDIO_CLOCK_POWER_SAVE_DISABLE;
    hsd->Init.BusWide = SDIO_BUS_WIDE_1B;
    hsd->Init.HardwareFlowControl = SDIO_HARDWARE_FLOW_CONTROL_DISABLE;
    hsd->Init.ClockDiv = 0u;
}

static bool make_temp_file(char *path, size_t path_len, FIL *file, FRESULT *fr_out)
{
    for (unsigned i = 0u; i < 100u; ++i)
    {
        snprintf(path, path_len, "%s/ROS%02u.TST", SDPath, i);
        FRESULT fr = f_open(file, path, FA_CREATE_NEW | FA_WRITE);
        if (fr == FR_OK)
        {
            *fr_out = FR_OK;
            return true;
        }

        if (fr != FR_EXIST)
        {
            *fr_out = fr;
            return false;
        }
    }

    *fr_out = FR_EXIST;
    return false;
}

bool RosettaSD_RunAcceptance(SD_HandleTypeDef *hsd, RosettaSDData *out)
{
    if (hsd == NULL || out == NULL)
        return false;

    clear_result(out);

    /*
     * Start every manual test from a known peripheral state. HAL_SD_DeInit()
     * is harmless if the peripheral was not previously active.
     */
    if (hsd->Instance == SDIO)
        (void)HAL_SD_DeInit(hsd);

    configure_handle(hsd);

    if (HAL_SD_Init(hsd) != HAL_OK)
    {
        /*
         * Rosetta has no physical CARD_DETECT line. Therefore software cannot
         * distinguish an empty socket from a card/contact/bus failure here.
         */
        out->result = ROSETTA_SD_NO_RESPONSE;
        return false;
    }

    out->card_present = 1u;

    if (HAL_SD_ConfigWideBusOperation(hsd, SDIO_BUS_WIDE_4B) != HAL_OK)
    {
        out->result = ROSETTA_SD_SDIO_ERROR;
        return false;
    }

    HAL_SD_CardInfoTypeDef info;
    if (HAL_SD_GetCardInfo(hsd, &info) != HAL_OK)
    {
        out->result = ROSETTA_SD_SDIO_ERROR;
        return false;
    }

    out->sdio_ok = 1u;
    out->logical_block_size = info.LogBlockSize;

    {
        uint64_t bytes = (uint64_t)info.LogBlockNbr * (uint64_t)info.LogBlockSize;
        out->size_mb = (uint32_t)(bytes / (1024ull * 1024ull));
    }

    FRESULT fr = f_mount(&SDFatFS, (TCHAR const *)SDPath, 1u);
    out->fatfs_result = (uint8_t)fr;

    if (fr != FR_OK)
    {
        out->result = ROSETTA_SD_FATFS_ERROR;
        return false;
    }

    out->fatfs_ok = 1u;

    uint8_t tx[ROSETTA_SD_TEST_BYTES];
    uint8_t rx[ROSETTA_SD_TEST_BYTES];

    for (uint32_t i = 0u; i < ROSETTA_SD_TEST_BYTES; ++i)
        tx[i] = (uint8_t)(0x5Au ^ (uint8_t)(i * 13u));

    memset(rx, 0, sizeof(rx));

    FIL file;
    char path[32];
    bool created = false;
    UINT bw = 0u;
    UINT br = 0u;

    if (!make_temp_file(path, sizeof(path), &file, &fr))
    {
        out->fatfs_result = (uint8_t)fr;
        out->result = ROSETTA_SD_RW_ERROR;
        (void)f_mount(NULL, (TCHAR const *)SDPath, 1u);
        return false;
    }

    created = true;

    fr = f_write(&file, tx, sizeof(tx), &bw);
    if (fr == FR_OK && bw == sizeof(tx))
        fr = f_sync(&file);

    (void)f_close(&file);

    if (fr != FR_OK || bw != sizeof(tx))
        goto rw_fail;

    fr = f_open(&file, path, FA_READ);
    if (fr != FR_OK)
        goto rw_fail;

    fr = f_read(&file, rx, sizeof(rx), &br);
    (void)f_close(&file);

    if (fr != FR_OK || br != sizeof(rx) || memcmp(tx, rx, sizeof(tx)) != 0)
        goto rw_fail;

    fr = f_unlink(path);
    created = false;

    if (fr != FR_OK)
        goto rw_fail;

    out->fatfs_result = (uint8_t)FR_OK;
    out->rw_ok = 1u;
    out->result = ROSETTA_SD_PASS;

    (void)f_mount(NULL, (TCHAR const *)SDPath, 1u);
    return true;

rw_fail:
    out->fatfs_result = (uint8_t)fr;

    if (created)
        (void)f_unlink(path);

    (void)f_mount(NULL, (TCHAR const *)SDPath, 1u);

    out->result = ROSETTA_SD_RW_ERROR;
    return false;
}
