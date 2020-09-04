/**
 * @file esp32_ssd1306.c
 * @author Alex Hoffman
 * @date 4 September 2020
 * @brief i2c driver for the SSD1306 OLED screen
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

#include "driver/gpio.h"
#include "driver/i2c.h"
#include "esp_err.h"
#include "esp_log.h"

#include "paf_config.h"

#include "esp32_ssd1306.h"
#include "fonts.h"

#define SSD1306_I2C_ADDR 0x78
#define SSD1306_BACKGROUND 0
#define SSD1306_FONT &Font_11x18

#define SSD1306_WIDTH 128
#define SSD1306_CHAR_WIDTH ssd1306_dev.font->FontWidth
#define SSD1306_WIDTH_CHARS SSD1306_WIDTH / SSD1306_CHAR_WIDTH

#define SSD1306_HEIGHT 64
#define SSD1306_CHAR_HEIGHT ssd1306_dev.font->FontHeight
#define SSD1306_HEIGHT_CHARS SSD1306_HEIGHT / SSD1306_CHAR_HEIGHT

#define SSD1306_X_OFFSET 5
#define SSD1306_Y_OFFSET 5

typedef enum {
    Black = 0x00, /*!< Black color, no pixel */
    White = 0x01 /*!< Pixel is set. Color depends on LCD */
} SSD1306_colour_t;

typedef struct ssd1306_device ssd1306_device_t;

struct ssd1306_device {
    uint16_t x;
    uint16_t y;
    unsigned char cursor_pos;

    uint8_t initialized;

    SSD1306_colour_t background;

    FontDef *font;

    uint8_t width;
    uint8_t height;

    uint8_t buffer[SSD1306_WIDTH * SSD1306_HEIGHT / 8];

    i2c_cmd_handle_t i2c_cmd;

    void (*clear)(void);
    signed char (*update)(void);
    void (*fill)(void);
    void (*string)(char *);
};

ssd1306_device_t ssd1306_dev = { 0 };

unsigned char ssd1306_get_cols(void)
{
    return SSD1306_WIDTH_CHARS;
}

unsigned char ssd1306_get_rows(void)
{
    return SSD1306_HEIGHT_CHARS;
}

