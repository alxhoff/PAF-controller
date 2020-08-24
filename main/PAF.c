#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_wifi.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_console.h"
#include "nvs_flash.h"
#include "esp_vfs_dev.h"

#include "driver/uart.h"

#include "linenoise/linenoise.h"

#include "argtable3/argtable3.h"

#define WIFI_CONNECTED_BIT BIT0

#define WIFI_SSID "PAF"
#define WIFI_PASSWORD "paf"
#define WIFI_DEF_TIMEOUT (10000)

static EventGroupHandle_t wifi_event_group;

static void wifi_event_handler(void)
{
}

static void ip_event_handler(void)
{
}

static void initialize_wifi(void)
{
	// Creates an LwIP core task
	tcpip_adapter_init();

	wifi_event_group = xEventGroupCreate();
	// Create default event look for system events
	ESP_ERROR_CHECK(esp_event_loop_create_default());

	// Init WiFi driver resources
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));

	// Register wifi_event_handler to handle all events with base WIFI_EVENT
	// event_base, event_id, event_handler, event_args
	ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
						   &wifi_event_handler, NULL));
	ESP_ERROR_CHECK(esp_event_handler_register(
		IP_EVENT, IP_EVENT_STA_GOT_IP, &ip_event_handler, NULL));

	wifi_config_t wifi_config = {
		// Station
		.sta = { .ssid = WIFI_SSID,
			 .password = WIFI_PASSWORD,
			 .scan_method = WIFI_FAST_SCAN,
			 .sort_method = WIFI_CONNECT_AP_BY_SIGNAL,
			 .threshold.rssi = -127,
			 .threshold.authmode = WIFI_AUTH_OPEN },
	};
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
	ESP_ERROR_CHECK(esp_wifi_start());
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

static int get_version(int argc, char **argv)
{
	esp_chip_info_t info;
	esp_chip_info(&info);
	printf("IDF Version:%s\r\n", esp_get_idf_version());
	printf("Chip info:\r\n");
	printf("\tmodel:%s\r\n", info.model == CHIP_ESP32 ? "ESP32" : "Unknow");
	printf("\tcores:%d\r\n", info.cores);
	printf("\tfeature:%s%s%s%s%d%s\r\n",
	       info.features & CHIP_FEATURE_WIFI_BGN ? "/802.11bgn" : "",
	       info.features & CHIP_FEATURE_BLE ? "/BLE" : "",
	       info.features & CHIP_FEATURE_BT ? "/BT" : "",
	       info.features & CHIP_FEATURE_EMB_FLASH ? "/Embedded-Flash:" :
							"/External-Flash:",
	       spi_flash_get_chip_size() / (1024 * 1024), " MB");
	printf("\trevision number:%d\r\n", info.revision);
	return 0;
}

void register_version(void)
{
	const esp_console_cmd_t cmd = {
		.command = "version",
		.help = "Get version of chip and SDK",
		.hint = NULL,
		.func = &get_version,
	};
	ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

static bool wifi_join(const char *ssid, const char *pass, int timeout_ms)
{
	wifi_config_t cfg = { 0 };
	strlcpy((char *)cfg.sta.ssid, ssid, sizeof(cfg.sta.ssid));
	if (pass)
		strlcpy((char *)cfg.sta.password, pass,
			sizeof(cfg.sta.password));

	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &cfg));
	ESP_ERROR_CHECK(esp_wifi_connect());

	int bits = xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT,
				       pdFALSE, pdTRUE,
				       timeout_ms / portTICK_PERIOD_MS);

	return (bits & WIFI_CONNECTED_BIT) != 0;
}

static struct {
	struct arg_int *timeout;
	struct arg_str *ssid;
	struct arg_str *password;
	struct arg_end *end;
} network_details;

static int connect_wifi(int argc, char **argv)
{
	int ret = arg_parse(argc, argv, (void **)&network_details);
	if (ret) {
		arg_print_errors(stderr, network_details.end, argv[0]);
		return 1;
	}

	ESP_LOGI(__func__, "Connecting to '%s'", network_details.ssid->sval[0]);

	if (!network_details.timeout->count)
		network_details.timeout->ival[0] = WIFI_DEF_TIMEOUT;

	bool connected = wifi_join(network_details.ssid->sval[0],
				   network_details.password->sval[0],
				   network_details.timeout->ival[0]);

	if (!connected) {
		ESP_LOGW(__func__, "Connection timed out");
		return 1;
	}

	ESP_LOGI(__func__, "Connected");
	return 0;
}

void register_connect_wifi(void)
{
	network_details.timeout =
		arg_int0(NULL, "timeout", "<t>", "Connection timeout, ms");
	network_details.ssid = arg_str0(NULL, NULL, "<ssid>", "SSID of AP");
	network_details.password =
		arg_str1(NULL, NULL, "<password>", "PSK of AP");
	network_details.end = arg_end(2);
	const esp_console_cmd_t cmd = {
		.command = "connect",
		.help = "Connect to a WiFi network",
		.hint = NULL,
		.func = &connect_wifi,
		.argtable = &network_details,
	};
	ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

void register_commands(void)
{
	register_version();
	register_connect_wifi();
}

void app_main(void)
{
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
	    ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);

	initialize_wifi();
	initialize_console();

	/* Register commands */
	esp_console_register_help_command();
	register_commands();

	const char *prompt = LOG_COLOR_I "esp32> " LOG_RESET_COLOR;

	printf("\n ==================================================\n");
	printf(" |              Housemate Scanner                 |\n");
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
		} else if (err == ESP_OK && ret != ESP_OK) {
			printf("Command returned non-zero error code: 0x%x\n",
			       ret);
		} else if (err != ESP_OK) {
			printf("Internal error: %s\n", esp_err_to_name(err));
		}
		/* linenoise allocates line buffer on the heap, so need to free it */
		linenoiseFree(line);
	}
}
