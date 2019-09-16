/*
  Copyright (c) 2009 Dave Gamble
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:
  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

/* bsp */

#include "bsp.h"
#include <rtthread.h>
#include <rtdevice.h>

void bsp_init(void)
{
	rt_pin_mode(LED_RED_PIN, PIN_MODE_OUTPUT);
	rt_pin_mode(LED_GREEN_PIN, PIN_MODE_OUTPUT);
	rt_pin_mode(LED_BLUE_PIN, PIN_MODE_OUTPUT);
}


void led_red_on(void)
{
	rt_pin_write(LED_RED_PIN, PIN_LOW);
	rt_pin_write(LED_GREEN_PIN, PIN_HIGH);
	rt_pin_write(LED_BLUE_PIN, PIN_HIGH);
}

void led_green_on(void)
{
	rt_pin_write(LED_RED_PIN, PIN_HIGH);
	rt_pin_write(LED_GREEN_PIN, PIN_LOW);
	rt_pin_write(LED_BLUE_PIN, PIN_HIGH);
}

void led_blue_on(void)
{
	rt_pin_write(LED_RED_PIN, PIN_HIGH);
	rt_pin_write(LED_GREEN_PIN, PIN_HIGH);
	rt_pin_write(LED_BLUE_PIN, PIN_LOW);
}

void led_off(void)
{
	rt_pin_write(LED_RED_PIN, PIN_HIGH);
	rt_pin_write(LED_GREEN_PIN, PIN_HIGH);
	rt_pin_write(LED_BLUE_PIN, PIN_HIGH);
}