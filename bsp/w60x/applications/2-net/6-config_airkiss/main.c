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

#include <sys/socket.h>
#include "smartconfig.h"

#define NET_READY_TIME_OUT       (rt_tick_from_millisecond(30 * 1000))

static int rt_wlan_device_connetct(char *ssid, char *passwd)
{
    int result = RT_EOK;
    rt_uint8_t time_cnt = 0;

    result = rt_wlan_connect(ssid, passwd);
    if (result != RT_EOK)
    {
        rt_kprintf("\nconnect ssid %s error:%d!\n", ssid, result);
        return result;
    };

    do
    {
        rt_thread_mdelay(1000);
        time_cnt ++;
        if (rt_wlan_is_ready())
        {
            break;
        }
    }
    while (time_cnt <= (NET_READY_TIME_OUT / 1000));

    if (time_cnt <= (NET_READY_TIME_OUT / 1000))
    {
        rt_kprintf("networking ready!\n");
    }
    else
    {
        rt_kprintf("wait ip got timeout!\n");
        result = -RT_ETIMEOUT;
    }

    return result;
}

static void airkiss_send_notification(void *args)
{
    int sock = -1;
    int udpbufsize = 2;
    rt_uint8_t random = (rt_uint32_t)args;
    struct sockaddr_in UDPBCAddr, UDPBCServerAddr;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        rt_kprintf("notify create socket error!\r\n");
        goto _exit;
    }

    UDPBCAddr.sin_family = AF_INET;
    UDPBCAddr.sin_port = htons(10000);
    UDPBCAddr.sin_addr.s_addr = htonl(0xffffffff);
    rt_memset(&(UDPBCAddr.sin_zero), 0, sizeof(UDPBCAddr.sin_zero));

    UDPBCServerAddr.sin_family = AF_INET;
    UDPBCServerAddr.sin_port = htons(10000);
    UDPBCServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    rt_memset(&(UDPBCServerAddr.sin_zero), 0, sizeof(UDPBCServerAddr.sin_zero));

    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &udpbufsize, sizeof(int)) != 0)
    {
        rt_kprintf("notify socket setsockopt error\r\n");
        goto _exit;
    }

    if (bind(sock, (struct sockaddr *)&UDPBCServerAddr, sizeof(UDPBCServerAddr)) != 0)
    {
        rt_kprintf("notify socket bind error\r\n");
        goto _exit;
    }

    for (int i = 0; i <= 20; i++)
    {
        rt_kprintf("send data[%d]:0x%02x\r\n", i, random);
        int ret = sendto(sock, (char *)&random, 1, 0, (struct sockaddr *)&UDPBCAddr, sizeof(UDPBCAddr));
        rt_thread_mdelay(100);
    }

    rt_kprintf("airkiss notification thread exit!\r\n");

_exit:
    if (sock >= 0)
    {
        closesocket(sock);
    }
}

static int smartconfig_result(rt_smartconfig_type result_type, char *ssid, char *passwd, void *userdata, rt_uint8_t userdata_len)
{
    int ret = RT_EOK;
    rt_uint8_t random = *(rt_uint8_t *)userdata;

    rt_kprintf("type:%d\r\n", result_type);
    rt_kprintf("ssid:%s\r\n", ssid);
    rt_kprintf("passwd:%s\r\n", passwd);
    rt_kprintf("user_data:0x%02x\r\n", random);
    if (rt_wlan_device_connetct(ssid, passwd) == RT_EOK)
    {
        //create airkiss_send_notification thread
        rt_thread_t notify_thread = rt_thread_create("notify", airkiss_send_notification, (void *)random, 4*1024, 25, 10);
        if (notify_thread != NULL)
        {
            rt_thread_startup(notify_thread);
        }else
        {
            ret = RT_ERROR;
            rt_kprintf("create notify thread error!!!\r\n");
        }
    }

    return 0;
}

void smartconfig_demo(void)
{
    rt_smartconfig_start(SMARTCONFIG_TYPE_AIRKISS, SMARTCONFIG_ENCRYPT_NONE, RT_NULL, smartconfig_result);
}

int main(void)
{
    rt_err_t ret = RT_EOK;

    ret = rt_wlan_set_mode(RT_WLAN_DEVICE_STA_NAME, RT_WLAN_STATION);
    if (RT_EOK == ret)
    {
        rt_kprintf("Start airkiss...\r\n");
        smartconfig_demo();
    }

exit:
    return ret;
}