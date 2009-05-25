#define _GNU_SOURCE

#include <libintl.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Ecore_X.h>
#include <Edje.h>

#include <echoicebox.h>

#include <language.h>

static void die(const char* fmt, ...)
{
   va_list ap;
   va_start(ap, fmt);
   vfprintf(stderr, fmt, ap);
   va_end(ap);
   exit(EXIT_FAILURE);
}

static int exit_handler(void* param, int ev_type, void* event)
{
   ecore_main_loop_quit();
   return 1;
}

static void main_win_close_handler(Ecore_Evas* main_win)
{
   ecore_main_loop_quit();
}

static void exit_app(void* param)
{
    ecore_main_loop_quit();
}

static void draw_handler(Evas_Object* choicebox, Evas_Object* item,
                         int item_num, int page_position, void* param)
{
    languages_t* languages = param;
    language_t* lang = languages->langs + item_num;

    char* buf;
    if(lang->native_name)
        asprintf(&buf, "%s / %s", lang->native_name, lang->name);
    else
        buf = strdup(lang->name);

    edje_object_part_text_set(item, "text", buf);
    free(buf);
}

static void page_handler(Evas_Object* choicebox, int cur_page, int total_pages,
                         void* param)
{
    Evas* canvas = evas_object_evas_get(choicebox);
    Evas_Object* main_edje = evas_object_name_find(canvas, "main_edje");

    choicebox_aux_edje_footer_handler(main_edje, "footer", cur_page, total_pages);
}

static void item_handler(Evas_Object* choicebox, int item_num, bool is_alt,
                         void* param)
{
    languages_t* languages = param;
    language_t* lang = languages->langs + item_num;

    set_language(languages, lang->internal_name);

    ecore_main_loop_quit();
}

static void main_win_resize_handler(Ecore_Evas* main_win)
{
   Evas* canvas = ecore_evas_get(main_win);
   int w, h;
   evas_output_size_get(canvas, &w, &h);

   Evas_Object* main_edje = evas_object_name_find(canvas, "main_edje");
   evas_object_resize(main_edje, w, h);
}

static void key_down(void* param, Evas* e, Evas_Object* o, void* event_info)
{
    Evas_Event_Key_Down* ev = (Evas_Event_Key_Down*)event_info;
    Evas_Object* choicebox = evas_object_name_find(e, "choicebox");

    if(!strcmp(ev->keyname, "Escape"))
        ecore_main_loop_quit();

    choicebox_aux_key_down_handler(choicebox, ev);
}

static void run(languages_t* languages)
{
   ecore_event_handler_add(ECORE_EVENT_SIGNAL_EXIT, exit_handler, NULL);

   Ecore_Evas* main_win = ecore_evas_software_x11_new(0, 0, 0, 0, 600, 800);
   ecore_evas_title_set(main_win, "Language selector");
   ecore_evas_name_class_set(main_win, "language-selector", "language-selector");

   Evas* main_canvas = ecore_evas_get(main_win);

   ecore_evas_callback_delete_request_set(main_win, main_win_close_handler);

   Evas_Object* main_edje = edje_object_add(main_canvas);
   edje_object_file_set(main_edje, "/usr/share/language-selector/language-selector.edj",
                         "main_window");

   evas_object_name_set(main_edje, "main_edje");
   edje_object_part_text_set(main_edje, "title", "Select language");
   evas_object_move(main_edje, 0, 0);
   evas_object_resize(main_edje, 600, 800);
   evas_object_show(main_edje);

   Evas_Object* choicebox = choicebox_new(main_canvas, "/usr/share/echoicebox/echoicebox.edj",
                                          "full", item_handler,
                                          draw_handler, page_handler, languages);
   choicebox_set_size(choicebox, languages->n);
   evas_object_name_set(choicebox, "choicebox");
   edje_object_part_swallow(main_edje, "contents", choicebox);
   evas_object_show(choicebox);

   evas_object_focus_set(main_edje, true);
   evas_object_event_callback_add(main_edje, EVAS_CALLBACK_KEY_DOWN, &key_down, NULL);

   ecore_evas_callback_resize_set(main_win, main_win_resize_handler);
   ecore_evas_show(main_win);

   ecore_x_io_error_handler_set(exit_app, NULL);

   ecore_main_loop_begin();
}

int main(int argc, char** argv)
{
   if(!evas_init())
      die("Unable to initialize Evas\n");
   if(!ecore_init())
      die("Unable to initialize Ecore\n");
   if(!ecore_evas_init())
      die("Unable to initialize Ecore_Evas\n");
   if(!edje_init())
      die("Unable to initialize Edje\n");

   languages_t* languages = get_supported_languages();
   if(!languages)
       die("Unable to obtain languages list.\n");

   run(languages);

   edje_shutdown();
   ecore_evas_shutdown();
   ecore_shutdown();
   evas_shutdown();
   return 0;
}
