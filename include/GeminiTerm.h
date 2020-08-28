/* MIT License
#
# Copyright (c) 2020 Ferhat Geçdoğan All Rights Reserved.
# Distributed under the terms of the MIT License.
#
# */

#ifndef GEMINI_TERM_H
#define GEMINI_TERM_H

#include <vte/vte.h>

/*
	TODO: Add Major, Minor definitions for Gemini version.
*/
#define GEMINI_TERM_VERSION "0.1-beta-1"

/* Prototype for Handle terminal keypress events. */
gboolean gemini_on_keypress(GtkWidget *, GdkEventKey *,
	gpointer);

#endif /* GEMINI_TERM_H */
