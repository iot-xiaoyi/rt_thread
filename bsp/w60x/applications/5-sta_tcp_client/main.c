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

#define TCP_SERVER_ADDR "192.168.1.13"
#define TCP_SERVER_PORT 8089

static rt_sem_t wait_sem = NULL;

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

static void tcp_client_thread_entry(void *args)
{
    int ret = 0;
    int fd = -1;
    struct sockaddr_in server_addr;
    struct  timeval t;
    fd_set readfds;

    char buf[512] = { 0x00 };
    int len = 0;

reconnect:
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == fd)
    {
        rt_kprintf("create socket error!!!\r\n");
        goto exit;
    }
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(TCP_SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr( TCP_SERVER_ADDR );
    rt_memset(&server_addr.sin_zero, 0x00, sizeof(server_addr.sin_zero));

    ret = connect(fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr ));
    if (0 == ret)
    {
        rt_kprintf("connect success\r\n");
    }else
    {
        rt_kprintf("connect error!!!\r\n");
        goto label_try_reconnect;
    }
    
    t.tv_sec = 2;
    t.tv_usec = 0;

    while (1)
    {
        FD_ZERO(&readfds);
        FD_SET(fd, &readfds);

        ret = select(fd + 1, &readfds, 0, 0, &t);
        if (-1 == ret)
        {
            rt_kprintf("select() error!\r\n");
            goto label_try_reconnect;
        }
        // else if(0 == ret)
        // {
        //     rt_kprintf("select() timeout!\r\n");
        // }
        else if(ret > 0)
        {
            if (FD_ISSET(fd, &readfds));
            {
                len = recv(fd, buf, sizeof(buf), 0);
                if (len > 0)
                {
                    buf[len] = 0x00;
                    rt_kprintf("receive data:%s\r\n", buf);
                }else
                {
                    rt_kprintf("receive data from tcp server error!\r\n");
                    goto label_try_reconnect;
                }

                if (-1 != fd)
                {
                    rt_sprintf(buf, "%s\r\n", buf);
                    ret = send(fd, buf, strlen(buf), 0);
                    if (ret < 0)
                    {
                        rt_kprintf("send error, closee socket");
                        goto label_try_reconnect;
                    }
                }
            }
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
    rt_kprintf("thread tcp_client exit!\r\n");
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
    rt_kprintf("connect to ap success!\r\n");

    //create client
    rt_thread_t client_thread = rt_thread_create("tcp_client", tcp_client_thread_entry, RT_NULL, 4*1024, 25, 10);
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