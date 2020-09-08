/*
 * screen.h
 *
 *  Created on: Sep 16, 2017
 *      Author: alxhoff
 */

#ifndef SCREEN_H_
#define SCREEN_H_

#include "screen_API.h"

#ifndef SCREEN_CURSOR_PERIOD
#define SCREEN_CURSOR_PERIOD (50)
#endif //SCREEN_CURSOR_PERIOD
#ifndef SCREEN_PERIOD
#define SCREEN_PERIOD (100)
#endif //SCREEN_PERIOD

int screen_get_cursor_x(void);
int screen_get_cursor_y(void);
void screen_move_cursor_left(void);
void screen_move_cursor_right(void);
void screen_move_cursor_up(void);
void screen_mode_cursor_down(void);

signed char screen_add_line(char *line);
signed char screen_add_line_at_index(unsigned char index, char *line);
signed char screen_replace_line(char *line);
signed char screen_replace_line_at_index(signed char index, char *line);

signed char screen_delete_line(void);
signed char screen_delete_line_at_index(unsigned char index);

//A simple function that write a string to the bottom line of the screen, equivalent to calling screen_add_line_at_index with the cursor_y row as the index
signed char screen_write_string(char *str);
//Writes a string to the current bottom line of the screen at a certain position in the current string.
signed char screen_write_string_at_pos(char *str, unsigned int pos);

void screen_log_fb(void);

signed char screen_init(void);

#endif /* SCREEN_H_ */
