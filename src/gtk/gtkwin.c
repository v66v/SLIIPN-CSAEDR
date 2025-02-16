#include <gtk/gtk.h>

#include "gtk.h"
#include "gtkwin.h"

struct _ExampleAppWindow
{
  GtkApplicationWindow parent;
};

typedef struct _ExampleAppWindowPrivate ExampleAppWindowPrivate;

struct _ExampleAppWindowPrivate
{
  GtkWidget *stack;
  GtkWidget *callback_box;
};

G_DEFINE_TYPE_WITH_PRIVATE (ExampleAppWindow, example_app_window,
                            GTK_TYPE_APPLICATION_WINDOW);

static void
example_app_window_init (ExampleAppWindow *win)
{
  gtk_widget_init_template (GTK_WIDGET (win));
}

static void
example_app_window_class_init (ExampleAppWindowClass *class)
{
  gtk_widget_class_set_template_from_resource (
      GTK_WIDGET_CLASS (class), "/org/gtk/exampleapp/window.ui");
  gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class),
                                                ExampleAppWindow, stack);
  gtk_widget_class_bind_template_child_private (
      GTK_WIDGET_CLASS (class), ExampleAppWindow, callback_box);
}

ExampleAppWindow *
example_app_window_new (ExampleApp *app)
{
  return g_object_new (EXAMPLE_APP_WINDOW_TYPE, "application", app, NULL);
}

void
example_app_window_open (ExampleAppWindow *win)
{
  ExampleAppWindowPrivate *priv;
  GtkWidget *scrolled, *view;

  priv = example_app_window_get_instance_private (win);

  scrolled = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrolled);
  gtk_widget_set_hexpand (scrolled, TRUE);
  gtk_widget_set_vexpand (scrolled, TRUE);
  view = gtk_text_view_new ();
  gtk_text_view_set_editable (GTK_TEXT_VIEW (view), FALSE);
  gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (view), FALSE);
  gtk_widget_show (view);

  GtkWidget *button_yes = gtk_button_new_with_label ("Yes");
  GtkWidget *button_no = gtk_button_new_with_label ("No");

  GtkWidget *label = gtk_label_new ("");
  gtk_label_set_markup (
      GTK_LABEL (label),
      "<span size=\"larger\">Do you require medical assistance?</span>");

  g_signal_connect (button_yes, "clicked", G_CALLBACK (btn_callback_yes),
                    NULL);
  g_signal_connect (button_no, "clicked", G_CALLBACK (btn_callback_no), NULL);

  gtk_widget_show (button_yes);
  gtk_widget_show (button_no);
  gtk_widget_show (label);

  GtkWidget *box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 5);
  GtkWidget *btn_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 5);
  gtk_widget_set_hexpand (button_yes, TRUE);
  gtk_widget_set_hexpand (button_no, TRUE);

  /* gtk_widget_hide (btn_box); */
  gtk_widget_show (btn_box);

  gtk_container_add (GTK_CONTAINER (btn_box), button_yes);
  gtk_container_add (GTK_CONTAINER (btn_box), button_no);

  gtk_container_add (GTK_CONTAINER (box), label);
  gtk_container_add (GTK_CONTAINER (box), btn_box);

  gtk_container_add (GTK_CONTAINER (priv->callback_box), box);

  gtk_container_add (GTK_CONTAINER (scrolled), view);

  cdata.box = priv->callback_box;
  cdata.button_yes = button_yes;
  cdata.button_no = button_no;
  cdata.text_buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));
  cdata.text_view = GTK_TEXT_VIEW (view);

  gtk_text_buffer_get_end_iter (cdata.text_buffer, &iter);
  mark = gtk_text_buffer_create_mark (cdata.text_buffer, NULL, &iter, FALSE);
  gtk_stack_add_titled (GTK_STACK (priv->stack), scrolled, "NOTIFICATIONS",
                        "NOTIFICATIONS");

  gtk_widget_hide (box);
  g_timeout_add (100, notif_callback, NULL);
}
