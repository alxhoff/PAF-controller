
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
#include "esp_netif.h"
#include "argtable3/argtable3.h"
#include "paf_config.h"
#include "paf_util.h"
#include "paf_flash.h"
#include "lwip/netdb.h"

#define WIFI_CONNECTED_BIT BIT0

static char wifi_initd = 0;
static esp_netif_t *esp_netif_ap = NULL;
static wifi_config_t *wifi_config = NULL;
static EventGroupHandle_t wifi_event_group = NULL;

static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
}

static void ip_event_handler(void *arg, esp_event_base_t event_base,
                             int32_t event_id, void *event_data)
{
}

void paf_wifi_init_ap(void)
{
    if (!paf_flash_is_initd()) {
        paf_flash_init();
    }
    if (wifi_initd) {
        return;
    }

    wifi_config = (wifi_config_t *)calloc(1, sizeof(wifi_config_t));
    wifi_event_group = xEventGroupCreate();
    ESP_LOGI(__func__, "Wifi event group created");

    // Creates an LwIP core task
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_LOGI(__func__, "Netif init'd");

    // Create default event look for system events
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_LOGI(__func__, "Event loop created");

    esp_netif_ap = esp_netif_create_default_wifi_ap();

    // Init WiFi driver resources
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_LOGI(__func__, "Wifi init'd");
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_LOGI(__func__, "Storage set to RAM");

    //Event handles
    esp_event_handler_instance_t instance_wifi_event;
    esp_event_handler_instance_t instance_ip_event;

    // Register wifi_event_handler to handle all events with base WIFI_EVENT
    // event_base, event_id, event_handler, event_args
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
                        WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL,
                        &instance_wifi_event));
    ESP_LOGI(__func__, "Wifi event handler registered");
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
                        IP_EVENT, ESP_EVENT_ANY_ID, &ip_event_handler, NULL,
                        &instance_ip_event));
    ESP_LOGI(__func__, "IP event handler registered");

    char ssid_mac_str[32] = { 0 };
    strcpy((char *)ssid_mac_str, PAF_DEF_WIFI_SSID);
    strcat((char *)ssid_mac_str, "_");
    get_mac_string((char *)&ssid_mac_str[strlen(ssid_mac_str)]);

    wifi_config_t wifi_config = {
        .ap = {
            .ssid_len = 0,
            .ssid = PAF_DEF_WIFI_SSID,
            .channel = PAF_DEF_WIFI_CHANNEL,
            .authmode = WIFI_AUTH_OPEN,
            .ssid_hidden = 0,
            .max_connection = PAF_DEF_WIFI_AP_MAX_CON,
            .beacon_interval = 100,
        },
    };

    memcpy(wifi_config.ap.ssid, ssid_mac_str, sizeof(wifi_config.ap.ssid));
    ESP_LOGI(__func__, "SSID: %s\n", wifi_config.ap.ssid);

    esp_netif_dhcps_stop(esp_netif_ap);
    ESP_LOGI(__func__, "DHCP stopped");
    esp_netif_ip_info_t ap_ip_info;
    memset(&ap_ip_info, 0x00, sizeof(ap_ip_info));
    inet_pton(AF_INET, PAF_DEF_AP_IP, &ap_ip_info.ip);
    inet_pton(AF_INET, PAF_DEF_GATEWAY_IP, &ap_ip_info.gw);
    inet_pton(AF_INET, PAF_DEF_NETMASK, &ap_ip_info.netmask);
    ESP_LOGI(__func__, "Setting IP info");
    ESP_LOGI(__func__, "IP: %s", inet_ntoa(ap_ip_info.ip));
    ESP_LOGI(__func__, "GW: %s", inet_ntoa(ap_ip_info.gw));
    ESP_LOGI(__func__, "NM: %s", inet_ntoa(ap_ip_info.netmask));
    ESP_ERROR_CHECK(esp_netif_set_ip_info(esp_netif_ap, &ap_ip_info));
    ESP_LOGI(__func__, "IP info set");
    ESP_ERROR_CHECK(esp_netif_dhcps_start(esp_netif_ap));
    ESP_LOGI(__func__, "DHCP stared");
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
    ESP_LOGI(__func__, "Wifi mode set to APSTA");
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
    ESP_LOGI(__func__, "Wifi config'd");
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGI(__func__, "Wifi started");

    wifi_initd = 1;
}
