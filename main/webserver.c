
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "lwip/api.h"
#include "lwip/err.h"
#include "lwip/netdb.h"

const static char http_html_hdr[] = "HTTP/1.1 200 OK\nContent-type: text/html\n\n";

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

        if(first_line) {
			// default page
			if((bufflen >= 5) && (strncmp(buf, "GET /", 5) == 0)){

                if(strncmp((char const *)buf, "GET /index.html", 15) == 0){

                }
                if(strncmp((char const *)buf, "GET /led1", 9) == 0){

                }
                if(strncmp((char const *)buf, "GET /frequency", 14) == 0){

                }
                if(strncmp((char const *)buf, "GET /dutycycle", 14) == 0){

                }
                if(strncmp((char const *)buf, "GET /status", 11) == 0){

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

	do {
		err = netconn_accept(conn, &newconn);
		if (err = ERR_OK) {
			http_server_netconn_serve(newconn);
			netconn_delete(newconn);
		}
		vTaskDelay(1);
	} while (err == ERR_OK);
}
