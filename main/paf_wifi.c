
/**
 * @file pad_wifi.h
 * @author Alex Hoffman
 * @date 25 August 2020
 * @brief Wifi functionality of board
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
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_console.h"
#include "argtable3/argtable3.h"
#include "paf_config.h"
#include "paf_util.h"

#define WIFI_CONNECTED_BIT BIT0

static struct {
    struct arg_int *timeout;
    struct arg_str *ssid;
    struct arg_str *password;
    struct arg_end *end;
} network_details;

static char wifi_initd = 0;

static EventGroupHandle_t wifi_event_group;

static void wifi_event_handler(void)
{
}

static void ip_event_handler(void)
{
}

static void paf_wifi_init(void)
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
}

void paf_wifi_init_ap(void)
{
    if (!wifi_initd) {
        paf_wifi_init();
        wifi_initd = 1;
    }

    esp_wifi_set_storage(WIFI_STORAGE_RAM);

    char *ssid_mac = (char *)calloc(sizeof(char), (strlen(PAF_DEF_WIFI_SSID) + 13));

    strcpy(ssid_mac, PAF_DEF_WIFI_SSID);
    get_mac_string((char *)&ssid_mac[strlen(ssid_mac)]);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = PAF_DEF_WIFI_SSID,
            .channel = PAF_DEF_WIFI_CHANNEL,
            .authmode = WIFI_AUTH_OPEN,
            .ssid_hidden = 0,
            .max_connection = PAF_DEF_WIFI_AP_MAX_CON,
            .beacon_interval = 100,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}

void paf_wifi_init_station(const char *ssid, const char *passwd)
{
    if (!wifi_initd) {
        paf_wifi_init();
        wifi_initd = 1;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

    wifi_config_t wifi_config = {
        // Station
        .sta = {
            .ssid = { ssid },
            .password = { passwd },
            .scan_method = WIFI_FAST_SCAN,
            .sort_method = WIFI_CONNECT_AP_BY_SIGNAL,
            .threshold.rssi = -127,
            .threshold.authmode = WIFI_AUTH_OPEN
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
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

static int connect_wifi(int argc, char **argv)
{
    int ret = arg_parse(argc, argv, (void **)&network_details);
    if (ret) {
        arg_print_errors(stderr, network_details.end, argv[0]);
        return 1;
    }

    ESP_LOGI(__func__, "Connecting to '%s'", network_details.ssid->sval[0]);

    if (!network_details.timeout->count) {
        network_details.timeout->ival[0] = PAF_DEF_WIFI_TIMEOUT;
    }

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
