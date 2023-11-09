/**
 * MIT License
 * Copyright (c) 2023 Grzegorz Grzęda
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef USER_SCREEN_H
#define USER_SCREEN_H

#include <stdbool.h>
#include <stdint.h>
#include "generic-display.h"

typedef struct user_screen user_screen_t;

typedef struct user_screen_window user_screen_window_t;

typedef struct user_screen_element user_screen_element_t;

typedef struct user_screen_box {
    uint32_t x0;
    uint32_t y0;
    uint32_t x1;
    uint32_t y1;
} user_screen_box_t;

typedef void (*user_screen_element_draw_handler_t)(generic_display_t* display, user_screen_element_t* me);

user_screen_t* user_screen_create(generic_display_t* display);
void user_screen_change_window(user_screen_t* screen, uint8_t next_window_id);
void user_screen_redraw(user_screen_t* screen);

user_screen_window_t* user_screen_add_window(user_screen_t* screen, uint8_t id);
void user_screen_add_element_to_window(user_screen_window_t* window, user_screen_element_t* element);

user_screen_element_t* user_screen_element_create(const char* text, uint32_t x, uint32_t y);
void user_screen_element_change_text(user_screen_element_t* element, const char* new_text);
void user_screen_element_change_position(user_screen_element_t* element, user_screen_box_t new_bounding_box);
void user_screen_element_change_visibility(user_screen_element_t* element, bool is_visible);

#endif  // USER_SCREEN_H