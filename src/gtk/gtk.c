#include <gtk/gtk.h>
#include <stdlib.h>

#include "gtk.h"
#include "gtkwin.h"

#include "external/global.h"

mem_external ext;

int answer = 0;

GtkTextMark *mark;
GtkTextIter iter;
_callback_data cdata = { 0 };

struct _ExampleApp
{
  GtkApplication parent;
};

G_DEFINE_TYPE (ExampleApp, example_app, GTK_TYPE_APPLICATION);

static void
example_app_init (ExampleApp *app)
{
  (void)app;
  shm_init (&ext.shd, sizeof (*ext.mem), IPC_CREAT);
  cdata.answerp = 0;
  ext.mem
      = (mem_t *)ext.shd.ptr; // short hand, in order to not constantly cast
}

static void
example_app_open (GApplication *app, const gchar *hint)
{
  (void)hint;
  GList *windows;
  ExampleAppWindow *win;

  windows = gtk_application_get_windows (GTK_APPLICATION (app));
  if (windows)
    win = EXAMPLE_APP_WINDOW (windows->data);
  else
    win = example_app_window_new (EXAMPLE_APP (app));

  example_app_window_open (win);

  gtk_window_present (GTK_WINDOW (win));
}

static void
example_app_activate (GApplication *app)
{
  ExampleAppWindow *win;

  win = example_app_window_new (EXAMPLE_APP (app));
  gtk_window_present (GTK_WINDOW (win));
  example_app_open (app, NULL);
}

static void
example_app_class_init (ExampleAppClass *class)
{
  G_APPLICATION_CLASS (class)->activate = example_app_activate;
  /* G_APPLICATION_CLASS (class)->open = example_app_open; */
}

ExampleApp *
example_app_new (void)
{
  return g_object_new (EXAMPLE_APP_TYPE, "application-id",
                       "org.gtk.exampleapp", "flags",
                       G_APPLICATION_HANDLES_OPEN, NULL);
}

void
btn_callback_yes (GtkWidget *widget, gpointer data)
{
  (void)data;
  (void)widget;
  gtk_widget_hide (cdata.box);
  cdata.answerp = 0;

  pthread_mutex_lock (&ext.mem->mutex);
  ext.mem->assist[ext.mem->index_assist].type = MSG_ASSISTANCE;
  strcpy (ext.mem->assist[ext.mem->index_assist].data, "YES");
  ext.mem->index_assist++;
  pthread_mutex_unlock (&ext.mem->mutex);
}

void
btn_callback_no (GtkWidget *widget, gpointer data)
{
  (void)data;
  (void)widget;
  gtk_widget_hide (cdata.box);
  cdata.answerp = 0;

  pthread_mutex_lock (&ext.mem->mutex);
  ext.mem->assist[ext.mem->index_assist].type = MSG_ASSISTANCE;
  strcpy (ext.mem->assist[ext.mem->index_assist].data, "NO");
  ext.mem->index_assist++;
  pthread_mutex_unlock (&ext.mem->mutex);
}

void
insert_txt (const char *prefix)
{
  gtk_text_buffer_insert (cdata.text_buffer, &iter, prefix, strlen (prefix));
  gtk_text_buffer_insert (cdata.text_buffer, &iter,
                          ext.mem->msgs[ext.mem->index_e].data,
                          strlen (ext.mem->msgs[ext.mem->index_e].data));

  gtk_text_buffer_insert (cdata.text_buffer, &iter, "\n", strlen ("\n"));

  gtk_text_view_scroll_to_mark (cdata.text_view, mark, 0., FALSE, 0., 0.);

  ext.mem->index_e++;
}

#define PLAY_SOUND                                                            \
  system ("paplay ./etc/bell.oga");                                           \

gboolean
notif_callback (gpointer args)
{
  (void)args;
  pthread_mutex_lock (&ext.mem->mutex);

  if (ext.mem->index - ext.mem->index_e > 0)
    {
      if (ext.mem->msgs[ext.mem->index_e].type == MSG_NOTIF)
        {
          insert_txt ("");
          PLAY_SOUND
        }
      else if (ext.mem->msgs[ext.mem->index_e].type == MSG_ERROR)
        {
          insert_txt ("ERROR: ");
          PLAY_SOUND
        }
      else if (ext.mem->msgs[ext.mem->index_e].type == MSG_ASSISTANCE)
        {
          cdata.answerp = 1;
          gtk_widget_show_all (cdata.box);
          ext.mem->index_e++;
          PLAY_SOUND
        }
      else if (ext.mem->msgs[ext.mem->index_e].type == MSG_ASSISTANCE_REVERT)
        {
          cdata.answerp = 0;
          gtk_widget_hide (cdata.box);
          insert_txt ("ASSIST: ");
        }
    }
  pthread_mutex_unlock (&ext.mem->mutex);

  return 1;
}
