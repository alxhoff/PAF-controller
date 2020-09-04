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

static esp_err_t ssd1306_write_start(void)
{
    if (!ssd1306_dev.i2c_cmd) {
        ssd1306_dev.i2c_cmd = i2c_cmd_link_create();
    }
    ESP_ERROR_CHECK(i2c_master_start(ssd1306_dev.i2c_cmd));
    return i2c_master_write_byte(ssd1306_dev.i2c_cmd,
                                 (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE,
                                 true);
}

static esp_err_t ssd1306_write_end(void)
{
    esp_err_t ret = i2c_master_stop(ssd1306_dev.i2c_cmd);
    i2c_cmd_link_delete(ssd1306_dev.i2c_cmd);
    ssd1306_dev.i2c_cmd = NULL;
    return ret;
}

static esp_err_t ssd1306_write_single_command(uint8_t command)
{
    if (ssd1306_dev.i2c_cmd) {
        ssd1306_write_start();
        ESP_ERROR_CHECK(i2c_master_write_byte(ssd1306_dev.i2c_cmd,
                                              command, true));
        ssd1306_write_end();
        return ESP_OK;
    }
    return ESP_FAIL;
}

static esp_err_t ssd1306_write_command(uint8_t command)
{
    if (ssd1306_dev.i2c_cmd) {
        ESP_ERROR_CHECK(i2c_master_write_byte(ssd1306_dev.i2c_cmd,
                                              command, true));
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
        ssd1306_write_start();
        ssd1306_write_command(OLED_CMD_PAGE_START_ADDR + i);
        ssd1306_write_command(OLED_CMD_PAGE_VERT_RIGHT);
        ssd1306_write_command(OLED_CONTROL_BYTE_CMD_STREAM);
        i2c_master_write(ssd1306_dev.i2c_cmd,
                         &ssd1306_dev.buffer[ssd1306_dev.width * i],
                         ssd1306_dev.width, true);
        ssd1306_write_end();
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

static esp_err_t ssd1306_i2c_init(void)
{
    esp_err_t ret;
    i2c_config_t i2c_config = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = PAF_DEF_OLED_SDA_PIN,
        .scl_io_num = PAF_DEF_OLED_SCL_PIN,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000,
    };
    ESP_ERROR_CHECK(i2c_set_pin(PAF_DEF_I2C_NUM, PAF_DEF_OLED_SDA_PIN,
                                PAF_DEF_OLED_SCL_PIN, true, true,
                                I2C_MODE_MASTER));
    ESP_ERROR_CHECK(i2c_param_config(PAF_DEF_I2C_NUM, &i2c_config));
    ret = i2c_driver_install(PAF_DEF_I2C_NUM, I2C_MODE_MASTER, 0, 0, 0);
    ESP_LOGI(__func__, "I2C Driver install: %s", esp_err_to_name(ret));
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(__func__, "SSD1306 I2C init'd");

    return ret;
}

signed char ssd1306_init(void)
{
    ESP_ERROR_CHECK(ssd1306_i2c_init());

    ssd1306_dev.clear = &ssd1306_clear;
    ssd1306_dev.update = &ssd1306_update_screen;
    ssd1306_dev.fill = &ssd1306_fill;
    ssd1306_dev.string = &ssd1306_write_string;

    ssd1306_dev.width = SSD1306_WIDTH;
    ssd1306_dev.height = SSD1306_HEIGHT;

    ssd1306_dev.background = SSD1306_BACKGROUND;
    ssd1306_dev.font = SSD1306_FONT;

    /* Init LCD */
    ssd1306_write_start();
    ssd1306_write_command(OLED_CMD_DISPLAY_OFF);
    ssd1306_write_command(OLED_CMD_SET_MEMORY_ADDR_MODE);
    ssd1306_write_command(OLED_CMD_PAGE_ADDR_MODE);
    ssd1306_write_command(OLED_CMD_PAGE_START_ADDR);
    ssd1306_write_command(OLED_CMD_SET_COM_SCAN_MODE);
    ssd1306_write_command(0x00); //---set low column address
    ssd1306_write_command(0x10); //---set high column address
    ssd1306_write_command(OLED_CONTROL_BYTE_DATA_STREAM);
    ssd1306_write_command(OLED_CMD_SET_CONTRAST);
    ssd1306_write_command(0xFF);
    ssd1306_write_command(OLED_CMD_SET_SEGMENT_REMAP);
    ssd1306_write_command(OLED_CMD_DISPLAY_NORMAL);
    ssd1306_write_command(OLED_CMD_SET_MUX_RATIO);
    ssd1306_write_command(0x3F);
    ssd1306_write_command(OLED_CMD_DISPLAY_RAM);
    ssd1306_write_command(OLED_CMD_SET_DISPLAY_OFFSET);
    ssd1306_write_command(0x00); //-not offset
    ssd1306_write_command(OLED_CMD_SET_DISPLAY_CLK_DIV);
    ssd1306_write_command(0xF0); //--set divide ratio
    ssd1306_write_command(OLED_CMD_SET_PRECHARGE);
    ssd1306_write_command(0x22);
    ssd1306_write_command(OLED_CMD_SET_COM_PIN_MAP);
    ssd1306_write_command(OLED_CMD_SET_COM_PIN_RESET);
    ssd1306_write_command(OLED_CMD_SET_VCOMH_DESELCT);
    ssd1306_write_command(OLED_CMD_SET_VCOMH_0V77);
    ssd1306_write_command(OLED_CMD_SET_CHARGE_PUMP);
    ssd1306_write_command(OLED_CMD_SET_CHARGE_PUMP_ENABLE);
    ssd1306_write_command(OLED_CMD_DISPLAY_ON);
    ssd1306_write_end();

    ssd1306_clear();

    ssd1306_dev.x = 0;
    ssd1306_dev.y = 0;

    ssd1306_dev.initialized = 1;

    ESP_LOGI(__func__, "SSD1306 init finished");

    return 0;
}
