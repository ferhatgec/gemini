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
	TODO: Add transparent option, customization with given value.
*/

GtkWidget *window, *terminal, *header, *button, *image; /* Window, Headerbar && Terminal widget */
GdkPixbuf *icon; /* Icon */

static char *input;
static gchar **command, **envp;

static PangoFontDescription *fontDesc; /* Description for the terminal font */
static int currentFontSize;

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
	
	printf("\n"); /* Newline */
}

/* Gemini Terminal set font. */
void gemini_set_Term_Font(int fontSize) {
    /* GEMINI_TERM_FONT + " " + GEMINI_TERM_FONT_SIZE */
    gchar *fontStr = g_strconcat(GEMINI_TERM_FONT, " ", g_strdup_printf("%d", fontSize), NULL);
    if ((fontDesc = pango_font_description_from_string(fontStr)) != NULL) {
    	vte_terminal_set_font(VTE_TERMINAL(terminal), fontDesc);
	currentFontSize = fontSize;
	pango_font_description_free(fontDesc);
	g_free(fontStr);	  
    }
}

gboolean gemini_on_title_Changed(GtkWidget *terminal, gpointer user_data) {
    GtkWindow *window = user_data;
    gtk_window_set_title(window,
	vte_terminal_get_window_title(VTE_TERMINAL(terminal))?:"Gemini"); /* Default = GeminiTerm */
    return TRUE;
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
    	
    /* Disable audible bell */
    vte_terminal_set_audible_bell(VTE_TERMINAL(terminal), FALSE);

    /* Allow hyperlinks */
    vte_terminal_set_allow_hyperlink(VTE_TERMINAL(terminal), TRUE);

    /* Disable Blick mode */
    vte_terminal_set_cursor_blink_mode(VTE_TERMINAL(terminal), 
        VTE_CURSOR_BLINK_OFF);
	
    /* Set the terminal colors and font */
    vte_terminal_set_colors(VTE_TERMINAL(terminal),
        &CLR_GDK(0xc0d6e4),          /* Foreground */
        &(GdkRGBA){ .alpha = 1 }, /* Background (RGBA) */
        (const GdkRGBA[]){           /* Palette */
            CLR_GDK(0x111111),
            CLR_GDK(0xd36265),
            CLR_GDK(0xaece91),
            CLR_GDK(0xe7e18c),
            CLR_GDK(0x5297cf),
            CLR_GDK(0x963c59),
            CLR_GDK(0x5E7175),
            CLR_GDK(0xbebebe),
            CLR_GDK(0x666666),
            CLR_GDK(0xef8171),
            CLR_GDK(0xcfefb3),
            CLR_GDK(0xfff796),
            CLR_GDK(0x74b8ef),
            CLR_GDK(0xb85e7b),
            CLR_GDK(0xA3BABF),
            CLR_GDK(0xffffff)
        }, 16);
	
	
    gemini_set_Term_Font(GEMINI_TERM_FONT_SIZE);  
    
    gtk_widget_set_visual(window, 
        gdk_screen_get_rgba_visual(gtk_widget_get_screen(window)));
    
}

void gemini_connect_signals() {
    g_signal_connect(window, "delete-event", gtk_main_quit, NULL);
    g_signal_connect(terminal, "child-exited", gtk_main_quit, NULL);
 
    /* For Shortcuts */
    g_signal_connect(terminal, "key-press-event", G_CALLBACK(gemini_on_keypress), 
    	GTK_WINDOW(window));
    
    /* Terminal window title */
    g_signal_connect(terminal, "window-title-changed", G_CALLBACK(gemini_on_title_Changed), 
                        GTK_WINDOW(window));
}


void gemini_start() {
    terminal = vte_terminal_new();
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    header = gtk_header_bar_new();
	
    gtk_window_set_title(GTK_WINDOW(window), "Fegeya Gemini");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 500);
    gtk_window_set_resizable (GTK_WINDOW(window), TRUE);
    
    icon = create_pixbuf("/usr/share/pixmaps/gemini/gemini_32.png"); /* Gemini icon. */
    image = gtk_image_new_from_file("/usr/share/pixmaps/gemini/gemini_32.png");
    button = gtk_tool_button_new(image, NULL);
    
    gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(header), TRUE);    
    gtk_window_set_icon(GTK_WINDOW(window), icon);	

    //gtk_button_set_image(GTK_BUTTON (button), image);
    gtk_header_bar_pack_start(GTK_HEADER_BAR(header), button);
    gtk_window_set_titlebar(GTK_WINDOW(window), header);
    	
    /* Start a new shell */
    envp = g_get_environ();
    command = (gchar *[]){g_strdup(g_environ_getenv(envp, "SHELL")), NULL}; /* Get SHELL environment. */
    
    /* If argc > 1 and input is not null, shell run with input */
    if(input != NULL)
    	command = (gchar *[]){g_strdup(g_environ_getenv(envp, "SHELL")), "-c", input, NULL};
    
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

	     /* 
		Change font size 
			CTRL + ALT + 1
			CTRL + ALT + 2
			CTRL + ALT + = 
	      */
             case GDK_KEY_plus:
             case GDK_KEY_1:
                gemini_set_Term_Font(currentFontSize + 1);
                return TRUE;
             
	     case GDK_KEY_minus:
             case GDK_KEY_2:
                gemini_set_Term_Font(currentFontSize - 1);
                return TRUE;

             case GDK_KEY_equal:
	        gemini_set_Term_Font(GEMINI_TERM_FONT_SIZE);
	        return TRUE;
	    }
    }
    return FALSE;
}

int main(int argc, char *argv[]) {
    /* Initialize GTK, the window and the terminal */  
    gtk_init(&argc, &argv);
    
    if(argc > 1)
    	if(argv[1] != NULL) 
    		input = argv[1];
    
    gemini_start();
}
