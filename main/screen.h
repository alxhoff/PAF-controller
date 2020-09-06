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
#define SCREEN_CURSOR_PERIOD (500)
#endif //SCREEN_CURSOR_PERIOD
#ifndef SCREEN_PERIOD
#define SCREEN_PERIOD (100)
#endif //SCREEN_PERIOD

signed char screen_init(void);
char **screen_get_buffer(void);
int screen_get_cursor_x(void);
int screen_get_cursor_y(void);
signed char screen_add_line(char *line);
signed char screen_add_line_at_index(unsigned char index, char *line);
void screen_move_cursor_left(void);
void screen_move_cursor_right(void);

#endif /* SCREEN_H_ */
