
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
#include "../webpages/bootstrap.h"
#include "../webpages/jquery.h"

#include "paf_config.h"
#include "paf_led.h"
#include "paf_gpio.h"
#include "paf_test.h"

static httpd_handle_t http_server = NULL;

const static char http_200_hdr[] = "200 OK";
const static char http_content_type_html[] = "text/html";

const static char get_root[] = "/";
const static char get_bootstrap_css[] = "bootstrap.min.css";
const static char get_jquery[] = "jquery.min.js";
const static char get_btn_test_start[] = "btn-test-start";
const static char get_btn_test_stop[] = "btn-test-stop";
const static char get_btn_next[] = "btn-next";
const static char get_btn_prev[] = "btn-prev";
const static char get_test_status[] = "test-status";
const static char get_test_count_total[] = "get_test_count_total";
const static char get_test_number[] = "get_test_num";
const static char get_test_freq[] = "get_test_freq";
const static char get_test_dc[] = "get_test_dc";
const static char get_test_dur[] = "get_test_dur";
const static char get_freq[] = "get_frequency";
const static char get_time_remaining[] = "get_time_remaining";
const static char get_onDuration[] = "get_duration";
const static char get_set_onDuration[] = "duration-set";
const static char get_set_freq[] = "freq-set";
const static char get_dutycycle[] = "get_dutycycle";
const static char get_set_dutycycle[] = "dc-set";
const static char get_set_GPIO[] = "GPIO-set";
const static char post_auto_check[] = "auto-set";

int dutyCyclePercentToCounter(int duty_per)
{
    return (int)((float)duty_per * 8191) / 100;
}

int dutyCycleCounterToPercent(int duty_cnt)
{
    return (int)((float)duty_cnt * 100) / 8191;
}

