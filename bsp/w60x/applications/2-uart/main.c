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

//  用于接收消息的信号量 */
static struct rt_semaphore rx_sem;
rt_device_t user_uart;

static rt_err_t user_uart_input(rt_device_t dev, rt_size_t size)
{
    rt_sem_release(&rx_sem);

    return RT_EOK;
}

static void user_uart_thread_entry(void *args)
{
    char ch;

    while (1)
    {
        while (1 != rt_device_read(user_uart, -1, &ch, 1))
        {
            rt_sem_take(&rx_sem, RT_WAITING_FOREVER);
        }
        //send back
        rt_device_write(user_uart, 0, &ch, 1);
    }
    
}

int main(void)
{
    rt_err_t ret = RT_EOK;
    char str[] = "hello world!\r\n";

    user_uart = rt_device_find("uart1");
    if (!user_uart)
    {
        rt_kprintf("find uart1 failed!\r\n");
        return RT_ERROR;
    }

    rt_sem_init(&rx_sem, "rx_sem", 0, RT_IPC_FLAG_FIFO);
    rt_device_open(user_uart, RT_DEVICE_FLAG_INT_RX);
    rt_device_set_rx_indicate(user_uart, user_uart_input);
    rt_device_write(user_uart, 0, str, sizeof(str) - 1);

    rt_thread_t uart_thread = rt_thread_create("user_uart", user_uart_thread_entry, RT_NULL, 1024, 25, 10);
    if (uart_thread != NULL)
    {
        rt_thread_startup(uart_thread);
    }else
    {
        ret = RT_ERROR;
    }
    
    return ret;
}