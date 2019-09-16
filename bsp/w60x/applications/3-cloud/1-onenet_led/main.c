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
#include <easyflash.h>
#include "onenet.h"

#define LED_PIN           (23)


static rt_sem_t wait_sem = NULL;


static void wifi_connect_callback(int event, struct rt_wlan_buff *buff, void *parameter)
{
    rt_kprintf("%s\n", __FUNCTION__);
}

static void wifi_disconnect_callback(int event, struct rt_wlan_buff *buff, void *parameter)
{
    rt_kprintf("%s\n", __FUNCTION__);
    if ((buff != RT_NULL) && (buff->len == sizeof(struct rt_wlan_info)))
    {
        rt_kprintf("ssid : %s\r\n", ((struct rt_wlan_info *)buff->data)->ssid.val);
    }
}

static void wifi_connect_fail_callback(int event, struct rt_wlan_buff *buff, void *parameter)
{
    rt_kprintf("%s\n", __FUNCTION__);
    if ((buff != RT_NULL) && (buff->len == sizeof(struct rt_wlan_info)))
    {
        rt_kprintf("ssid : %s\r\n", ((struct rt_wlan_info *)buff->data)->ssid.val);
    }
}

static void wifi_ready_callback(int event, struct rt_wlan_buff *buff, void *parameter)
{
    rt_kprintf("%s\n", __FUNCTION__);
    rt_sem_release(wait_sem);
}

int main(void)
{
    rt_err_t ret = RT_EOK;

    // 创建一个动态信号量，初始值为0
    wait_sem = rt_sem_create("sem_conn", 0, RT_IPC_FLAG_FIFO);

    /* Start automatic connection */
    rt_wlan_config_autoreconnect(RT_TRUE);

    // register event
    ret = rt_wlan_register_event_handler(RT_WLAN_EVT_STA_CONNECTED, wifi_connect_callback, RT_NULL);
    if (0 != ret)
    {
        rt_kprintf("register event handler error!\r\n");
    }
    ret = rt_wlan_register_event_handler(RT_WLAN_EVT_STA_DISCONNECTED, wifi_disconnect_callback, RT_NULL);
    if (0 != ret)
    {
        rt_kprintf("register event handler error!\r\n");
    }
    ret = rt_wlan_register_event_handler(RT_WLAN_EVT_STA_CONNECTED_FAIL, wifi_connect_fail_callback, RT_NULL);
    if (0 != ret)
    {
        rt_kprintf("register event handler error!\r\n");
    }
    ret = rt_wlan_register_event_handler(RT_WLAN_EVT_READY, wifi_ready_callback, RT_NULL);
    if (0 != ret)
    {
        rt_kprintf("register event handler error!\r\n");
    }

    /* fal init */
    fal_init();
    
    /* easyflash init */
    easyflash_init();
    
    //hardware init
    rt_pin_mode(LED_PIN, PIN_MODE_OUTPUT);
	bsp_init();


    // connect to router
    rt_wlan_set_mode(RT_WLAN_DEVICE_STA_NAME, RT_WLAN_STATION);
    rt_wlan_connect("lxy2305", "123456789a");
    rt_kprintf("start to connect ap ...\n");

    // wait until module connect to ap success
    ret = rt_sem_take(wait_sem, RT_WAITING_FOREVER);
    if (0 != ret)
    {
        rt_kprintf("wait_sem error!\r\n");
    }
    rt_kprintf("connect to AP success, and ready OK, can send data\r\n");

    // rt_thread_mdelay(5000);

    //start mqtt service
    onenet_mqtt_init();

exit:
    rt_sem_delete(wait_sem);
    return ret;
}

