/*
 * File      : fal_cfg.h
 * COPYRIGHT (C) 2012-2018, Shanghai Real-Thread Technology Co., Ltd
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-08-21     MurphyZhao   the first version
 */
#ifndef _FAL_CFG_H_
#define _FAL_CFG_H_

#include <board.h>

extern const struct fal_flash_dev nor_flash0;

/* flash device table */
#define FAL_FLASH_DEV_TABLE                                          \
{                                                                    \
    &nor_flash0,                                                     \
}

/* ====================== Partition Configuration ========================== */
#define FAL_PART_HAS_TABLE_CFG
#ifdef FAL_PART_HAS_TABLE_CFG
/* partition table */
#define FAL_PART_TABLE                                                                                         \
{                                                                                                              \
    {FAL_PART_MAGIC_WROD, "app", "nor_flash",                              (4 + 4 + 56) * 1024, 512 * 1024, 0},  \
    {FAL_PART_MAGIC_WROD, "download", "nor_flash",                   (4 + 4 + 56 + 512) * 1024, 384 * 1024, 0},  \
    {FAL_PART_MAGIC_WROD, "fs_part", "nor_flash",              (4 + 4 + 56 + 512 + 384) * 1024,  44 * 1024, 0},  \
    {FAL_PART_MAGIC_WROD, "easyflash", "nor_flash",       (4 + 4 + 56 + 512 + 384 + 44) * 1024,   4 * 1024, 0},  \
}
#endif /* FAL_PART_HAS_TABLE_CFG */
#endif /* _FAL_CFG_H_ */