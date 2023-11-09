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
#include "user-screen.h"
#include <stdlib.h>
#include <string.h>
#include "dynamic-list.h"
#include "font.h"
#define G2LABS_LOG_MODULE_NAME "user-screen"
#define G2LABS_LOG_MODULE_LEVEL (G2LABS_LOG_MODULE_LEVEL_INFO)
#include <g2labs-log.h>

typedef struct user_screen {
    generic_display_t* display;
    dynamic_list_t* windows;
    user_screen_window_t* current_window;
} user_screen_t;

typedef struct user_screen_window {
    uint8_t id;
    dynamic_list_t* elements;
    bool needs_redraw;
} user_screen_window_t;

typedef struct user_screen_element {
    user_screen_window_t* parent;
    uint32_t x;
    uint32_t y;
    const char* text;
    const font_t* font;
    bool is_visible;
    user_screen_element_draw_handler_t draw;
} user_screen_element_t;

user_screen_t* user_screen_create(generic_display_t* display) {
    if (!display) {
        return NULL;
    }
    user_screen_t* us = calloc(1, sizeof(*us));
    if (!us) {
        return NULL;
    }
    us->windows = dynamic_list_create();
    if (!us->windows) {
        free(us);
        return NULL;
    }
    display->reset(display);
    us->display = display;
    return us;
}

void user_screen_change_window(user_screen_t* screen, uint8_t next_window_id) {
    if (!screen) {
        return;
    }
    for (dynamic_list_iterator_t* it = dynamic_list_begin(screen->windows); it; it = dynamic_list_next(it)) {
        user_screen_window_t* window = dynamic_list_get(it);
        if (window->id == next_window_id) {
            screen->current_window->needs_redraw = false;
            window->needs_redraw = true;
            screen->current_window = window;
            screen->display->fill(screen->display, 0);
            return;
        }
    }
}

void user_screen_redraw(user_screen_t* screen) {
    if (!screen || !screen->current_window || !screen->current_window->needs_redraw) {
        return;
    }
    for (dynamic_list_iterator_t* it = dynamic_list_begin(screen->current_window->elements); it;
         it = dynamic_list_next(it)) {
        user_screen_element_t* element = dynamic_list_get(it);
        element->draw(screen->display, element);
    }
    screen->current_window->needs_redraw = false;
    screen->display->update(screen->display);
}

user_screen_window_t* user_screen_window_create(user_screen_t* screen, uint8_t id) {
    if (!screen) {
        return NULL;
    }
    user_screen_window_t* window = calloc(1, sizeof(*window));
    if (!window) {
        return NULL;
    }
    window->elements = dynamic_list_create();
    if (!window->elements) {
        free(window);
        return NULL;
    }
    window->id = id;
    dynamic_list_append(screen->windows, window);
    if (!screen->current_window) {
        screen->current_window = window;
    }
    return window;
}

void user_screen_window_add_element(user_screen_window_t* window, user_screen_element_t* element) {
    if (!window || !element) {
        return;
    }
    dynamic_list_append(window->elements, element);
    element->parent = window;
    element->parent->needs_redraw = true;
}

static void draw_element(generic_display_t* display, user_screen_element_t* me) {
    uint32_t x = me->x;
    uint32_t y = me->y;
    const char* ptr = me->text;
    while (*ptr) {
        for (uint8_t i = 0; i < me->font->width; i++) {
            for (uint8_t j = 0; j < me->font->height; j++) {
                uint8_t value = font_get_pixel(me->font, i, j, *ptr);
                display->pixel(display, x + i, y + j - me->font->height - 1, value);
            }
        }
        x += me->font->width + 1;
        ptr++;
    }
}

user_screen_element_t* user_screen_element_create(const char* text, uint32_t x, uint32_t y) {
    user_screen_element_t* element = calloc(1, sizeof(*element));
    if (!element) {
        return NULL;
    }
    element->text = text;
    element->is_visible = true;
    element->font = font_default();
    element->draw = draw_element;
    element->x = x;
    element->y = y;
    return element;
}

void user_screen_element_set_text(user_screen_element_t* element, const char* new_text) {
    if (!element || !new_text) {
        return;
    }
    element->text = new_text;
    size_t text_length = strlen(new_text);
    if (element->parent) {
        element->parent->needs_redraw = true;
    }
}

void user_screen_element_set_position(user_screen_element_t* element, uint32_t x, uint32_t y) {
    if (!element) {
        return;
    }
    element->x = x;
    element->y = y;
    element->parent->needs_redraw = true;
}

void user_screen_element_set_visibility(user_screen_element_t* element, bool is_visible) {
    if (!element) {
        return;
    }
    element->is_visible = is_visible;
    if (element->parent) {
        element->parent->needs_redraw = true;
    }
}

void user_screen_element_set_font(user_screen_element_t* element, const char* font_name) {
    if (!element || !font_name) {
        return;
    }
    element->font = font_get(font_name);
    if (element->parent) {
        element->parent->needs_redraw = true;
    }
}