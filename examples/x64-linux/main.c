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
#include <gtk/gtk.h>
#include <stddef.h>
#include "user-screen.h"

#define WIDHT (128)
#define HEIGHT (64)
#define PIXELS_PER_PIXEL (5)

typedef struct display_user_screen {
    generic_display_t* display;
    user_screen_t* user_screen;
} display_user_screen_t;

generic_display_pixel_handler_t pixel;
generic_display_line_handler_t line;

static void display_reset(generic_display_t* display) {
    memset(display->context, 0, WIDHT * HEIGHT * sizeof(uint8_t));
}

static void display_update(generic_display_t* display) {}

static void display_destroy(generic_display_t* display) {
    free(display->context);
    free(display);
}

static void display_fill(generic_display_t* display, uint32_t color) {
    memset(display->context, color ? 1 : 0, WIDHT * HEIGHT * sizeof(uint8_t));
}

static void display_pixel(generic_display_t* display, uint32_t x, uint32_t y, uint32_t color) {
    uint8_t* buffer = display->context;
    buffer[y * display->info.width + x] = color;
}

static generic_display_t* create_display(void) {
    generic_display_t* gd = calloc(1, sizeof(*gd));
    gd->context = calloc(WIDHT * HEIGHT, sizeof(uint8_t));
    gd->info.color_type = GENERIC_DISPLAY_COLOR_1B;
    gd->info.width = 128;
    gd->info.height = 64;
    gd->reset = display_reset;
    gd->update = display_update;
    gd->destroy = display_destroy;
    gd->fill = display_fill;
    gd->pixel = display_pixel;
    return gd;
}

static void draw_cb(GtkDrawingArea* drawing_area, cairo_t* cr, int width, int height, gpointer data) {
    display_user_screen_t* dus = (display_user_screen_t*)data;
    uint8_t* buffer = (uint8_t*)dus->display->context;

    user_screen_redraw(dus->user_screen);

    for (int x = 0; x < WIDHT; x++) {
        for (int y = 0; y < HEIGHT; y++) {
            if (buffer[y * WIDHT + x]) {
                cairo_set_source_rgb(cr, 255.0, 255.0, 255.0);
            } else {
                cairo_set_source_rgb(cr, 0, 0, 0);
            }
            cairo_rectangle(cr, x * PIXELS_PER_PIXEL, y * PIXELS_PER_PIXEL, PIXELS_PER_PIXEL, PIXELS_PER_PIXEL);
            cairo_fill(cr);
        }
    }
}

static void activate_gtk_window(GtkApplication* app, gpointer user_data) {
    GtkWidget* window;

    window = gtk_application_window_new(app);
    gtk_window_set_default_size(GTK_WINDOW(window), WIDHT * PIXELS_PER_PIXEL, HEIGHT * PIXELS_PER_PIXEL);

    GtkWidget* drawing_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(drawing_area, WIDHT * PIXELS_PER_PIXEL, HEIGHT * PIXELS_PER_PIXEL);
    gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(drawing_area), draw_cb, user_data, NULL);

    gtk_window_set_child(GTK_WINDOW(window), drawing_area);

    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char** argv) {
    display_user_screen_t* dus = calloc(1, sizeof(*dus));
    dus->display = create_display();
    dus->user_screen = user_screen_create(dus->display);
    user_screen_window_t* w0 = user_screen_add_window(dus->user_screen, 0);
    user_screen_element_t* e0 = user_screen_element_create("Hello world", 30, 20);
    user_screen_add_element_to_window(w0, e0);
    user_screen_element_t* e1 = user_screen_element_create("User screen test", 10, 40);
    user_screen_add_element_to_window(w0, e1);

    GtkApplication* app;
    int status;

    app = gtk_application_new("org.gtk.example", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(activate_gtk_window), dus);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}