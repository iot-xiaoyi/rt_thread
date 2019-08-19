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
#include <sys/socket.h> //使用BSD socket需要包含此头文件


#define UDP_LOCAL_PORT 2000

// #define UDP_REMOTE_ADDR "192.168.1.13"
// #define UDP_REMOTE_PORT 8089

static rt_sem_t wait_sem = NULL;

char *send_data = "hello UDP object!\r\n";

static void wifi_connect_callback(int event, struct rt_wlan_buff *buff, void *parameter)
{
    rt_kprintf("%s\n", __FUNCTION__);
    rt_sem_release(wait_sem);
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

static void udp_thread_entry(void *args)
{
    int ret = 0;
    int fd = -1;
    struct sockaddr_in addr, remote_addr;
    socklen_t addrLen = sizeof(addr);
    struct  timeval t;
    fd_set readfds;

    char buf[512] = { 0x00 };
    int len = 0;

reconnect:
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 == fd)
    {
        rt_kprintf("create socket error!!!\r\n");
        goto exit;
    }
    addr.sin_family = AF_INET;
    addr.sin_port = htons(UDP_LOCAL_PORT);
    addr.sin_addr.s_addr = INADDR_ANY;
    rt_memset(&addr.sin_zero, 0x00, sizeof(addr.sin_zero));

    ret = bind( fd, (struct sockaddr *)&addr, sizeof(addr));
    if (0 != ret)
    {
        rt_kprintf("bind addr error!!!\r\n");
    }

    while (1)
    {
        len = recvfrom(fd, buf, sizeof(buf), 0, (struct sockaddr *)&addr, &addrLen);
        if (len > 0)
        {
            buf[len] = 0x00;
            rt_kprintf("receive data:%s, from %s:%d\r\n", buf, inet_ntoa(addr.sin_addr), addr.sin_port);
            sendto(fd, buf, len, 0, (struct sockaddr *)&addr, sizeof(struct sockaddr_in) );
        }else
        {
            rt_kprintf("receive data from tcp server error!\r\n");
            goto label_try_reconnect;
        }
    }

label_try_reconnect:
    if (-1 != fd)
    {
        closesocket(fd);
    }
    rt_thread_mdelay(1000);
    goto reconnect;

exit:
    if (-1 != fd)
    {
        closesocket(fd);
    }
    rt_kprintf("thread udp exit!\r\n");
}

int main(void)
{
    rt_err_t ret = RT_EOK;
    char str[] = "hello world!\r\n";

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

    // connect to router
    rt_wlan_set_mode(RT_WLAN_DEVICE_STA_NAME, RT_WLAN_STATION);
    rt_wlan_connect("HUAWEI-6ZCHWJ", "123456789a");
    rt_kprintf("start to connect ap ...\n");

    // wait until module connect to ap success
    ret = rt_sem_take(wait_sem, RT_WAITING_FOREVER);
    if (0 != ret)
    {
        rt_kprintf("wait_sem error!\r\n");
    }
    rt_kprintf("connect to AP success!\r\n");

    //create udp thread
    rt_thread_t client_thread = rt_thread_create("udp", udp_thread_entry, RT_NULL, 4*1024, 25, 10);
    if (client_thread != NULL)
    {
        rt_thread_startup(client_thread);
    }else
    {
        ret = RT_ERROR;
        rt_kprintf("create tcp client error!!!");
    }

exit:
    rt_sem_delete(wait_sem);
    return ret;
}