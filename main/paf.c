
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

void app_main(void)
{

    paf_wifi_init();
    paf_console_init();

    while (1) {
        vTaskDelay(1000);
    }

}
