
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

#include "lwip/api.h"
#include "lwip/err.h"
#include "lwip/netdb.h"

#include "../webpages/index.h"

#include "paf_config.h"

static TaskHandle_t webserverHandle;

static void http_server_netconn_serve(struct netconn *conn)
{
    struct netbuf *inbuf;
    char *buf;
    u16_t buflen;
    err_t err;

    err = netconn_recv(conn, &inbuf);

    if (err == ERR_OK) {
        netbuf_data(inbuf, (void **)&buf, &buflen);

        char *first_line = strtok(buf, "\n");

        if (first_line) {
            // default page
            if ((buflen >= 5) && (strncmp(buf, "GET /", 5) == 0)) {
                if (strncmp((char const *)buf,
                            "GET /index.html", 15) == 0) {
                    netconn_write(conn,
                                  (const unsigned char *)
                                  index_html,
                                  index_html_len,
                                  NETCONN_NOCOPY);
                }
                if (strncmp((char const *)buf, "GET /led1",
                            9) == 0) {
                    //TODO toggle led
                }
                if (strncmp((char const *)buf, "GET /frequency",
                            14) == 0) {
                    //TODO set PWM freq
                }
                if (strncmp((char const *)buf, "GET /dutycycle",
                            14) == 0) {
                    //TODO set PWM duty cycle
                }
                if (strncmp((char const *)buf, "GET /status",
                            11) == 0) {
                    //TODO get LED status
                }
            }
        }
    }

    netconn_close(conn);
    netconn_delete(inbuf);
}

static void http_server_task(void *pvParameters)
{
    struct netconn *conn, *newconn;
    err_t err;

    conn = netconn_new(NETCONN_TCP);
    netconn_bind(conn, NULL, 80);
    netconn_listen(conn);
    printf("Webserver listening on port 80 (http)\n");

    do {
        err = netconn_accept(conn, &newconn);
        if (err == ERR_OK) {
            /** http_server_netconn_serve(newconn); */
            printf("connection\n");
            netconn_delete(newconn);
        }
        else {
            printf("Connection failed\n");
        }
        vTaskDelay(1);
    }
    while (err == ERR_OK);
}

int paf_webserver_init(void)
{
    if (xTaskCreatePinnedToCore(http_server_task, "webserver",
                                PAF_WEBSERVER_STACK, NULL,
                                PAF_WEBSERVER_PRIORITY, &webserverHandle,
                                PAF_WEBSERVER_CORE) != pdPASS) {
        return -1;
    }
    return 0;
}