static esp_err_t ssd1306_write_command(uint8_t command)
{
    if (ssd1306_dev.i2c_cmd) {
        ESP_ERROR_CHECK(i2c_master_start(ssd1306_dev.i2c_cmd));
        i2c_master_write_byte(
            ssd1306_dev.i2c_cmd,
            (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
        ESP_ERROR_CHECK(i2c_master_write_byte(ssd1306_dev.i2c_cmd,
                                              command, true));
        ESP_ERROR_CHECK(i2c_master_stop(ssd1306_dev.i2c_cmd));
        return ESP_OK;
    }

    return ESP_FAIL;
}

void ssd1306_fill(void)
{
    for (int i = 0; i < sizeof(ssd1306_dev.buffer); i++) {
        ssd1306_dev.buffer[i] =
            (ssd1306_dev.background == Black) ? 0xFF : 0x00;
    }
}

signed char ssd1306_update_screen(void)
{
    uint8_t i;

    for (i = 0; i < 8; i++) {
        i2c_master_start(ssd1306_dev.i2c_cmd);
        i2c_master_write_byte(
            ssd1306_dev.i2c_cmd,
            (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
        i2c_master_write_byte(ssd1306_dev.i2c_cmd,
                              OLED_CMD_PAGE_START_ADDR + i, true);
        i2c_master_write_byte(ssd1306_dev.i2c_cmd,
                              OLED_CMD_PAGE_VERT_RIGHT, true);
        i2c_master_write_byte(ssd1306_dev.i2c_cmd,
                              OLED_CONTROL_BYTE_CMD_STREAM, true);
        i2c_master_write(ssd1306_dev.i2c_cmd,
                         &ssd1306_dev.buffer[ssd1306_dev.width * i],
                         ssd1306_dev.width, true);
        i2c_master_stop(ssd1306_dev.i2c_cmd);
    }
    return 0;
}

void ssd1306_clear(void)
{
    ssd1306_fill();
    ssd1306_update_screen();
}

void ssd1306_set_pixel(unsigned char x, unsigned char y)
{
    ssd1306_dev.x = x;
    ssd1306_dev.y = y;
}

void ssd1306_set_draw_cursor(int x, int y)
{
    if (x >= (SSD1306_WIDTH -
              SSD1306_CHAR_WIDTH)) { /* X greater than screen width */
        ssd1306_dev.x = SSD1306_WIDTH - SSD1306_CHAR_WIDTH;
    }
    else if (x < SSD1306_X_OFFSET) {
        ssd1306_dev.x = SSD1306_X_OFFSET;
    }
    else {
        ssd1306_dev.x = x;
    }

    if (y > (SSD1306_HEIGHT - SSD1306_CHAR_HEIGHT)) {
        ssd1306_dev.y = SSD1306_HEIGHT - SSD1306_CHAR_HEIGHT;
    }
    else if (y < SSD1306_Y_OFFSET) {
        ssd1306_dev.y = SSD1306_Y_OFFSET;
    }
    else {
        ssd1306_dev.y = y;
    }
}

void ssd1306_mv_cursor_left(void)
{
    if (ssd1306_dev.cursor_pos > 0) {
        ssd1306_dev.cursor_pos--;
    }
}

void ssd1306_mv_cursor_right(void)
{
    if (ssd1306_dev.cursor_pos < SSD1306_WIDTH_CHARS) {
        ssd1306_dev.cursor_pos++;
    }
}

signed char ssd1306_draw_pixel(uint8_t x, uint8_t y, SSD1306_colour_t colour)
{
    if (x >= ssd1306_dev.width || y >= ssd1306_dev.height) {
        return -1;
    }

    if (colour == Black) {
#if SCREEN_INVERTED
        ssd1306_dev.buffer[(SSD1306_WIDTH * SSD1306_HEIGHT / 8) -
                                                                (x + (y / 8) * ssd1306_dev.width)] |=
                               1 << (7 - (y % 8));
    }
    else {
        ssd1306_dev.buffer[(SSD1306_WIDTH * SSD1306_HEIGHT / 8) -
                                                                (x + (y / 8) * ssd1306_dev.width)] &=
                               ~(1 << (7 - (y % 8)));
#else
        ssd1306_dev.buffer[x + (y / 8) * ssd1306_dev.width] |=
            1 << (y % 8);
    }
    else {
        ssd1306_dev.buffer[x + (y / 8) * ssd1306_dev.width] &=
            ~(1 << (y % 8));
#endif
    }

    return 0;
}

signed char ssd1306_invert_pixel(uint8_t x, uint8_t y)
{
    if (x >= ssd1306_dev.width || y >= ssd1306_dev.height) {
        return -1;
    }
#if SCREEN_INVERTED
    ssd1306_dev.buffer[(SSD1306_WIDTH * SSD1306_HEIGHT / 8) -
                                                            (x + (y / 8) * ssd1306_dev.width)] ^=
                           1 << (7 - (y % 8));
    ;
#else
    ssd1306_dev.buffer[x + (y / 8) * ssd1306_dev.width] ^= 1 << (y % 8);
    ;
#endif
    return 0;
}

void ssd1306_write_char(char ch)
{
    static unsigned short b;

    if (ssd1306_dev.width <=
        (ssd1306_dev.x +
         ssd1306_dev.font
         ->FontWidth) /* would print outside of bounds */
        || ssd1306_dev.height <=
        (ssd1306_dev.y + ssd1306_dev.font->FontHeight)) {
        return;
    }

    for (unsigned char i = 0; i < ssd1306_dev.font->FontHeight; i++) {
        b = ssd1306_dev.font
            ->data[(ch - 32) * ssd1306_dev.font->FontHeight + i];
        for (unsigned char j = 0; j < ssd1306_dev.font->FontWidth; j++)
            if ((b << j) & 0x8000)
                ssd1306_draw_pixel(
                    ssd1306_dev.x + j, (ssd1306_dev.y + i),
                    (SSD1306_colour_t)!ssd1306_dev
                    .background);
            else
                ssd1306_draw_pixel(
                    ssd1306_dev.x + j, (ssd1306_dev.y + i),
                    (SSD1306_colour_t)
                    ssd1306_dev.background);
    }

    ssd1306_dev.x += ssd1306_dev.font->FontWidth;
}

signed char ssd1306_invert_box(unsigned char x, unsigned char y)
{
    for (unsigned char i = x; i < x + SSD1306_CHAR_WIDTH + 1; i++)
        for (unsigned char j = y; j < y + SSD1306_CHAR_HEIGHT; j++)
            if (ssd1306_invert_pixel(i, j) != 0) {
                return -1;
            }
    return 0;
}

void ssd1306_write_string(char *str)
{
    int chars_to_fit = (SSD1306_WIDTH - ssd1306_dev.x) / SSD1306_CHAR_WIDTH;
    int i = 0;

    while (*str && i++ < chars_to_fit) {
        ssd1306_write_char(*str);
        str++;
    }
}

void ssd1306_draw_framebuffer(char **buf, int cursor_x)
{
    int offset = 0;

    if (cursor_x >= SSD1306_WIDTH_CHARS) {
        offset += cursor_x - SSD1306_WIDTH_CHARS;
    }

    for (unsigned char i = 0; i < SSD1306_HEIGHT_CHARS; i++) {
        ssd1306_set_draw_cursor(SSD1306_X_OFFSET,
                                SSD1306_Y_OFFSET +
                                i * SSD1306_CHAR_HEIGHT);
        ssd1306_write_string(buf[i] + offset);
    }
}

void ssd1306_draw_cursor(unsigned char state, int x, int y)
{
    if (state) {
        if (x >= SSD1306_WIDTH_CHARS) {
            x = SSD1306_WIDTH_CHARS - 1;
        }
        ssd1306_invert_box(
            SSD1306_X_OFFSET + SSD1306_CHAR_WIDTH * x - 1,
            SSD1306_Y_OFFSET + SSD1306_CHAR_HEIGHT * y - 1);
    }
}

void ssd1306_refresh(char **buf, unsigned char cursor_on, int cursor_x,
                     int cursor_y)
{
    ssd1306_draw_framebuffer(buf, cursor_x);
    ssd1306_draw_cursor(cursor_on, cursor_x, cursor_y);
}

static void ssd1306_i2c_init(void)
{
    i2c_config_t i2c_config = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = PAF_DEF_OLED_SDA_PIN,
        .scl_io_num = PAF_DEF_OLED_SCL_PIN,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 1000000,
    };
    i2c_param_config(PAF_DEF_I2C_NUM, &i2c_config);
    i2c_driver_install(PAF_DEF_I2C_NUM, I2C_MODE_MASTER, 0, 0, 0);

    ssd1306_dev.i2c_cmd = i2c_cmd_link_create();

    i2c_master_start(ssd1306_dev.i2c_cmd);
}

signed char ssd1306_init(void)
{
    ssd1306_i2c_init();

    //functions
    ssd1306_dev.clear = &ssd1306_clear;
    ssd1306_dev.update = &ssd1306_update_screen;
    ssd1306_dev.fill = &ssd1306_fill;
    ssd1306_dev.string = &ssd1306_write_string;

    ssd1306_dev.width = SSD1306_WIDTH;
    ssd1306_dev.height = SSD1306_HEIGHT;

    ssd1306_dev.background = SSD1306_BACKGROUND;
    ssd1306_dev.font = SSD1306_FONT;

    /* Init LCD */
    ssd1306_write_command(OLED_CMD_DISPLAY_OFF); //display off
    ssd1306_write_command(
        OLED_CMD_SET_MEMORY_ADDR_MODE); //memory addressing mode
    ssd1306_write_command(
        OLED_CMD_PAGE_ADDR_MODE); //00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
    ssd1306_write_command(
        OLED_CMD_PAGE_START_ADDR); //Set Page Start Address for Page Addressing Mode,0-7
    ssd1306_write_command(0xC8); //Set COM Output Scan Direction
    ssd1306_write_command(0x00); //---set low column address
    ssd1306_write_command(0x10); //---set high column address
    ssd1306_write_command(0x40); //--set start line address
    ssd1306_write_command(0x81); //--set contrast control register
    ssd1306_write_command(0xFF);
    ssd1306_write_command(0xA1); //--set segment re-map 0 to 127
    ssd1306_write_command(0xA6); //--set normal display
    ssd1306_write_command(0xA8); //--set multiplex ratio(1 to 64)
    ssd1306_write_command(0x3F); //
    ssd1306_write_command(
        0xA4); //0xa4,Output follows RAM content;0xa5,Output ignores RAM content
    ssd1306_write_command(0xD3); //-set display offset
    ssd1306_write_command(0x00); //-not offset
    ssd1306_write_command(
        0xD5); //--set display clock divide ratio/oscillator frequency
    ssd1306_write_command(0xF0); //--set divide ratio
    ssd1306_write_command(0xD9); //--set pre-charge period
    ssd1306_write_command(0x22); //
    ssd1306_write_command(0xDA); //--set com pins hardware configuration
    ssd1306_write_command(0x12);
    ssd1306_write_command(0xDB); //--set vcomh
    ssd1306_write_command(0x20); //0x20,0.77xVcc
    ssd1306_write_command(0x8D); //--set DC-DC enable
    ssd1306_write_command(0x14); //
    ssd1306_write_command(0xAF); //--turn on SSD1306 panel

    ssd1306_clear();

    ssd1306_dev.x = 0;
    ssd1306_dev.y = 0;

    ssd1306_dev.initialized = 1;

    return 0;
}