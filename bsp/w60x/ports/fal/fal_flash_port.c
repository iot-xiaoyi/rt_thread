/*
 * File      : fal_flash_sfud_port.c
 * COPYRIGHT (C) 2012-2018, Shanghai Real-Thread Technology Co., Ltd
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-01-26     armink       the first version
 * 2018-08-21     MurphyZhao   update to stm32l4xx
 */

#include <fal.h>
#include "wm_flash_map.h"
#include "wm_internal_flash.h"

#define FLASH_BEGIN    (USER_ADDR_START)
#define FLASH_SIZE     (1024 * 1024)
#define FLASH_SECTOR   (4096)

static int init(void)
{
    return 0;
}

static int read(long offset, uint8_t *buf, size_t size)
{
    if (tls_fls_read(offset, buf, size) != TLS_FLS_STATUS_OK)
    {
        return -1;
    }

    return size;
}

static int write(long offset, const uint8_t *buf, size_t size)
{
    if (tls_fls_write(offset, (uint8_t *)buf, size) != TLS_FLS_STATUS_OK)
    {
        return -1;
    }
    return size;
}

static int erase(long offset, size_t size)
{
    int count = size / FLASH_SECTOR;
    int sector = offset / FLASH_SECTOR;

    while (count)
    {
        if (tls_fls_erase(sector) != TLS_FLS_STATUS_OK)
        {
            return -1;
        }
        count--;
        sector++;
    }

    return size;
}
const struct fal_flash_dev nor_flash0 = { "nor_flash", 0, FLASH_SIZE, 4096, {init, read, write, erase} };