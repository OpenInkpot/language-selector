/*
 * Copyright © 2009,2010 Mikhail Gusarov <dottedmag@dottedmag.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */
#define _GNU_SOURCE

#include <libintl.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>

#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Ecore_X.h>
#include <Edje.h>

#include <libchoicebox.h>
#include <liblanguage.h>
#include <libeoi.h>

static int
exit_handler(void *param, int ev_type, void *event)
{
    ecore_main_loop_quit();
    return 1;
}

static void
main_win_close_handler(Ecore_Evas *main_win)
{
    ecore_main_loop_quit();
}

static void
exit_app(void *param)
{
    ecore_main_loop_quit();
}

static void
draw_handler(Evas_Object *choicebox, Evas_Object *item,
             int item_num, int page_position, void *param)
{
    languages_t *languages = param;
    language_t *lang = languages->langs + item_num;

    char *buf;
    if (lang->native_name)
        asprintf(&buf, "%s / %s", lang->native_name, lang->name);
    else
        buf = strdup(lang->name);

    edje_object_part_text_set(item, "text", buf);
    free(buf);
}

static void
page_handler(Evas_Object *choicebox, int cur_page, int total_pages, void *param)
{
    Evas *canvas = evas_object_evas_get(choicebox);
    Evas_Object *main_edje = evas_object_name_find(canvas, "main_edje");

    choicebox_aux_edje_footer_handler(main_edje, "footer", cur_page, total_pages);
}

static void
item_handler(Evas_Object *choicebox, int item_num, bool is_alt, void *param)
{
    languages_t *languages = param;
    language_t *lang = languages->langs + item_num;

    languages_set(languages, lang->internal_name);

    ecore_main_loop_quit();
}

static void
close_handler(Evas_Object *choicebox, void *param)
{
    ecore_main_loop_quit();
}

static void
run(languages_t *languages)
{
    ecore_event_handler_add(ECORE_EVENT_SIGNAL_EXIT, exit_handler, NULL);

    Ecore_Evas *main_win = ecore_evas_software_x11_new(0, 0, 0, 0, 600, 800);
    ecore_evas_title_set(main_win, "Language Selector");
    ecore_evas_name_class_set(main_win, "language-selector", "language-selector");

    Evas *main_canvas = ecore_evas_get(main_win);

    ecore_evas_callback_delete_request_set(main_win, main_win_close_handler);

    Evas_Object *main_edje = eoi_main_window_create(main_canvas);

    evas_object_name_set(main_edje, "main_edje");
    edje_object_part_text_set(main_edje, "title", "Select language");
    evas_object_move(main_edje, 0, 0);
    evas_object_resize(main_edje, 600, 800);
    evas_object_show(main_edje);

    choicebox_info_t info = {
        NULL,
        "choicebox",
        "full",
        "choicebox",
        "item-default",
        item_handler,
        draw_handler,
        page_handler,
        close_handler,
    };

    Evas_Object *choicebox = choicebox_new(main_canvas, &info, languages);

    choicebox_set_size(choicebox, languages->n);
    evas_object_name_set(choicebox, "choicebox");
    edje_object_part_swallow(main_edje, "contents", choicebox);
    evas_object_show(choicebox);

    eoi_register_fullscreen_choicebox(choicebox);

    evas_object_focus_set(choicebox, true);
    choicebox_aux_subscribe_key_up(choicebox);

    eoi_fullwindow_object_register(main_win, main_edje);

    ecore_evas_show(main_win);

    ecore_x_io_error_handler_set(exit_app, NULL);

    ecore_main_loop_begin();
}

int main(int argc, char **argv)
{
    if (!ecore_x_init(NULL))
        errx(1, "Unable to initialize Ecore_X, maybe missing DISPLAY");
    if (!ecore_init())
        errx(1, "Unable to initialize Ecore");
    if (!ecore_evas_init())
        errx(1, "Unable to initialize Ecore_Evas");
    if (!edje_init())
        errx(1, "Unable to initialize Edje\n");

    languages_t *languages = languages_get_supported();
    if (!languages)
        errx(1, "Unable to obtain languages list.");

    run(languages);

    languages_free(languages);

    edje_shutdown();
    ecore_evas_shutdown();
    ecore_shutdown();
    ecore_x_shutdown();
    return 0;
}
