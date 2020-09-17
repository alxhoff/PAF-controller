/*
 *
 * @file paf_gpio.c
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
#include "paf_gpio.h"
#include "esp_err.h"

gpio_config_t pin_config_output = { .pin_bit_mask = 0,
                                    .mode = GPIO_MODE_OUTPUT,
                                    .pull_up_en = GPIO_PULLUP_ENABLE,
                                    .pull_down_en = GPIO_PULLDOWN_DISABLE,
                                    .intr_type = GPIO_INTR_DISABLE
                                  };

esp_err_t paf_gpio_init(uint64_t pin_set_register)
{
    pin_config_output.pin_bit_mask = pin_set_register;

    return gpio_config(&pin_config_output);
}

esp_err_t paf_gpio_toggle_state(gpio_num_t pin)
{
    if (pin_config_output.pin_bit_mask & (1 << pin)) {
        return gpio_set_level(pin, !gpio_get_level(pin));
    }
    else {
        return ESP_FAIL;
    }
}

esp_err_t paf_set_gpio_high(gpio_num_t pin)
{
    if (pin_config_output.pin_bit_mask & (1 << pin)) {
        gpio_set_level(pin, 1);
        return ESP_OK;
    }
    else {
        return ESP_FAIL;
    }
}

esp_err_t paf_set_gpio_low(gpio_num_t pin)
{
    if (pin_config_output.pin_bit_mask & (1 << pin)) {
        gpio_set_level(pin, 0);
        return ESP_OK;
    }
    else {
        return ESP_FAIL;
    }
}
