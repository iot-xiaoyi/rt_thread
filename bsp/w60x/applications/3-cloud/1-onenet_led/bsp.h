/*
     * File      : HX711.h
     * This file is part of RT-Thread RTOS
     * COPYRIGHT (C) 2006 - 2012, RT-Thread Development Team
     *
     *  This program is free software; you can redistribute it and/or modify
     *  it under the terms of the GNU General Public License as published by
     *  the Free Software Foundation; either version 2 of the License, or
     *  (at your option) any later version.
     *
     *  This program is distributed in the hope that it will be useful,
     *  but WITHOUT ANY WARRANTY; without even the implied warranty of
     *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     *  GNU General Public License for more details.
     *
     *  You should have received a copy of the GNU General Public License along
     *  with this program; if not, write to the Free Software Foundation, Inc.,
     *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
     *
     * Change Logs:
     * Date           Author       Notes
     * 2019-06-17     Bernard      the first version
     * ...
     */
#ifndef __BSP_H__
#define __BSP_H__


#define LED_RED_PIN       (26)
#define LED_GREEN_PIN     (27)
#define LED_BLUE_PIN      (28)

void bsp_init(void);
void led_red_on(void);
void led_green_on(void);
void led_blue_on(void);
void led_off(void);

#endif