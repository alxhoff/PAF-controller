/**
 * @file paf_console.c
 * @author Alex Hoffman
 * @date 25 August 2020
 * @brief UART console to interact with the board
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

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_console.h"
#include "esp_vfs_dev.h"

#include "driver/uart.h"

#include "linenoise/linenoise.h"

#include "paf_util.h"
#include "paf_wifi.h"
#include "paf_flash.h"
#include "paf_config.h"

static xTaskHandle consoleHandle = NULL;

void register_commands(void)
{
    register_version();
    register_connect_wifi();
}

static void initialize_console(void)
{
    /* Disable buffering on stdin */
    setvbuf(stdin, NULL, _IONBF, 0);

    /* Minicom, screen, idf_monitor send CR when ENTER key is pressed */
    esp_vfs_dev_uart_set_rx_line_endings(ESP_LINE_ENDINGS_CR);
    /* Move the caret to the beginning of the next line on '\n' */
    esp_vfs_dev_uart_set_tx_line_endings(ESP_LINE_ENDINGS_CRLF);

    /* Install UART driver for interrupt-driven reads and writes */
    ESP_ERROR_CHECK(uart_driver_install(CONFIG_ESP_CONSOLE_UART_NUM, 256, 0,
                                        0, NULL, 0));

    /* Tell VFS to use UART driver */
    esp_vfs_dev_uart_use_driver(CONFIG_ESP_CONSOLE_UART_NUM);

    /* Initialize the console */
    esp_console_config_t console_config = {
        .max_cmdline_args = 32,
        .max_cmdline_length = 256,
#if CONFIG_LOG_COLORS
        .hint_color = atoi(LOG_COLOR_CYAN)
#endif
    };
    ESP_ERROR_CHECK(esp_console_init(&console_config));

    /* Configure linenoise line completion library */
    /* Enable multiline editing. If not set, long commands will scroll within
     * single line.
     */
    linenoiseSetMultiLine(1);

    /* Tell linenoise where to get command completions and hints */
    linenoiseSetCompletionCallback(&esp_console_get_completion);
    linenoiseSetHintsCallback(
        (linenoiseHintsCallback *)&esp_console_get_hint);

    /* Set command history size */
    linenoiseHistorySetMaxLen(100);
}

void console_task(void *pvParameters)
{
    const char *prompt = LOG_COLOR_I "esp32> " LOG_RESET_COLOR;

    printf("\n ==================================================\n");
    printf(" |                   PAF Console                  |\n");
    printf(" |                                                |\n");
    printf(" |     Print 'help' to gain overview of commands  |\n");
    printf(" |                                                |\n");
    printf(" =================================================\n\n");

    /* Figure out if the terminal supports escape sequences */
    int probe_status = linenoiseProbe();
    if (probe_status) { /* zero indicates success */
        printf("\n"
               "Your terminal application does not support escape sequences.\n"
               "Line editing and history features are disabled.\n"
               "On Windows, try using Putty instead.\n");
        linenoiseSetDumbMode(1);
#if CONFIG_LOG_COLORS
        /* Since the terminal doesn't support escape sequences,
         * don't use color codes in the prompt.
         */
        prompt = "esp32> ";
#endif //CONFIG_LOG_COLORS
    }

    /* Main loop */
    while (true) {
        /* Get a line using linenoise.
         * The line is returned when ENTER is pressed.
         */
        char *line = linenoise(prompt);
        if (line == NULL) { /* Ignore empty lines */
            continue;
        }
        /* Add the command to the history */
        linenoiseHistoryAdd(line);

        /* Try to run the command */
        int ret;
        esp_err_t err = esp_console_run(line, &ret);
        if (err == ESP_ERR_NOT_FOUND) {
            printf("Unrecognized command\n");
        }
        else if (err == ESP_OK && ret != ESP_OK) {
            printf("Command returned non-zero error code: 0x%x\n",
                   ret);
        }
        else if (err != ESP_OK) {
            printf("Internal error: %s\n", esp_err_to_name(err));
        }
        /* linenoise allocates line buffer on the heap, so need to free it */
        linenoiseFree(line);
    }
}

int paf_console_init(void)
{
    initialize_console();

    /* Register commands */
    esp_console_register_help_command();
    register_commands();

    xTaskCreatePinnedToCore(console_task, "console", PAF_CONSOLE_STACK,
                            NULL, PAF_CONSOLE_PRIORITY, &consoleHandle, PAF_CONSOLE_CORE);

    return 0;
}
