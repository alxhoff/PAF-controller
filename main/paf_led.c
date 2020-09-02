
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

char paf_led_get_led(void)
{
    return led_status;
}

int paf_led_init(void)
{
    ledc_timer_config(&ledc_timer);

    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));

    ledc_fade_func_install(0);

    return 0;
}

int paf_led_set_on(void)
{
    /** esp_err_t ret; */
    ESP_LOGI(__func__, "Setting LED on");
    /** if ((ret = ledc_fade_start(ledc_channel.speed_mode, */
    /**                ledc_channel.channel, LEDC_FADE_NO_WAIT)) == */
    /**     ESP_OK) */
    led_status = 1;
    /** return ret; */
    return ESP_OK;
}

int paf_led_set_off(void)
{
    /** esp_err_t ret; */
    ESP_LOGI(__func__, "Setting LED off");
    /** if ((ret = ledc_stop(ledc_channel.speed_mode, ledc_channel.channel, */
    /**              0)) == ESP_OK) */
    led_status = 0;
    /** return ret; */
    return ESP_OK;
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
    ledc_set_duty(ledc_channel.speed_mode, ledc_channel.channel,
                  duty_cycle);
    ledc_update_duty(ledc_channel.speed_mode, ledc_channel.channel);

    ESP_LOGI(__func__, "DC set to %d", duty_cycle);

    return 0;
}

int paf_led_get_dc(void)
{
    return ledc_get_duty(ledc_channel.speed_mode, ledc_channel.channel);
}

int paf_led_set_freq(int freq)
{
    ledc_set_freq(ledc_timer.speed_mode, ledc_timer.timer_num, freq);

    ESP_LOGI(__func__, "Freq set to %d", freq);

    return 0;
}

int paf_led_get_freq(void)
{
    return ledc_get_freq(ledc_timer.speed_mode, ledc_timer.timer_num);
}
