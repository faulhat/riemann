#include <gtk/gtk.h>

void activate(GtkApplication *app, gpointer userData) {
   GtkWidget *window = gtk_application_window_new(app);
   gtk_window_set_title(GTK_WINDOW(window), "Window!");
   gtk_window_set_default_size(GTK_WINDOW(window), 1280, 960);
   gtk_widget_show_all(window);
}

int gui_app() {
   GtkApplication *app = gtk_application_new("org.gtk.example", G_APPLICATION_DEFAULT_FLAGS);
   g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
   int status = g_application_run(G_APPLICATION(app), 0, NULL);
   g_object_unref(app);

   return status;
}

