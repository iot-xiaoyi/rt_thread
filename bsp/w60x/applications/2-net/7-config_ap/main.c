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
#include "cJSON.h"

#define UDP_LOCAL_PORT 20032

typedef struct _ap_result_t{
    /* data */
    char ssid[32];
    char password[32];
}ap_result_t;


static rt_sem_t wait_sem = NULL;
static rt_sem_t ssid_sem = NULL;
ap_result_t ap_result = { 0x00 };

static rt_uint8_t mac[10] = { 0x00 };
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

//{"ssid":"Tenda_00F430","password":"15037013924"}
int get_ssid_from_json_data(char *buf, ap_result_t *ap_result)
{
    cJSON *json=cJSON_Parse(buf);
    cJSON *item = cJSON_GetObjectItem(json,"ssid");
    rt_kprintf("ssid:%s\n",item->valuestring);
    strcpy(ap_result->ssid, item->valuestring);

    item = cJSON_GetObjectItem(json,"password");
    rt_kprintf("password:%s\n",item->valuestring);
    strcpy(ap_result->password, item->valuestring);

    return 0;
}

static void wifi_config_thread_entry(void *args)
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
            //get ssid
            get_ssid_from_json_data(buf, &ap_result);

            rt_memset(buf, 0x00, sizeof(buf));
            rt_sprintf(buf, "{\"status\":\"ok\",\"mac\":\"%02x%02x%02x%02x%02x%02x\",\"type\":\"led\"}", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
            len = strlen(buf);
            for (int i = 0; i < 5; i++)
            {
                sendto(fd, buf, len, 0, (struct sockaddr *)&addr, sizeof(struct sockaddr_in) );
                rt_thread_mdelay(100);
            }
            rt_sem_release(ssid_sem);
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
    char softAP_ssid[32] = { 0x00 };
    struct rt_wlan_info info = { 0x00 };

    // 创建一个动态信号量，初始值为0
    wait_sem = rt_sem_create("sem_conn", 0, RT_IPC_FLAG_FIFO);
    ssid_sem = rt_sem_create("sem_get_ssid", 0, RT_IPC_FLAG_FIFO);

    rt_wlan_set_mode(RT_WLAN_DEVICE_STA_NAME, RT_WLAN_STATION);

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

    // get mac address
    rt_wlan_get_mac(mac);
    rt_sprintf(softAP_ssid, "LED_%02x%02x%02x", mac[3], mac[4], mac[5]);

    /* start softAP, ready to receive router's ssid and password. */
    rt_wlan_set_mode(RT_WLAN_DEVICE_AP_NAME, RT_WLAN_AP);
    info.security = SECURITY_WPA2_AES_PSK;
    rt_memcpy(&info.ssid.val, softAP_ssid, strlen(softAP_ssid));
    info.ssid.len = strlen(softAP_ssid);
    info.channel = 6;
    info.band = RT_802_11_BAND_2_4GHZ;

    rt_wlan_start_ap_adv(&info, "12345678");

    //get router's ssid
    rt_thread_t config_thread = rt_thread_create("wifi_config", wifi_config_thread_entry, RT_NULL, 4*1024, 25, 10);
    if (config_thread != NULL)
    {
        rt_thread_startup(config_thread);
    }else
    {
        ret = RT_ERROR;
        rt_kprintf("create wifi_config thread error!!!");
    }

    //wait untill get router's ssid
    ret = rt_sem_take(ssid_sem, RT_WAITING_FOREVER);
    if (0 != ret)
    {
        rt_kprintf("wait_sem error!\r\n");
    }
    rt_wlan_ap_stop();

    rt_wlan_set_mode(RT_WLAN_DEVICE_STA_NAME, RT_WLAN_STATION);
    /* Start automatic connection */
    rt_wlan_config_autoreconnect(RT_TRUE);
    
    // connect to router
    rt_wlan_connect(ap_result.ssid, ap_result.password);
    rt_kprintf("start to connect ap ...\n");

    // wait until module connect to ap success
    ret = rt_sem_take(wait_sem, RT_WAITING_FOREVER);
    if (0 != ret)
    {
        rt_kprintf("wait_sem error!\r\n");
    }
    rt_kprintf("connect to AP success!\r\n");

exit:
    rt_sem_delete(wait_sem);
    return ret;
}