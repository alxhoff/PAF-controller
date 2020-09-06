/*
 * screen.c
 *
 *  Created on: Sep 16, 2017
 *      Author: alxhoff
 */
#include <stdlib.h>
#include <string.h>

#include "paf_config.h"

#include "screen.h"

#ifdef FREERTOS
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#endif


typedef struct screen_device {
    int rows;
    int cols;

#ifdef FREERTOS
    SemaphoreHandle_t framebuffer_lock;
    SemaphoreHandle_t cursor_lock;
    TaskHandle_t refresh_task;
    TimerHandle_t cursor_timer;
#endif

    char **framebuffer;

    int row_count;

    unsigned char cursor_on;
    int cursor_period;
    int cursor_location_x;
    int cursor_location_y;

    void (*draw_text)(char **, unsigned char, int, int);
    void (*clear_screen)(void);
    signed char (*update_screen)(void);
#ifdef SCREEN_USE_CURSOR
    void (*mv_cursor_left)(void);
    void (*mv_cursor_right)(void);
#endif
} screen_device_t;

screen_device_t screen_dev = { .cursor_period = SCREEN_CURSOR_PERIOD,
                               .draw_text = &SCREEN_DRAW,
                               .clear_screen = &SCREEN_CLEAR,
                               .update_screen = &SCREEN_REFRESH,
#ifdef SCREEN_USE_CURSOR
                               .mv_cursor_left = &SCREEN_MV_CUR_LEFT,
                               .mv_cursor_right = &SCREEN_MV_CUR_RIGHT
#endif
                             };

#ifdef FREERTOS
void screen_cursor_callback(TimerHandle_t timer)
{
    xSemaphoreTake(screen_dev.cursor_lock, portMAX_DELAY);
    screen_dev.cursor_on = !screen_dev.cursor_on;
    xSemaphoreGive(screen_dev.cursor_lock);
}
#endif

void screen_move_cursor_left(void)
{
    if (!screen_dev.cursor_location_x) {
        return;
    }

    screen_dev.cursor_location_x--;
}

char **screen_get_buffer(void)
{
    if (!screen_dev.framebuffer) {
        return NULL;
    }

    return screen_dev.framebuffer;
}

int screen_get_cursor_x(void)
{
    return screen_dev.cursor_location_x;
}

int screen_get_cursor_y(void)
{
    return screen_dev.cursor_location_y;
}

void screen_move_cursor_right(void)
{
    if (screen_dev.framebuffer && screen_dev.cursor_location_y < screen_dev.row_count)
        if (screen_dev.framebuffer[screen_dev.cursor_location_y]) {
            int max_len = strlen(screen_dev.framebuffer[screen_dev.cursor_location_y]);
            if (screen_dev.cursor_location_x >= max_len) {
                screen_dev.cursor_location_x = max_len - 1;
            }
            else {
                screen_dev.cursor_location_x++;
            }
        }
}

char *screen_get_framebuffer_line(unsigned char line)
{
    unsigned char rows = SCREEN_GET_ROWS;
    if (line < rows) {
        return screen_dev.framebuffer[line];
    }
    return NULL;
}

void screen_clear(void)
{
    for (unsigned char i = 0; i < screen_dev.rows; i++)
        memset(screen_dev.framebuffer[i], 0,
               sizeof(char) * (screen_dev.cols + 1));
}

void screen_refresh(void *args)
{
#ifdef FREERTOS
    TickType_t xLastWakeTime = xTaskGetTickCount();
    TickType_t xPeriod = 20;

    while (1) {
        xSemaphoreTake(screen_dev.framebuffer_lock, portMAX_DELAY);
        xSemaphoreTake(screen_dev.cursor_lock, portMAX_DELAY);
#endif

        screen_dev.clear_screen();

#ifdef SCREEN_USE_CURSOR

        screen_dev.draw_text(screen_dev.framebuffer, screen_dev.cursor_on,
                             screen_dev.cursor_location_x, screen_dev.cursor_location_y);

#else
        screen_dev.draw_text(screen_dev.framebuffer, 0, 0, 0);
#endif
        screen_dev.update_screen();
#ifdef FREERTOS
        xSemaphoreGive(screen_dev.cursor_lock);
        xSemaphoreGive(screen_dev.framebuffer_lock);

        xPeriod = SCREEN_PERIOD - (xLastWakeTime - xTaskGetTickCount());
        vTaskDelayUntil(&xLastWakeTime, xPeriod);
    }
#endif
}

signed char screen_add_line(char *line)
{
    screen_dev.framebuffer = realloc(screen_dev.framebuffer, sizeof(char) * (screen_dev.row_count + 1));

    if (!screen_dev.framebuffer) {
        return -1;
    }

    screen_dev.framebuffer[screen_dev.row_count] = malloc(sizeof(char) * (strlen(line) + 1));

    if (!screen_dev.framebuffer[screen_dev.row_count]) {
        return -1;
    }

    strcpy(screen_dev.framebuffer[screen_dev.row_count], line); \
    screen_dev.row_count++;

    return 0;
}

signed char screen_add_line_at_index(unsigned char index, char *line)
{
    if (index + 1 >= screen_dev.row_count) {
        screen_dev.framebuffer = realloc(screen_dev.framebuffer, sizeof(char *) * (index + 1));
        if (!screen_dev.framebuffer) {
            return -1;
        }
        for (int i = screen_dev.row_count; i <= index; i++) {
            screen_dev.framebuffer[i] = NULL;
        }
        screen_dev.row_count = index + 1;
    }

    screen_dev.framebuffer[index] = realloc(screen_dev.framebuffer[index], sizeof(char) * (strlen(line) + 1));

    if (!screen_dev.framebuffer[index]) {
        return -1;
    }

    strcpy(screen_dev.framebuffer[index], line); //CRASHES HERE

    return 0;
}

signed char screen_init(void)
{

    screen_dev.rows = SCREEN_GET_ROWS;
    screen_dev.cols = SCREEN_GET_COLS;

#ifdef FREERTOS
    screen_dev.cursor_timer = xTimerCreate("Cursor Timer",
                                           screen_dev.cursor_period, 1, NULL, screen_cursor_callback);
    if (!screen_dev.cursor_timer) {
        goto timer_error;
    }

    screen_dev.cursor_lock = xSemaphoreCreateMutex();
    if (!screen_dev.cursor_lock) {
        goto c_lock_error;
    }

    screen_dev.framebuffer_lock = xSemaphoreCreateMutex();
    if (!screen_dev.framebuffer_lock) {
        goto f_lock_error;
    }

    xTimerStart(screen_dev.cursor_timer, 0);
    xTaskCreate(screen_refresh, "screen", PAF_DEF_SCREEN_STACK, NULL, PAF_DEF_SCREEN_PRIORITY, screen_dev.refresh_task);
#endif
    return 0;

#ifdef FREERTOS
f_lock_error:
    vSemaphoreDelete(screen_dev.cursor_lock);
c_lock_error:
    xTimerDelete(screen_dev.cursor_timer, portMAX_DELAY);
timer_error:
    return -1;
#endif
}
