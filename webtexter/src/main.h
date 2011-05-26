/* This file is part of webtexter
 *
 * Copyright (C) 2009 Thom Troy
 *
 * WebTexter is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License (GPL) as published by
 * the Free Software Foundation
 *
 * WebTexter is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with webtexter. If not, see <http://www.gnu.org/licenses/>.
 */

/*
 ============================================================================
 Name        : main.h
 Author      : Matrim
 Version     : 0.1
 Description : Main part of the gui application
 ============================================================================
 */

#ifndef _MAIN_H
#define _MAIN_H

#include "settings.h"

#include <conicconnection.h>
#include <hildon/hildon-program.h>
#include <libebook/e-book.h>
#include <libosso-abook/osso-abook.h>
#include <gtk/gtk.h>
#include <gtk/gtkmain.h>
#include <gtk/gtkbutton.h>
#include <glib.h>
#include <libosso.h>
#include <string.h>
#include <stdlib.h>

G_BEGIN_DECLS

/* Defines to add the application to dbus and keep it running
 * Please do not modify "APP_NAME" (or other defines) to different name
 */
#define APP_NAME "webtexter"
#define APP_VER "0.9"
#define APP_SERVICE "org.maemo.webtexter"
#define APP_METHOD "/org/maemo/webtexter"
/* end defines */

#define PORTRAIT_ENABLE "*#7678#*"
#define SAVEMSG_ENABLE "*#3328#*"
#define EXTRA_LOGGING_ENABLE "*#9898#*"

#define MAX_MSG_SIZE 160

/* Define structure for data that's passed around
 * the application */
typedef struct
{
	/* program and window elements */
	HildonProgram *program;
	HildonStackableWindow *messageWindow;

	GtkWidget *msgEditor;
	GtkWidget *toEditor;

	GtkWidget *contactDetailsSelector;
	GtkWidget *contactChooser;
	GtkWidget *msgSizeLabel;
	GtkWidget *sendButton;
	GtkWidget *accountBox;
	GtkWidget *account_entry, *user_entry, *pass_entry, *number_entry;

	HildonAppMenu *menu;
	GtkWidget *aboutButton;

	osso_context_t *osso_context;

	/* Other Elements */
	GtkTextBuffer *msgBuffer;
	GtkTextBuffer *toBuffer;

	GList *contactList;
	gint contactChooser_responseid;

	AppSettings settings;
	ConIcConnection *connection;
	GConfClient *gconf_client;
	gboolean msg_to_send;

	/* Portrait elements */
	gboolean portrait;
	gboolean accelerometers;

	/* Get the DBus connection */
	 DBusConnection *con;

} AppData;

G_END_DECLS

#endif
