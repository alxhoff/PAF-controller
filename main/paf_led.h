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

#include "paf_config.h"

#define PAF_LED_TIMER LEDC_TIMER_0
#define PAF_LED_MODE LEDC_HIGH_SPEED_MODE
#define PAF_LED_GPIO PAF_DEF_LED_GPIO
#define PAF_LED_CHANNEL LEDC_CHANNEL_0

#endif // __PAF_LED_H__
