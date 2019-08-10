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

#define W600_IO_LED1    19
#define W600_IO_LED2    20
#define W600_IO_LED3    21
#define W600_IO_LED4    22
#define W600_IO_LED5    23

int main(void)
{
    /* set wifi work mode */
    rt_wlan_set_mode(RT_WLAN_DEVICE_STA_NAME, RT_WLAN_STATION);
    rt_wlan_set_mode(RT_WLAN_DEVICE_AP_NAME, RT_WLAN_AP);

    /* init led */
    rt_pin_mode(W600_IO_LED1, PIN_MODE_OUTPUT);
    rt_pin_mode(W600_IO_LED2, PIN_MODE_OUTPUT);
    rt_pin_mode(W600_IO_LED3, PIN_MODE_OUTPUT);
    rt_pin_mode(W600_IO_LED4, PIN_MODE_OUTPUT);
    rt_pin_mode(W600_IO_LED5, PIN_MODE_OUTPUT);

    while (1)
    {
        rt_pin_write(W600_IO_LED1, PIN_LOW);
        rt_pin_write(W600_IO_LED2, PIN_LOW);
        rt_pin_write(W600_IO_LED3, PIN_LOW);
        rt_pin_write(W600_IO_LED4, PIN_LOW);
        rt_pin_write(W600_IO_LED5, PIN_LOW);
        rt_kprintf("led on\r\n");

        rt_thread_mdelay(2000);

        rt_pin_write(W600_IO_LED1, PIN_HIGH);
        rt_pin_write(W600_IO_LED2, PIN_HIGH);
        rt_pin_write(W600_IO_LED3, PIN_HIGH);
        rt_pin_write(W600_IO_LED4, PIN_HIGH);
        rt_pin_write(W600_IO_LED5, PIN_HIGH);
        rt_kprintf("led off\r\n");

        rt_thread_mdelay(2000);
    }

    return 0;
}

