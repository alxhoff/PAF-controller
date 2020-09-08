
/**
 * @file paf.c
 * @author Alex Hoffman
 * @date 25 August 2020
 * @brief Main application
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

#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "paf_console.h"
#include "paf_flash.h"
#include "paf_wifi.h"
#include "paf_webserver.h"
#include "paf_led.h"
#include "paf_config.h"
#include "screen.h"

void app_main(void)
{
    vTaskDelay(pdMS_TO_TICKS(100));
    paf_wifi_init_ap();
    paf_webserver_init();
    paf_led_init(PAF_DEF_LED_MODE);
    screen_init();
    paf_console_init();

    screen_add_line("Hello world1");
    screen_add_line("Hello world2");
    screen_add_line("Hello world3");
    screen_add_line_at_index(1, "Init'd");
    screen_add_line_at_index(4, "Fourth");
    screen_log_fb();
    screen_delete_line();
    screen_log_fb();
    screen_delete_line_at_index(1);
    screen_delete_line_at_index(5);
    screen_log_fb();
    screen_replace_line("replaced");
    screen_log_fb();
    screen_replace_line_at_index(1, "also replaced");
    screen_log_fb();
    screen_replace_line_at_index(4, "fourth line replaced");
    screen_log_fb();
}
