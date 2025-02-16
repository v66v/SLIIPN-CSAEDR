#ifndef __EXAMPLEAPPWIN_H
#define __EXAMPLEAPPWIN_H

#include <gtk/gtk.h>
#include "gtk.h"


#define EXAMPLE_APP_WINDOW_TYPE (example_app_window_get_type ())
G_DECLARE_FINAL_TYPE (ExampleAppWindow, example_app_window, EXAMPLE, APP_WINDOW, GtkApplicationWindow)


ExampleAppWindow       *example_app_window_new          (ExampleApp *app);
void                    example_app_window_open         (ExampleAppWindow *win);


#endif /* __EXAMPLEAPPWIN_H */
