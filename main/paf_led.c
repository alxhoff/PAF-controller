
/**
 * @file paf_led.c
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

#include "driver/ledc.h"

#include "esp_err.h"
#include "esp_log.h"

#include "driver/gpio.h"

#include "paf_config.h"
#include "paf_led.h"

#define PAF_LED_TIMER LEDC_TIMER_0
#define PAF_LED_MODE LEDC_HIGH_SPEED_MODE
#define PAF_LED_GPIO PAF_DEF_LED_GPIO
#define PAF_LED_CHANNEL LEDC_CHANNEL_0

ledc_timer_config_t ledc_timer = {
    .duty_resolution = LEDC_TIMER_13_BIT,
    .freq_hz = PAF_DEF_LED_FREQ,
    .speed_mode = PAF_LED_MODE,
    .timer_num = PAF_LED_TIMER,
    .clk_cfg = LEDC_AUTO_CLK,
};

ledc_channel_config_t ledc_channel = {
    .channel = PAF_LED_CHANNEL,
    .duty = PAF_DEF_LED_DC,
    .gpio_num = PAF_DEF_LED_GPIO,
    .speed_mode = PAF_LED_MODE,
    .hpoint = 0,
    .timer_sel = PAF_LED_TIMER,
};

static char led_status = 0;
static paf_led_mode_t led_mode = PAF_LED_CONSOLE;

char paf_led_get_led(void)
{
    return led_status;
}

int paf_led_init(paf_led_mode_t mode)
{
    switch (mode) {
        case PAF_LED_BINARY:
            gpio_pad_select_gpio(PAF_DEF_LED_GPIO);
            gpio_set_direction(PAF_DEF_LED_GPIO, GPIO_MODE_OUTPUT);
            break;
        case PAF_LED_PWM:
            ledc_timer_config(&ledc_timer);
            ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
            ledc_fade_func_install(0);
            led_mode = PAF_LED_PWM;
            break;
        case PAF_LED_CONSOLE:
            break;
        default:
            return -1;
    }
    led_mode = mode;

    return 0;
}

int paf_led_set_on(void)
{
    esp_err_t ret = ESP_OK;
    switch (led_mode) {
        case PAF_LED_BINARY:
            ret = gpio_set_level(PAF_DEF_LED_GPIO, 1);
            break;
        case PAF_LED_PWM:
            ledc_fade_start(ledc_channel.speed_mode, ledc_channel.channel,
                            LEDC_FADE_NO_WAIT);
            break;
        case PAF_LED_CONSOLE:
            ESP_LOGI(__func__, "Setting LED on");
            ret = ESP_OK;
            break;
        default:
            ret = -1;
            break;
    }
    led_status = 1;
    return ret;
}

int paf_led_set_off(void)
{
    esp_err_t ret = ESP_OK;
    switch (led_mode) {
        case PAF_LED_BINARY:
            ret = gpio_set_level(PAF_DEF_LED_GPIO, 0);
            break;
        case PAF_LED_PWM:
            ret = ledc_stop(ledc_channel.speed_mode, ledc_channel.channel,
                            0);
            break;
        case PAF_LED_CONSOLE:
            ESP_LOGI(__func__, "Setting LED off");
            ret = ESP_OK;
            break;
        default:
            ret = -1;
            break;
    }
    led_status = 0;
    return ret;
}

int paf_led_set_toggle(void)
{
    ESP_LOGI(__func__, "Toggling LED %d -> %d", led_status, !led_status);
    if (led_status) {
        return paf_led_set_off();
    }

    return paf_led_set_on();
}

int paf_led_set_dc(int duty_cycle)
{
    if (led_mode != PAF_LED_PWM) {
        return -1;
    }

    ledc_set_duty(ledc_channel.speed_mode, ledc_channel.channel,
                  duty_cycle);
    ledc_update_duty(ledc_channel.speed_mode, ledc_channel.channel);

    ESP_LOGI(__func__, "DC set to %d", duty_cycle);

    return 0;
}

int paf_led_get_dc(void)
{
    if (led_mode != PAF_LED_PWM) {
        return -1;
    }

    return ledc_get_duty(ledc_channel.speed_mode, ledc_channel.channel);
}

int paf_led_set_freq(int freq)
{
    if (led_mode != PAF_LED_PWM) {
        return -1;
    }

    ledc_set_freq(ledc_timer.speed_mode, ledc_timer.timer_num, freq);

    ESP_LOGI(__func__, "Freq set to %d", freq);

    return 0;
}

int paf_led_get_freq(void)
{
    if (led_mode != PAF_LED_PWM) {
        return -1;
    }

    return ledc_get_freq(ledc_timer.speed_mode, ledc_timer.timer_num);
}
