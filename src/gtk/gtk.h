#ifndef __EXAMPLEAPP_H
#define __EXAMPLEAPP_H

#include <gtk/gtk.h>

#define EXAMPLE_APP_TYPE (example_app_get_type ())
G_DECLARE_FINAL_TYPE (ExampleApp, example_app, EXAMPLE, APP, GtkApplication)

ExampleApp *example_app_new (void);

extern GtkTextMark *mark;
extern GtkTextIter iter;

gboolean notif_callback (gpointer args);

void btn_callback_yes (GtkWidget *widget, gpointer data);
void btn_callback_no (GtkWidget *widget, gpointer data);

typedef struct
{
  GtkWidget *box;
  GtkWidget *button_yes;
  GtkWidget *button_no;
  GtkTextBuffer *text_buffer;
  GtkTextView *text_view;
  int answerp;
} _callback_data;

extern _callback_data cdata;

#endif /* __EXAMPLEAPP_H */
