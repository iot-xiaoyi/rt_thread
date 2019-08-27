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
#include <webclient.h>

#define HTTP_URL_GET   "http://www.rt-thread.com/service/rt-thread.txt"
#define BUFF_RESP_SIZE  1024
#define BUFF_HEADER_SIZE 1024

// #define http_client_REMOTE_ADDR "192.168.1.13"
// #define http_client_REMOTE_PORT 8089

static rt_sem_t wait_sem = NULL;

char *send_data = "hello http_client object!\r\n";

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


static void http_client_thread_entry(void *args)
{
    int ret = 0;
    int resp_status;
    struct  webclient_ssion *session = RT_NULL;
    int len = 0, bytes_read = 0, pos = 0;
    char buff_recv[BUFF_RESP_SIZE] = { 0x00 };
    
    //创建会话
    session = webclient_session_create(BUFF_HEADER_SIZE);
    if (RT_NULL == session)
    {
        rt_kprintf("No memory for get header!\r\n");
        goto exit;
    }

    // 使用默认头部发送GET请求
    if ((resp_status = webclient_get(session, HTTP_URL_GET)) != 200)
    {
        rt_kprintf("webclient GET request failed, error code is:%d\r\n", resp_status);
        goto exit;
    }

    len = webclient_content_length_get(session);
    if (len < 0)
    {
        while (1)
        {
            bytes_read = webclient_read(session, buff_recv, BUFF_RESP_SIZE);
            if (bytes_read <= 0)
            {
                break;
            }
        }
    }
    else
    {
        while (pos < len)
        {
            bytes_read = webclient_read(session, buff_recv, len - pos > BUFF_RESP_SIZE ? BUFF_RESP_SIZE : len - pos);
            if (bytes_read <= 0)
            {
                break;
            }
            pos += bytes_read;
        }
    }

    rt_kprintf("recv data:%s\r\n", buff_recv);

exit:
    if (NULL != session)
    {
        webclient_close(session);
    }
    rt_kprintf("thread http_client exit!\r\n");
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
    ret = rt_wlan_register_event_handler(RT_WLAN_EVT_READY, wifi_ready_callback, RT_NULL);
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
    rt_kprintf("connect to AP success, and ready OK, can send data\r\n");

    // rt_thread_mdelay(5000);

    //create http_client thread
    rt_thread_t client_thread = rt_thread_create("http", http_client_thread_entry, RT_NULL, 4*1024, 25, 10);
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