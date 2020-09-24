/**
 * @file paf_test.c
 * @author Alex Hoffman
 * @date 23 September 2020
 * @brief Testing utils
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

#include "paf_config.h"

struct test_config {
    unsigned int freq;
    unsigned int dc;
    unsigned int duration;
};

PAF_DEF_TESTS;

struct tests {
    unsigned int num_tests;
    unsigned int cur_test;
    struct test_config *tests;
} paf_test = { .num_tests = PAF_TEST_COUNT, .tests = paf_def_tests };

static unsigned char auto_skip = 1;
static unsigned int cur_time_remaining = 0;
static TaskHandle_t cur_test_task = NULL;

void paf_test_set_auto_skip(void)
{
    auto_skip = 1;
}

void paf_test_unset_auto_skip(void)
{
    auto_skip = 0;
}

unsigned int paf_test_get_test_count_total(void)
{
    return paf_test.num_tests;
}

unsigned int paf_test_get_cur_test(void)
{
    return paf_test.cur_test;
}

unsigned int paf_test_get_time_remaining(void)
{
    return cur_time_remaining;
}

void paf_test_stop_cur_test(void)
{
    if (cur_test_task) {
        vTaskDelete(cur_test_task);
    }
    cur_test_task = NULL;
}


static void wait_for_test(void *params)
{
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
        cur_time_remaining--;
        if (!cur_time_remaining) {
            if (auto_skip) {
                paf_test.cur_test++;
                paf_test.cur_test %= paf_test.num_tests;
            }
            ESP_LOGI(__func__, "Test task ending");
            paf_test_stop_cur_test();
        }
    }
}

void paf_test_next_test(void)
{
    paf_test.cur_test++;
    paf_test.cur_test %= paf_test.num_tests;
}

void paf_test_prev_test(void)
{
    if (paf_test.cur_test) {
        paf_test.cur_test--;
    }
}

void paf_test_pause_cur_test(void)
{
    if (cur_test_task) {
        vTaskSuspend(cur_test_task);
    }
}

void paf_test_resume_cur_test(void)
{
    if (cur_test_task) {
        vTaskResume(cur_test_task);
    }
}

unsigned int paf_test_get_cur_freq(void)
{
    return paf_test.tests[paf_test.cur_test].freq;
}

unsigned int paf_test_get_cur_dc(void)
{
    return paf_test.tests[paf_test.cur_test].dc;
}

unsigned int paf_test_get_cur_dur(void)
{
    return paf_test.tests[paf_test.cur_test].duration;
}

static esp_err_t paf_test_run_test(struct test_config *test)
{
    cur_time_remaining = test->duration;

    //TODO setup output according to test

    ESP_LOGI(__func__, "Creating test task");
    if (xTaskCreatePinnedToCore(wait_for_test, "test", 1024, NULL,
                                PAF_TEST_TASK_PRIORITY, &cur_test_task,
                                tskNO_AFFINITY) != pdPASS) {
        return ESP_FAIL;
    }

    ESP_LOGI(__func__, "Created");
    return ESP_OK;
}

esp_err_t paf_test_run_next_test(void)
{
    struct test_config *cur_test = &paf_test.tests[paf_test.cur_test];
    ESP_LOGI(__func__, "Running test #%d {freq: %d, dc: %d, dur: %d}",
             paf_test.cur_test, cur_test->freq, cur_test->dc,
             cur_test->duration);
    return paf_test_run_test(cur_test);
}
