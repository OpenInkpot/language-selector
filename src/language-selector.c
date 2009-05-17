#include <libintl.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <Ecore.h>
#include <Ecore_Evas.h>
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

/* FIXME */
#define BUFSIZE 512

static void draw_handler(Evas_Object* choicebox, Evas_Object* item,
                         int item_num, int page_position, void* param)
{
    languages_t* languages = param;
    language_t* lang = languages->langs + item_num;

    char buf[BUFSIZE];
    if(lang->native_name)
        snprintf(buf, BUFSIZE, "%s / %s", lang->native_name, lang->name);
    else
        strncpy(buf, lang->name, BUFSIZE);

    edje_object_part_text_set(item, "text", buf);
}

static void page_handler(Evas_Object* choicebox, int cur_page, int total_pages,
                         void* param)
{
    char buf[BUFSIZE];
    if(total_pages < 2)
        *buf = 0;
    else
        snprintf(buf, BUFSIZE, gettext("%d/%d"), cur_page + 1, total_pages);

    Evas* canvas = evas_object_evas_get(choicebox);
    Evas_Object* footer = evas_object_name_find(canvas, "footer");

    edje_object_part_text_set(footer, "text", buf);
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

   Evas_Object* bg = evas_object_name_find(canvas, "bg");
   evas_object_resize(bg, w, h);

   /* FIXME */
   int header_h = 49;
   int footer_h = 49;

   Evas_Object* header = evas_object_name_find(canvas, "header");
   evas_object_move(header, 0, 0);
   evas_object_resize(header, w, header_h);

   Evas_Object* choicebox = evas_object_name_find(canvas, "choicebox");
   evas_object_move(choicebox, 0, header_h);
   evas_object_resize(choicebox, w, h - header_h - footer_h);

   Evas_Object* footer = evas_object_name_find(canvas, "footer");
   evas_object_move(footer, 0, h - footer_h);
   evas_object_resize(footer, w, footer_h);
}

static void key_down(void* param, Evas* e, Evas_Object* o, void* event_info)
{
    Evas_Event_Key_Down* ev = (Evas_Event_Key_Down*)event_info;
    Evas_Object* r = evas_object_name_find(e, "choicebox");

    if(!strcmp(ev->keyname, "Up") || !strcmp(ev->keyname, "Prior"))
        choicebox_prev(r);
    if(!strcmp(ev->keyname, "Down") || !strcmp(ev->keyname, "Next"))
        choicebox_next(r);
    if(!strcmp(ev->keyname, "Left"))
        choicebox_prevpage(r);
    if(!strcmp(ev->keyname, "Right"))
        choicebox_nextpage(r);
    if((ev->keyname[0] >= '1') && (ev->keyname[0] <= '9') && !ev->keyname[1])
        choicebox_activate_nth_visible(r, ev->keyname[0] - '1', false);
    if(!strcmp(ev->keyname, "Return"))
        choicebox_activate_current(r, false);
    if(!strcmp(ev->keyname, "Escape"))
        ecore_main_loop_quit();
}

static void run(languages_t* languages)
{
   ecore_event_handler_add(ECORE_EVENT_SIGNAL_EXIT, exit_handler, NULL);

   Ecore_Evas* main_win = ecore_evas_software_x11_new(0, 0, 0, 0, 600, 800);
   ecore_evas_title_set(main_win, "Language selector");
   ecore_evas_name_class_set(main_win, "language-selector", "language-selector");

   Evas* main_canvas = ecore_evas_get(main_win);

   ecore_evas_callback_delete_request_set(main_win, main_win_close_handler);

   Evas_Object* bg = evas_object_rectangle_add(main_canvas);
   evas_object_name_set(bg, "bg");
   evas_object_color_set(bg, 255, 255, 255, 255);
   evas_object_show(bg);

   /* FIXME */
   Evas_Object* header = edje_object_add(main_canvas);
   evas_object_name_set(header, "header");
   edje_object_file_set(header, "/usr/share/language-selector/language-selector.edj",
                        "header");
   edje_object_part_text_set(header, "text", "Select language");
   evas_object_show(header);

   /* FIXME */
   Evas_Object* footer = edje_object_add(main_canvas);
   evas_object_name_set(footer, "footer");
   edje_object_file_set(footer, "/usr/share/language-selector/language-selector.edj",
                        "footer");
   evas_object_show(footer);

   Evas_Object* choicebox = choicebox_new(main_canvas, "/usr/share/echoicebox/echoicebox.edj",
                                          "full", item_handler,
                                          draw_handler, page_handler, languages);

   choicebox_set_size(choicebox, languages->n);
   evas_object_name_set(choicebox, "choicebox");
   evas_object_show(choicebox);

   evas_object_focus_set(choicebox, true);
   evas_object_event_callback_add(choicebox, EVAS_CALLBACK_KEY_DOWN, &key_down, NULL);

   ecore_evas_callback_resize_set(main_win, main_win_resize_handler);

   ecore_evas_show(main_win);

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
