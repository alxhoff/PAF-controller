
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

static httpd_handle_t http_server = NULL;

const static char http_200_hdr[] = "200 OK";
const static char http_content_type_html[] = "text/html";

static esp_err_t http_server_get_root(httpd_req_t *req)
{
    ESP_LOGI(__func__, "GET root: %s", req->uri);
    httpd_resp_set_status(req, http_200_hdr);
    httpd_resp_set_type(req, http_content_type_html);
    httpd_resp_send(req, (const char *)index_html, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static esp_err_t http_server_get_btn1(httpd_req_t *req)
{
    ESP_LOGI(__func__, "GET btn1");
    return ESP_OK;
}

static esp_err_t http_server_get_led1(httpd_req_t *req)
{
    ESP_LOGI(__func__, "GET led1");
    return ESP_OK;
}

static esp_err_t http_server_get_freq(httpd_req_t *req)
{
    ESP_LOGI(__func__, "GET freq");
    return ESP_OK;
}

static esp_err_t http_server_get_set_freq(httpd_req_t *req)
{
    ESP_LOGI(__func__, "GET set freq");
    return ESP_OK;
}

static esp_err_t http_server_get_duty(httpd_req_t *req)
{
    ESP_LOGI(__func__, "GET duty");
    return ESP_OK;
}

static esp_err_t http_server_get_set_duty(httpd_req_t *req)
{
    ESP_LOGI(__func__, "GET set duty");
    return ESP_OK;
}

static esp_err_t http_server_get_status(httpd_req_t *req)
{
    ESP_LOGI(__func__, "GET status");
    return ESP_OK;
}

static esp_err_t http_server_post(httpd_req_t *req)
{
    return ESP_OK;
}

static const httpd_uri_t http_post_request = {
    .uri = "*",
    .method = HTTP_POST,
    .handler = http_server_post,
};

static const httpd_uri_t http_get_root = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = http_server_get_root,
    .user_ctx = NULL,
};

static const httpd_uri_t http_get_btn1 = {
    .uri = "/btn1",
    .method = HTTP_GET,
    .handler = http_server_get_led1,
    .user_ctx = NULL,
};

static const httpd_uri_t http_get_led1 = {
    .uri = "/led1",
    .method = HTTP_GET,
    .handler = http_server_get_led1,
    .user_ctx = NULL,
};

static const httpd_uri_t http_get_freq = {
    .uri = "/frequency",
    .method = HTTP_GET,
    .handler = http_server_get_freq,
    .user_ctx = NULL,
};

static const httpd_uri_t http_get_set_freq = {
    .uri = "/freq-set",
    .method = HTTP_GET,
    .handler = http_server_get_set_freq,
    .user_ctx = NULL,
};

static const httpd_uri_t http_get_duty = {
    .uri = "/dutycycle",
    .method = HTTP_GET,
    .handler = http_server_get_set_duty,
    .user_ctx = NULL,
};

static const httpd_uri_t http_get_set_duty = {
    .uri = "/dc-set",
    .method = HTTP_GET,
    .handler = http_server_get_duty,
    .user_ctx = NULL,
};
static const httpd_uri_t http_get_status = {
    .uri = "/status",
    .method = HTTP_GET,
    .handler = http_server_get_status,
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
                                       &http_get_root);
            httpd_register_uri_handler(http_server,
                                       &http_get_btn1);
            httpd_register_uri_handler(http_server,
                                       &http_get_led1);
            httpd_register_uri_handler(http_server,
                                       &http_get_freq);
            httpd_register_uri_handler(http_server,
                                       &http_get_set_freq);
            httpd_register_uri_handler(http_server,
                                       &http_get_duty);
            httpd_register_uri_handler(http_server,
                                       &http_get_set_duty);
            httpd_register_uri_handler(http_server,
                                       &http_get_status);
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