static esp_err_t http_server_get_handler(httpd_req_t *req)
{
    ESP_LOGI(__func__, "GET %s", req->uri);

    //TODO captive portal

    httpd_resp_set_status(req, http_200_hdr);
    httpd_resp_set_type(req, http_content_type_html);
    //ROOT
    if ((strlen(req->uri) == 1) && (strcmp(req->uri, get_root) == 0)) {
        ESP_LOGI(__func__, "Handling GET root");
        httpd_resp_send(req, (const char *)index_html,
                        HTTPD_RESP_USE_STRLEN);
        ESP_LOGI(__func__, "index.html sent");
    }
    else if (strlen(req->uri) > 1) {
        if (strcmp(req->uri + sizeof(char), get_btn_test_start) == 0) {
            ESP_LOGI(__func__, "Handling test start");
            paf_test_run_next_test();
            httpd_resp_send(req, NULL, 0);
        }
        else if (strcmp(req->uri + sizeof(char), get_bootstrap_css) ==
                 0) {
            httpd_resp_send(req, (const char *)bootstrap_min_css,
                            HTTPD_RESP_USE_STRLEN);
            ESP_LOGI(__func__, "bootstrap.min.css sent");
        }
        else if (strcmp(req->uri + sizeof(char), get_jquery) ==
                 0) {
            httpd_resp_send(req, (const char *)jquery_min_js,
                            HTTPD_RESP_USE_STRLEN);
            ESP_LOGI(__func__, "jquery.min.js sent");
        }
        else if (strcmp(req->uri + sizeof(char), get_btn_test_stop) ==
                 0) {
            paf_test_stop_cur_test();
            ESP_LOGI(__func__, "Handling test stop");
        }
        else if (strcmp(req->uri + sizeof(char), get_btn_next) == 0) {
            paf_test_next_test();
            ESP_LOGI(__func__, "Handling btn next");
        }
        else if (strcmp(req->uri + sizeof(char), get_btn_prev) == 0) {
            paf_test_prev_test();
            ESP_LOGI(__func__, "Handling btn prev");
        }
        else if (strcmp(req->uri + sizeof(char),
                        get_time_remaining) == 0) {
            sprintf((char *)req->uri, "%d",
                    paf_test_get_time_remaining());
            ESP_LOGI(__func__, "Handling get time remaining: %s",
                     req->uri);
            httpd_resp_send(req, (const char *)req->uri,
                            HTTPD_RESP_USE_STRLEN);
        }
        else if (strcmp(req->uri + sizeof(char), get_test_status) ==
                 0) {
            ESP_LOGI(__func__, "Handling test status");
            if (paf_test_get_time_remaining())
                httpd_resp_send(req, "RUNNING",
                                HTTPD_RESP_USE_STRLEN);
            else
                httpd_resp_send(req, "STOPPED",
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
            sprintf((char *)req->uri, "%d",
                    dutyCycleCounterToPercent(paf_led_get_dc()));
            ESP_LOGI(__func__, "Handling dc: %s", req->uri);
            httpd_resp_send(req, (const char *)req->uri,
                            HTTPD_RESP_USE_STRLEN);
        }
        else if (strcmp(req->uri + sizeof(char), get_onDuration) ==
                 0) {
            sprintf((char *)req->uri, "%d", paf_led_get_time());
            ESP_LOGI(__func__, "Handling on duration: %s",
                     req->uri);
            httpd_resp_send(req, (const char *)req->uri,
                            HTTPD_RESP_USE_STRLEN);
        }
        else if (strcmp(req->uri + sizeof(char),
                        get_test_count_total) == 0) {
            sprintf((char *)req->uri, "%d",
                    paf_test_get_test_count_total());
            ESP_LOGI(__func__, "Handling get total test count: %s",
                     req->uri);
            httpd_resp_send(req, (const char *)req->uri,
                            HTTPD_RESP_USE_STRLEN);
        }
        else if (strcmp(req->uri + sizeof(char), get_test_number) ==
                 0) {
            sprintf((char *)req->uri, "%d",
                    paf_test_get_cur_test());
            ESP_LOGI(__func__, "Handling get test num: %s",
                     req->uri);
            httpd_resp_send(req, (const char *)req->uri,
                            HTTPD_RESP_USE_STRLEN);
        }
        else if (strcmp(req->uri + sizeof(char), get_test_freq) ==
                 0) {
            sprintf((char *)req->uri, "%d",
                    paf_test_get_cur_freq());
            ESP_LOGI(__func__, "Handling get test freq: %s",
                     req->uri);
            httpd_resp_send(req, (const char *)req->uri,
                            HTTPD_RESP_USE_STRLEN);
        }
        else if (strcmp(req->uri + sizeof(char), get_test_dc) ==
                 0) {
            sprintf((char *)req->uri, "%d",
                    paf_test_get_cur_dc());
            ESP_LOGI(__func__, "Handling get test dc: %s",
                     req->uri);
            httpd_resp_send(req, (const char *)req->uri,
                            HTTPD_RESP_USE_STRLEN);
        }
        else if (strcmp(req->uri + sizeof(char), get_test_dur) ==
                 0) {
            sprintf((char *)req->uri, "%d",
                    paf_test_get_cur_dur());
            ESP_LOGI(__func__, "Handling get test dur: %s",
                     req->uri);
            httpd_resp_send(req, (const char *)req->uri,
                            HTTPD_RESP_USE_STRLEN);
        }
        else {
            ESP_LOGI(__func__, "Unhandled GET");
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
                    paf_led_set_dc(
                        dutyCyclePercentToCounter(
                            new_dc));
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
                else if (strcmp(req->uri + sizeof(char),
                                post_auto_check) == 0) {
                    unsigned int auto_check =
                        (unsigned int)strtoul(
                            content_buf, NULL, 10);
                    ESP_LOGI(__func__,
                             "Handling post auto-check: %u",
                             auto_check);
                    if (auto_check) {
                        paf_test_set_auto_skip();
                    }
                    else {
                        paf_test_unset_auto_skip();
                    }
                }
                else if (strcmp(req->uri + sizeof(char),
                                get_set_onDuration) == 0) {
                    unsigned int new_onTime =
                        (unsigned int)strtoul(
                            content_buf, NULL, 10);
                    ESP_LOGI(__func__,
                             "Handling set on-duration: %u",
                             new_onTime);
                    paf_led_set_time(new_onTime);
                    httpd_resp_send(req, "Duration Set",
                                    HTTPD_RESP_USE_STRLEN);
                }
                else if (strcmp(req->uri + sizeof(char),
                                get_set_GPIO) == 0) {
                    unsigned int GPIO_Pin =
                        (unsigned int)strtoul(
                            content_buf, NULL, 10);
                    ESP_LOGI(__func__,
                             "GPIO Pin %u toggled",
                             GPIO_Pin);
                    paf_gpio_toggle_state(GPIO_Pin);
                    httpd_resp_send(req, "GPIO Toggled",
                                    HTTPD_RESP_USE_STRLEN);
                }
                else {
                    ESP_LOGI(__func__, "Unhandled POST");
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
