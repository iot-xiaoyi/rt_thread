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
#include "easyflash.h"

#define KV_DATA  "kv_data"

EfErrCode kv_get_data(void)
{
    EfErrCode err = EF_NO_ERR;
    char *buff_get = NULL;

    buff_get = ef_get_env(KV_DATA);
    if (NULL != buff_get)
    {
        rt_kprintf("get data : %s", buff_get);
    }else
    {
        rt_kprintf("get data error");
        err = EF_ENV_NAME_ERR;
    }

    return err;
}

EfErrCode kv_set_data(char *data)
{
    EfErrCode err = EF_NO_ERR;

    err = ef_set_and_save_env(KV_DATA, data);
    if (EF_NO_ERR == err)
    {
        rt_kprintf("set data success");
    }else
    {
        rt_kprintf("set data error");
    }

    return err;
}

int main(void)
{
    rt_err_t ret = RT_EOK;
    EfErrCode code = EF_NO_ERR;
    int num = 0;
    char data[10] = { 0x00 };
    
    easyflash_init();

    kv_get_data();
    
    while (1)
    {
        rt_sprintf(data, "%d%d%d%d%d%d", num, num, num, num, num, num);
        kv_set_data(data);
        kv_get_data();
        rt_thread_mdelay(2000);

        if (9 < num)
        {
            num = 0;
        }else
        {
            num ++;
        }
    }
    
    return ret;
}