#ifndef __PAF_GPIO__
#define __PAF_GPIO__

/*
 *
 * @file paf_gpio.h
 * @author Lars Pauli
 * @date 16 September 2020
 * @brief GPIO control
 *
 * @verbatim
   ----------------------------------------------------------------------
    Copyright (C) Lars Pauli, 2020
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

#include "driver/gpio.h"
#include "paf_config.h"

#define PAF_LED_1 (12)
#define PAF_LED_2 (14)
#define PAF_LED_3 (27)
#define PAF_LED_4 (26)
#define PAF_LED_5 (25)
#define PAF_LED_6 (33)
#define PAF_LED_7 (32)
#define PAF_LED_8 (35)

esp_err_t paf_gpio_init(uint64_t pin_set_register);
esp_err_t paf_gpio_toggle_state(unsigned int pin);
int paf_set_gpio_high(unsigned int pin);
int paf_set_gpio_low(unsigned int pin);


#endif