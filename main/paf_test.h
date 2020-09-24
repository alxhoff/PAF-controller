/**
 * @file paf_test.H
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

#ifndef __PAF_TEST_H__
#define __PAF_TEST_H__

#include "esp_err.h"

unsigned int paf_test_get_test_count_total(void);
unsigned int paf_test_get_cur_test(void);
unsigned int paf_test_get_time_remaining(void);
void paf_test_next_test(void);
void paf_test_prev_test(void);
void paf_test_pause_cur_test(void);
void paf_test_resume_cur_test(void);
void paf_test_stop_cur_test(void);
esp_err_t paf_test_run_next_test(void);
void paf_test_set_auto_skip(void);
void paf_test_unset_auto_skip(void);
unsigned int paf_test_get_cur_freq(void);
unsigned int paf_test_get_cur_dc(void);
unsigned int paf_test_get_cur_dur(void);

#endif // __PAF_TEST_H__
