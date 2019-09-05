/*
 * Copyright (c) 2019 Winner Microelectronics Co., Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-02-13     tyx          first implementation
 */


#include <rtthread.h>
#include <rtdevice.h>

int main(void)
{
    rt_err_t ret = RT_EOK;
    rt_uint8_t mac[10];

    rt_wlan_get_mac(mac);

    rt_kprintf("mac is, %02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    
    return ret;
}