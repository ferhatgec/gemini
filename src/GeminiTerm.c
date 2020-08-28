/* MIT License
#
# Copyright (c) 2020 Ferhat Geçdoğan All Rights Reserved.
# Distributed under the terms of the MIT License.
#
# */

#include <stdio.h>
#include <vte/vte.h> /* LibVTE */
#include <gtk/gtk.h> /* GTK */
#include <locale.h> /* For Setlocale */

#include <GeminiTerm.h>

#ifdef GDK_WINDOWING_X11
	#include <gdk/gdkx.h>
	#include <X11/Xlib.h>
#endif

/* Color functions */
#define CLR_R(x)   (((x) & 0xff0000) >> 16)
#define CLR_G(x)   (((x) & 0x00ff00) >>  8)
#define CLR_B(x)   (((x) & 0x0000ff) >>  0)
#define CLR_16(x)  ((double)(x) / 0xff)
#define CLR_GDK(x) (const GdkRGBA){ .red = CLR_16(CLR_R(x)), \
                                    .green = CLR_16(CLR_G(x)), \
                                    .blue = CLR_16(CLR_B(x)), \
                                    .alpha = 0 }

/*
	TODO: Add color scheme, transparent option and version.
*/

GtkWidget *window, *terminal;
GdkPixbuf *icon;


GdkPixbuf *create_pixbuf(const gchar * filename) {
   GdkPixbuf *pixbuf;
   GError *error = NULL;
   pixbuf = gdk_pixbuf_new_from_file(filename, &error);
   
   if (!pixbuf) {
      fprintf(stderr, "%s\n", error->message);
      g_error_free(error);
   }

   return pixbuf;
}

void gemini_Callback(VteTerminal *term, GPid pid,
	GError *error, gpointer user_data) {
	if(error == NULL) {
		/* Logging */
		g_print("Gemini started. PID: %d", pid); 
	} else {
		g_print(error->message);
		g_clear_error(&error);
	} 
		
}

/*
	TODO: Add .config/gemini/configuration and read here.
*/
void gemini_configuration() {
	/* Set numeric locale to en_US.UTF-8 */
	setlocale(LC_NUMERIC, "en_US.UTF-8");

	/* Hide the mouse cursor when typing */
    	vte_terminal_set_mouse_autohide(VTE_TERMINAL(terminal), FALSE);

    	/* Scroll issues */
    	vte_terminal_set_scrollback_lines(VTE_TERMINAL(terminal), 0);
    	vte_terminal_set_scroll_on_output(VTE_TERMINAL(terminal), FALSE);
    	vte_terminal_set_scroll_on_keystroke(VTE_TERMINAL(terminal), TRUE);
    
	/* Rewrap the content when terminal size changed */    
        vte_terminal_set_rewrap_on_resize(VTE_TERMINAL(terminal), TRUE);
    	
        /* Disable audible bell */
    	vte_terminal_set_audible_bell(VTE_TERMINAL(terminal), FALSE);
}

void gemini_connect_signals() {
    g_signal_connect(window, "delete-event", gtk_main_quit, NULL);
    g_signal_connect(terminal, "child-exited", gtk_main_quit, NULL);
    g_signal_connect(terminal, "key-press-event", G_CALLBACK(gemini_on_keypress), 
    	GTK_WINDOW(window));
}

void gemini_start() {
    terminal = vte_terminal_new();
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "GeminiTerm");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 500);
    gtk_window_set_resizable (GTK_WINDOW(window), TRUE);

    icon = create_pixbuf("resource/gemini.png"); /* Gemini icon. */
    gtk_window_set_icon(GTK_WINDOW(window), icon);	
    /* Start a new shell */
    gchar **envp = g_get_environ();
    gchar **command = (gchar *[]){g_strdup(g_environ_getenv(envp, "SHELL")), NULL }; /* Get SHELL environment. */
    g_strfreev(envp);

    /* Spawn asynchronous terminal */
    vte_terminal_spawn_async(VTE_TERMINAL(terminal), 
        VTE_PTY_DEFAULT, /* VTE_PTY flag */
        NULL,		 /* Working Dir */
        command, 	 /* Argv */
        NULL, 		 /* Environment value */
        G_SPAWN_DEFAULT, /* Spawn flag */
        NULL,		 /* Child setup function */
        NULL,		 /* Child setup data */
        NULL,		 /* Child setup data destroy */
        -1,		 /* Timeout */
        NULL,		 /* Cancellable */
        gemini_Callback, /* Async Callback */
        NULL);		 /* Callback data */

    /* Connect signals */
    gemini_connect_signals();
    
    /* Gemini configuration */
    gemini_configuration();

    /* Put widgets together and run the main loop */
    gtk_container_add(GTK_CONTAINER(window), terminal);
    gtk_widget_show_all(window);
    g_object_unref(icon);
    gtk_main();
}

/* Prototype for Handle terminal keypress events. */
gboolean gemini_on_keypress(GtkWidget *terminal, GdkEventKey *event, 
	gpointer user_data) {
    /* Check for CTRL, ALT and SHIFT keys */
    switch (event->state & (GDK_CONTROL_MASK | GDK_SHIFT_MASK | GDK_MOD1_MASK)) {
        /* CTRL + ALT */
        case GDK_MOD1_MASK | GDK_CONTROL_MASK:
            switch (event->keyval) {
            /* Paste */
            case GDK_KEY_v:
                vte_terminal_paste_clipboard(VTE_TERMINAL(terminal));
                return TRUE;

            /* Copy */
            case GDK_KEY_c:
                vte_terminal_copy_clipboard_format(VTE_TERMINAL(terminal), 
                	VTE_FORMAT_TEXT);
                return TRUE;  
	    }
    }
    return FALSE;
}

int main(int argc, char *argv[]) {
    /* Initialize GTK, the window and the terminal */  
    gtk_init(&argc, &argv);
    gemini_start();
}
