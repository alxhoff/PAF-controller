
/**
 * @file paf_webserver.c
 * @author Alex Hoffman
 * @date 25 August 2020
 * @brief Serves a basic webserver to allow fo web based control of the
 * LEDs
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
#include "esp_http_server.h"

#include "../webpages/index.h"

#include "paf_config.h"
#include "paf_led.h"

static httpd_handle_t http_server = NULL;

const static char http_200_hdr[] = "200 OK";
const static char http_content_type_html[] = "text/html";

const static char get_root[] = "/";
const static char get_btn[] = "led-toggle";
const static char get_led[] = "led-status";
const static char get_freq[] = "frequency";
const static char get_set_freq[] = "freq-set";
const static char get_dutycycle[] = "dutycycle";
const static char get_set_dutycycle[] = "dc-set";

static esp_err_t http_server_get_handler(httpd_req_t *req)
{
    ESP_LOGI(__func__, "GET %s", req->uri);

    //TODO captive portal

    httpd_resp_set_status(req, http_200_hdr);
    httpd_resp_set_type(req, http_content_type_html);
    //ROOT
    if ((strlen(req->uri) == 1) && (strcmp(req->uri, get_root) == 0)) {
        httpd_resp_send(req, (const char *)index_html,
                        HTTPD_RESP_USE_STRLEN);
    }
    else if (strlen(req->uri) > 1) {
        if (strcmp(req->uri + sizeof(char), get_btn) == 0) {
            ESP_LOGI(__func__, "Handling led btn");
            paf_led_set_toggle();
            httpd_resp_send(req, NULL, 0);
        }
        else if (strcmp(req->uri + sizeof(char), get_led) == 0) {
            ESP_LOGI(__func__, "Handling led");
            if (paf_led_get_led())
                httpd_resp_send(req, "ON",
                                HTTPD_RESP_USE_STRLEN);
            else
                httpd_resp_send(req, "OFF",
                                HTTPD_RESP_USE_STRLEN);
        }
        else if (strcmp(req->uri + sizeof(char), get_freq) == 0) {
            sprintf((char *)req->uri, "%d", paf_led_get_freq());
            ESP_LOGI(__func__, "Handling freq: %s", req->uri);
            httpd_resp_send(req, (const char *)req->uri,
                            HTTPD_RESP_USE_STRLEN);
        }
        else if (strcmp(req->uri + sizeof(char), get_dutycycle) ==
                 0) {
            sprintf((char *)req->uri, "%d", paf_led_get_dc());
            ESP_LOGI(__func__, "Handling dc");
            httpd_resp_send(req, (const char *)req->uri,
                            HTTPD_RESP_USE_STRLEN);
        }
        else {
            httpd_resp_send(req, NULL, 0);
        }
    }
    else {
        httpd_resp_send(req, NULL, 0);
    }
    return ESP_OK;
}

static esp_err_t http_server_post(httpd_req_t *req)
{
    static char content_buf[20];
    int ret;

    ESP_LOGI(__func__, "POST %s", req->uri);

    if (strlen(req->uri) > 1) {
        ret = httpd_req_recv(req, content_buf, req->content_len);
        if (ret <= 20) {
            content_buf[ret] = '\0';
            ESP_LOGI(__func__, "POST recv %d bytes", ret);
            if (ret > 0) {
                if (strcmp(req->uri + sizeof(char),
                           get_set_dutycycle) == 0) {
                    unsigned int new_dc =
                        (unsigned int)strtoul(
                            content_buf, NULL, 10);
                    ESP_LOGI(__func__,
                             "Handling set dc: %u", new_dc);
                    paf_led_set_dc(new_dc);
                    httpd_resp_send(req, "DC Set",
                                    HTTPD_RESP_USE_STRLEN);
                }
                else if (strcmp(req->uri + sizeof(char),
                                get_set_freq) == 0) {
                    unsigned int new_freq =
                        (unsigned int)strtoul(
                            content_buf, NULL, 10);
                    ESP_LOGI(__func__,
                             "Handling set freq: %u",
                             new_freq);
                    paf_led_set_freq(new_freq);
                    httpd_resp_send(req, "Freq Set",
                                    HTTPD_RESP_USE_STRLEN);
                }
            }
        }
    }
    return ESP_OK;
}

static const httpd_uri_t http_post_request = {
    .uri = "*",
    .method = HTTP_POST,
    .handler = http_server_post,
};

static const httpd_uri_t http_get_request = {
    .uri = "*",
    .method = HTTP_GET,
    .handler = http_server_get_handler,
    .user_ctx = NULL,
};

int paf_webserver_init(void)
{
    if (http_server == NULL) {
        httpd_config_t http_config = HTTPD_DEFAULT_CONFIG();
        http_config.uri_match_fn = httpd_uri_match_wildcard;

        if (httpd_start(&http_server, &http_config) == ESP_OK) {
            ESP_LOGI(__func__, "Webserver started");
            httpd_register_uri_handler(http_server,
                                       &http_get_request);
            ESP_LOGI(__func__, "Webverser GET handlers registered");
            httpd_register_uri_handler(http_server,
                                       &http_post_request);
            ESP_LOGI(__func__, "Webverser POST handler registered");
        }
        else {
            return -1;
        }
    }
    return 0;
}
