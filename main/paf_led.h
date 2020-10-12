#ifndef __PAF_LED_H__
#define __PAF_LED_H__

/**
 * @file paf_led.h
 * @author Alex Hoffman
 * @date 25 August 2020
 * @brief LED control
 *
 * @verbatim
   ----------------------------------------------------------------------
    Copyright (C) Alexander Hoffman, 2020
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
   ----------------------------------------------------------------------
@endverbatim
 */
#include "esp_err.h"

#define PULS_TIMER_TICKS_S 100000

typedef enum paf_led_mode {
    PAF_LED_MODE_NOTSET = 0,
    PAF_LED_MODE_GPIO,
    PAF_LED_MODE_PWM,
    PAF_LED_MODE_CONSOLE,
} paf_led_mode_t;
esp_err_t paf_led_set_start_test(void);
esp_err_t paf_led_set_on(void);
esp_err_t paf_led_set_off(void);
esp_err_t paf_led_set_toggle(void);
esp_err_t  paf_led_set_dc(unsigned int duty_cycle);
unsigned int paf_led_get_dc(void);
esp_err_t paf_led_set_freq(unsigned int freq);
unsigned int paf_led_get_freq(void);
char paf_led_get_led(void);
esp_err_t paf_led_init(paf_led_mode_t mode);
unsigned int paf_led_get_time(void);
void paf_led_set_time(unsigned int duration);
void paf_led_init_hw_timer(void);
esp_err_t paf_led_init_pulse(void);
esp_err_t paf_led_start_test(void);

int paf_led_set_pulse_on_duration(unsigned int pulse_on_duration);
void paf_led_set_pulse_selected();
void paf_led_set_pulse_not_selected();
int paf_led_set_pulse_periode(unsigned int periode);
#endif // __PAF_LED_H__
