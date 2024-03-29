/* This file is part of webtexter
 *
 * Copyright (C) 2011 Thom Troy
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
 Name        : main.c
 Author      : Matrim
 Version     : 0.1
 Description : Main part of webtexter
 ============================================================================
 */
/* Includes */
#include "http_sender.h"
#include "settings.h"
#include "o2_sender.h"
#include "network_utilities.h"
#include "main.h"

#include <conicconnection.h>
#include <hildon/hildon-program.h>
#include <libebook/e-book.h>
#include <libosso-abook/osso-abook.h>
#include <gtk/gtk.h>
#include <gtk/gtkmain.h>
#include <gtk/gtkbutton.h>
#include <gtk/gtkcombobox.h>
#include <mce/dbus-names.h>
#include <mce/mode-names.h>
#include <glib.h>
#include <libosso.h>
#include <string.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include "localisation.h"
#include "he-about-dialog.h"
#include <stdio.h>

/* Orientation functions modified from those in conboy and mbarcode.*/

#define MCE_MATCH_RULE "type='signal',interface='" MCE_SIGNAL_IF "',member='" MCE_DEVICE_ORIENTATION_SIG "'"

static void set_orientation(AppData* appdata, const gchar* orientation)
{
	/* If we disabled the accelerometers, then we don't want to react on it if
	 * it was turned on by another application.
	 *
	 * E.g. when a dialog is open we want to disable auto rotation and if there
	 * is another application that turned on the accelerators we still don't want
	 * the input.
	 */
	if (appdata->accelerometers == FALSE)
		return;

	g_debug("INFO: Setting to orientation: %s\n", orientation);

	HildonPortraitFlags flags;


	if (strcmp(orientation, "landscape") == 0) {
		flags = HILDON_PORTRAIT_MODE_SUPPORT;
		appdata->portrait = FALSE;
	} else if (strcmp(orientation, "portrait") == 0) {
		flags = HILDON_PORTRAIT_MODE_REQUEST;
		appdata->portrait = TRUE;
	} else {
		g_debug("ERROR: Orientation must be 'landscape' or 'portrait', not '%s'.\n", orientation);
		return;
	}

	/* Switch the orientation of all open windows */
	hildon_gtk_window_set_portrait_flags(GTK_WINDOW(appdata->messageWindow), flags);

}



gboolean device_is_portrait_mode(AppData* appdata)
{
    osso_rpc_t ret;
    gboolean result = FALSE;

    if (osso_rpc_run_system(appdata->osso_context, MCE_SERVICE, MCE_REQUEST_PATH,
        MCE_REQUEST_IF, MCE_DEVICE_ORIENTATION_GET, &ret, DBUS_TYPE_INVALID) == OSSO_OK) {
        g_debug("INFO: DBus said orientation is: %s\n", ret.value.s);

        if (strcmp(ret.value.s, MCE_ORIENTATION_PORTRAIT) == 0) {
            result = TRUE;
        }

        osso_rpc_free_val(&ret);

    } else {
        g_debug("ERROR: Call do DBus failed\n");
    }

    return result;
}


static DBusHandlerResult dbus_handle_mce_message( DBusConnection *con, DBusMessage *msg, AppData* appdata)
{
    DBusMessageIter iter;
    const gchar *mode = NULL;
    /* Could also catch other MCE messages here. */

    if (dbus_message_is_signal(msg, MCE_SIGNAL_IF, MCE_DEVICE_ORIENTATION_SIG)) {
        if (dbus_message_iter_init(msg, &iter)) {
            dbus_message_iter_get_basic(&iter, &mode);
            g_debug("INFO: New orientation is now: %s\n", mode);
            if(appdata->settings.orientation_enabled)
            	set_orientation(appdata, mode);

        }
    }
    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

void orientation_enable_accelerometers(AppData* appdata)
{
	if (appdata->accelerometers == TRUE)
		return;

	/* TODO: Replace string "req_accelerometer_enable" with constant once we switched to final SDK */
	if (osso_rpc_run_system(appdata->osso_context, MCE_SERVICE, MCE_REQUEST_PATH,
			MCE_REQUEST_IF,	"req_accelerometer_enable", NULL, DBUS_TYPE_INVALID) == OSSO_OK) {
		g_debug("INFO: Accelerometers enabled\n");
		appdata->accelerometers = TRUE;
	} else {
		g_debug("WARN: Cannot enable accelerometers\n");
	}
}

void orientation_disable_accelerometers(AppData* appdata)
{
	if (appdata->accelerometers == FALSE)
		return;

	/* TODO: Replace string "req_accelerometer_disable" with constant once we switched to final SDK */
	if (osso_rpc_run_system(appdata->osso_context, MCE_SERVICE, MCE_REQUEST_PATH,
			MCE_REQUEST_IF,	"req_accelerometer_disable", NULL, DBUS_TYPE_INVALID) == OSSO_OK) {
		g_debug("INFO: Accelerometers disabled\n");
		appdata->accelerometers = FALSE;
	} else {
		g_printerr("WARN: Cannot disable accelerometers\n");
	}
}

void orientation_init(AppData* appdata)
{
    g_debug("Entered orientation_init()\n");


    g_debug("orientation_init(): about to run orientation_enable_accelerometers()\n");
	/* Enable accelerometers */
	orientation_enable_accelerometers(appdata);

    g_debug("orientation_init(): about to run device_is_portrait_mode()\n");
	/* Get current orientation from DBus and save into appdata */
	appdata->portrait = device_is_portrait_mode(appdata);

    g_debug("orientation_init(): about to dbus_bus_add_match()\n");
	/* Add a matchin rule */
	dbus_bus_add_match(appdata->con, MCE_MATCH_RULE, NULL);

    g_debug("orientation_init(): about to dbus_connection_add_filter()\n");
	/* Add the callback, which should be called, once the device is rotated */
	dbus_connection_add_filter(appdata->con, dbus_handle_mce_message, NULL, appdata);
}


/*
 * Dbus functions
 */

static gint
dbus_callback (const gchar *interface, const gchar *method,
               GArray *arguments, gpointer data,
               osso_rpc_t *retval)
{
  g_debug ("dbus: %s, %s\n", interface, method);

  if (!strcmp (method, "top_application"))
      gtk_window_present (GTK_WINDOW (data));

  return DBUS_TYPE_INVALID;
}

/*
 * callbacks
 */

static void contactDetails_response(GtkButton* button, gint responseid, AppData *appdata)
{
	OssoABookContactField *contactField;
	EVCardAttribute *contactAttribute;

	if(responseid == GTK_RESPONSE_OK )
	{
		contactField = osso_abook_contact_detail_selector_get_selected_field(
				OSSO_ABOOK_CONTACT_DETAIL_SELECTOR(appdata->contactDetailsSelector));

		contactAttribute = osso_abook_contact_field_get_attribute (
				OSSO_ABOOK_CONTACT_FIELD(contactField));

		gchar* attStr = e_vcard_attribute_get_value(contactAttribute);
		gchar* separator = ",";

		appdata->toBuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(appdata->toEditor));
		gint toCharCount = gtk_text_buffer_get_char_count(GTK_TEXT_BUFFER(appdata->toBuffer));

		if(toCharCount > 0)
			gtk_text_buffer_insert_at_cursor (appdata->toBuffer,separator, strlen(separator) );

		gtk_text_buffer_insert_at_cursor (appdata->toBuffer,attStr, strlen(attStr) );

		gtk_text_view_set_buffer(GTK_TEXT_VIEW(appdata->toEditor), appdata->toBuffer);

		g_free(attStr);

	}
}

static void contactDetails(OssoABookContact *contact, AppData *appdata)
{
	appdata->contactDetailsSelector = osso_abook_contact_detail_selector_new_for_contact(
			GTK_WINDOW(appdata->messageWindow), OSSO_ABOOK_CONTACT(contact),
			OSSO_ABOOK_CONTACT_DETAIL_PHONE);

	g_signal_connect(G_OBJECT(appdata->contactDetailsSelector), "response",
						G_CALLBACK(contactDetails_response), appdata);

	gtk_dialog_run(GTK_DIALOG(appdata->contactDetailsSelector));

	gtk_widget_destroy(GTK_WIDGET(appdata->contactDetailsSelector));
}

static void contactChooser_response(GtkButton* button, gint responseid, AppData *appdata)
{
	appdata->contactChooser_responseid = responseid;

	/* Check the response from the contact chooser and display details if it's ok
	 *
	 */
	if(appdata->contactChooser_responseid == GTK_RESPONSE_OK )
	{

		appdata->contactList = osso_abook_contact_chooser_get_selection(
					OSSO_ABOOK_CONTACT_CHOOSER(appdata->contactChooser));
		guint listLength = g_list_length(appdata->contactList);

		if(listLength > 0)
		{
			g_list_foreach(appdata->contactList, contactDetails, appdata);
		}

		/*
		 * I might remove this if I decide to show the old select contacted contacts if the users presses
		 * to again
		 */
		if(appdata->contactList != NULL)
		{
			g_list_free(appdata->contactList);
		}

	}

}

static void toButton_clicked (GtkButton* button, AppData *appdata)
{
	/*
	 * Create the contact chooser
	 */
	appdata->contactChooser = osso_abook_contact_chooser_new_with_capabilities (
			GTK_WINDOW(appdata->messageWindow), "Choose Contact",
	 				OSSO_ABOOK_CAPS_PHONE, OSSO_ABOOK_CONTACT_ORDER_NONE);

	osso_abook_contact_chooser_set_maximum_selection (OSSO_ABOOK_CONTACT_CHOOSER(
			appdata->contactChooser), 3);

	g_signal_connect(G_OBJECT(appdata->contactChooser), "response",
					G_CALLBACK(contactChooser_response), appdata);

	gtk_dialog_run(GTK_DIALOG(appdata->contactChooser));

	gtk_widget_destroy(GTK_WIDGET(appdata->contactChooser));

}

void settingsButton_clicked(GtkButton* button, AppData *appdata)
{
	int account;
	gint wizard_response = create_settings_wizard(appdata);
	
	if(wizard_response == HILDON_WIZARD_DIALOG_FINISH)
	{
		g_debug("settings response is good in settingsButton_Clicked \n");
		/*
		* Should be moved to response good if statment above
		 */
		account = gtk_combo_box_get_active (GTK_COMBO_BOX(appdata->accountBox));
		if (account < 0) {	/*  No accounts defined  */
			account = 0;
			set_settings(appdata->gconf_client, account, &appdata->settings);
			gtk_combo_box_append_text (GTK_COMBO_BOX(appdata->accountBox), appdata->settings.accountname);
		}
		else {
			set_settings(appdata->gconf_client, account, &appdata->settings);
			gtk_combo_box_insert_text (GTK_COMBO_BOX(appdata->accountBox), account, appdata->settings.accountname);
			gtk_combo_box_remove_text (GTK_COMBO_BOX(appdata->accountBox), account+1);
		}
		gtk_combo_box_set_active (GTK_COMBO_BOX(appdata->accountBox), account);
	}
}

void deleteButton_clicked(GtkButton* button, AppData *appdata)
{
	GtkWidget *note; 
	int i, account;
	const gchar confirm_text[32];
	
	snprintf (confirm_text, 32, "Delete account %s?", appdata->settings.accountname);
	
	note = hildon_note_new_confirmation (GTK_WINDOW(appdata->messageWindow), confirm_text);
	i = gtk_dialog_run (GTK_DIALOG (note));
	gtk_widget_destroy (GTK_WIDGET (note));

	if (i == GTK_RESPONSE_OK) {
		account = gtk_combo_box_get_active (GTK_COMBO_BOX(appdata->accountBox));
		i = account;
		while (get_settings(appdata->gconf_client, i+1, &appdata->settings)) {
			set_settings(appdata->gconf_client, i, &appdata->settings);
			i++;
		}
		set_settings(appdata->gconf_client, i, NULL);

		g_debug("delete response is YES in deleteButton_Clicked %u \n", i);
		
		gtk_combo_box_remove_text (GTK_COMBO_BOX(appdata->accountBox), account);
		if (account >= i)
			account = 0;
		gtk_combo_box_set_active (GTK_COMBO_BOX(appdata->accountBox), account);

		if (i == 0)
			gtk_widget_set_sensitive(GTK_WIDGET(appdata->sendButton), FALSE);

		g_debug("settings response is good in deleteButton_Clicked \n");
	}
}

void newButton_clicked(GtkButton* button, AppData *appdata)
{
	gint wizard_response;
	int account;
	
	/*  Find last account number  */
	account = 0;
	while (get_settings(appdata->gconf_client, account, &appdata->settings))
		account++;
	
	wizard_response = create_settings_wizard(appdata);
	
	if(wizard_response == HILDON_WIZARD_DIALOG_FINISH)
	{
		gtk_widget_set_sensitive(GTK_WIDGET(appdata->sendButton), TRUE);
		g_debug("settings response is good in newButton_Clicked %u \n", account);

		set_settings(appdata->gconf_client, account, &appdata->settings);
		
		gtk_combo_box_append_text (GTK_COMBO_BOX(appdata->accountBox), appdata->settings.accountname);
		gtk_combo_box_set_active (GTK_COMBO_BOX(appdata->accountBox), account);
	}
	else {
		account = gtk_combo_box_get_active (GTK_COMBO_BOX(appdata->accountBox));
		get_settings(appdata->gconf_client, account, &appdata->settings);
	}
}

void wizard_response(GtkDialog *dialog,
        gint       response_id,
        gint *response)
{
	*response = response_id;
}

gboolean
on_page_switch (GtkNotebook *notebook,
                GtkNotebookPage *page,
                guint num,
                GtkDialog *wizard)
{
	if(num != 5)
		gtk_dialog_set_response_sensitive (GTK_DIALOG (wizard),
		                                       HILDON_WIZARD_DIALOG_FINISH,
		                                       FALSE);

    return TRUE;
}

static gboolean
some_page_func (GtkNotebook *nb,
                gint current,
                AppData *appdata)
{
	GtkWidget *entry;
	entry = gtk_notebook_get_nth_page (nb, current);
	GList *children = gtk_container_get_children(GTK_CONTAINER(entry));

	/* Validate data and if valid set the app settings*/
	switch (current) {
		case 0:
		{
			const gchar *accounts = hildon_entry_get_text (HILDON_ENTRY(appdata->account_entry));
			const gchar *users = hildon_entry_get_text (HILDON_ENTRY(appdata->user_entry));
			const gchar *passs = hildon_entry_get_text (HILDON_ENTRY(appdata->pass_entry));
			const gchar *numbers = hildon_entry_get_text (HILDON_ENTRY(appdata->number_entry));
			if ((strlen (users) != 0) &&
					(strlen (passs) != 0) && (strlen (numbers) != 0))
			{
				if(appdata->settings.accountname != NULL)
					g_free(appdata->settings.accountname);

				if(appdata->settings.password != NULL)
					g_free(appdata->settings.password);

				if(appdata->settings.username != NULL)
					g_free(appdata->settings.username);

				if(appdata->settings.number != NULL)
					g_free(appdata->settings.number);

				appdata->settings.accountname = g_strdup(accounts);
				appdata->settings.password = g_strdup(passs);
				appdata->settings.username = g_strdup(users);
				appdata->settings.number = g_strdup(numbers);
				return TRUE;
			}
			else
			{
				return FALSE;
			}
		}
		case 1:
		{
			/*
			 * This is a bit ugly but for this page, children 0 is the pannable area. To get the vbox we need
			 * to get get the child of the pannable area and to get the list of buttons we have to get the
			 * children of the vbox
			 */
			GList *pan = gtk_container_get_children(GTK_CONTAINER(gtk_bin_get_child(GTK_BIN(g_list_nth_data(children,0)))));
			if(pan == NULL)
				g_debug("pan is null\n");
			GtkRadioButton *o2_but = GTK_RADIO_BUTTON(g_list_nth_data(pan, O2));
			GtkRadioButton *voda_but = GTK_RADIO_BUTTON(g_list_nth_data(pan, VODAFONE));
			GtkRadioButton *met_but = GTK_RADIO_BUTTON(g_list_nth_data(pan, METEOR));
			GtkRadioButton *three_but = GTK_RADIO_BUTTON(g_list_nth_data(pan, THREE));
			GtkRadioButton *blueface_but = GTK_RADIO_BUTTON(g_list_nth_data(pan, BLUEFACE));
			GtkRadioButton *voipcheap_but = GTK_RADIO_BUTTON(g_list_nth_data(pan, VOIPCHEAP));
			GtkRadioButton *smsdiscount_but = GTK_RADIO_BUTTON(g_list_nth_data(pan, SMSDISCOUNT));
			GtkRadioButton *lowratevoip_but = GTK_RADIO_BUTTON(g_list_nth_data(pan, LOWRATEVOIP));
			GtkRadioButton *otherbetamax_but = GTK_RADIO_BUTTON(g_list_nth_data(pan, OTHER_BETAMAX));
			GtkRadioButton *websmsru_but = GTK_RADIO_BUTTON(g_list_nth_data(pan, WEBSMSRU));
			GtkRadioButton *exetel_but = GTK_RADIO_BUTTON(g_list_nth_data(pan, EXETEL));
			GtkRadioButton *pennytel_but = GTK_RADIO_BUTTON(g_list_nth_data(pan, PENNYTEL));


			if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(o2_but)))
				appdata->settings.provider = O2;
			if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(voda_but)))
				appdata->settings.provider = VODAFONE;
			if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(met_but)))
				appdata->settings.provider = METEOR;
			if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(three_but)))
				appdata->settings.provider = THREE;
			if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(blueface_but)))
				appdata->settings.provider = BLUEFACE;
			if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(voipcheap_but)))
				appdata->settings.provider = VOIPCHEAP;
			if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(smsdiscount_but)))
				appdata->settings.provider = SMSDISCOUNT;
			if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(lowratevoip_but)))
				appdata->settings.provider = LOWRATEVOIP;
			if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(otherbetamax_but)))
				appdata->settings.provider = OTHER_BETAMAX;
			if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(websmsru_but)))
				appdata->settings.provider = WEBSMSRU;
			if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(exetel_but)))
				appdata->settings.provider = EXETEL;
			if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(pennytel_but)))
							appdata->settings.provider = PENNYTEL;

			return TRUE;
		}
		case 2:
		{
			GtkRadioButton *use_proxy_but = GTK_RADIO_BUTTON(g_list_nth_data(children, 1));
			GtkRadioButton *no_proxy_but = GTK_RADIO_BUTTON(g_list_nth_data(children, 2));
			if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(use_proxy_but)))
			{
				appdata->settings.use_proxy_script = TRUE;
			}
			if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(no_proxy_but)))
			{
				appdata->settings.use_proxy_script = FALSE;
			}


			/*
			 * Make the wizard more intuative by disabling or setting the context of pages based
			 * on previous answers. This is currently only for the URL \ Proxy page
			 */
			GtkWidget *url_page;
			url_page = gtk_notebook_get_nth_page (nb, 3);
			GList *url_page_children = gtk_container_get_children(GTK_CONTAINER(url_page));

			GtkWidget *url_label;
			url_label = GTK_WIDGET(g_list_nth_data(url_page_children, 0));
			HildonEntry *urlp_entry;
			urlp_entry = HILDON_ENTRY(g_list_nth_data(url_page_children, 1));
			GtkWidget *url_info_label;
			url_info_label = GTK_WIDGET(g_list_nth_data(url_page_children, 2));

			if(appdata->settings.provider == OTHER_BETAMAX)
			{
				hildon_entry_set_text (HILDON_ENTRY(urlp_entry), "https://www.provider.com/myaccount/sendsms.php");
				
				gtk_label_set_text(GTK_LABEL(url_label), "Betamax URL");
				gtk_label_set_text(GTK_LABEL(url_info_label),
						"Please enter the URL of the sms page. \nThis is normally in the format \nhttps://www.provider.com/myaccount/sendsms.php");

			}
			else if(appdata->settings.provider == WEBSMSRU || appdata->settings.provider == BLUEFACE || appdata->settings.provider == EXETEL
					|| appdata->settings.provider == PENNYTEL)
			{
				gtk_label_set_text(GTK_LABEL(url_label), "Ignore Setting");
				gtk_label_set_text(GTK_LABEL(url_info_label), "Please press next.");
				gtk_editable_set_editable(GTK_EDITABLE(urlp_entry), FALSE);
			}
			else if(appdata->settings.use_proxy_script)
			{
				gtk_label_set_text(GTK_LABEL(url_label), "Cabbage Script Address (http://...)");
				gtk_label_set_text(GTK_LABEL(url_info_label), "Enter the cabbage script address");
			}
			else
			{
				gtk_label_set_text(GTK_LABEL(url_label), "Ignore Setting");
				gtk_editable_set_editable(GTK_EDITABLE(urlp_entry), FALSE);
				gtk_label_set_text(GTK_LABEL(url_info_label), "Please press next.");
			}



			return TRUE;
		}
		case 3:
		{
			HildonEntry *url_entry;
			url_entry = HILDON_ENTRY(g_list_nth_data(children, 1));

			const gchar *url = hildon_entry_get_text (HILDON_ENTRY(url_entry));
			if ((strlen (url) != 0))
			{
				/*
				 * Potential memory leak here
				 */
				if(appdata->settings.proxy_url != NULL)
					g_free(appdata->settings.proxy_url);

				appdata->settings.proxy_url = g_strdup(url);
				return TRUE;
			}
			else
			{
				return FALSE;
			}

		}
		case 4:
		{
			GtkRadioButton *yes_save_but = GTK_RADIO_BUTTON(g_list_nth_data(children, 1));
			GtkRadioButton *no_save_but = GTK_RADIO_BUTTON(g_list_nth_data(children, 2));
			if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(yes_save_but)))
			{
				appdata->settings.savemsg = TRUE;
			}
			if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(no_save_but)))
			{
				appdata->settings.savemsg = FALSE;
			}
				return TRUE;
		}
		default:
			return TRUE;
	}

}

void aboutButton_clicked (GtkButton* button, AppData *appdata)
{
	he_about_dialog_present(GTK_WINDOW(appdata->messageWindow),
	                        APP_NAME,
	                        NULL,
	                        APP_VER,
	                        "Send web texts to Irish Mobile Operators \nand selected VoIP operators.",
	                        "(c) 2010 Thom Troy",
	                        "http://webtexter.garage.maemo.org",
	                        "https://bugs.maemo.org/enter_bug.cgi?product=webtexter",
	                        "https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=8DVWR56KA5F48");


}


void accountBox_clicked (GtkButton* button, AppData *appdata)
{
  	int account = gtk_combo_box_get_active (GTK_COMBO_BOX(appdata->accountBox));
    get_settings(appdata->gconf_client, account, &appdata->settings);
	
	g_debug("INFO: Setting to account: %s\n", appdata->settings.accountname);
}


void sendButton_clicked (GtkButton* button, AppData *appdata)
{
	GtkWidget *banner;
	/*appdata->msgBuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(appdata->msgEditor));*/
	appdata->toBuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(appdata->toEditor));
	gint msgCharCount = gtk_text_buffer_get_char_count(GTK_TEXT_BUFFER(appdata->msgBuffer));
	gint toCharCount = gtk_text_buffer_get_char_count(GTK_TEXT_BUFFER(appdata->toBuffer));

	GtkTextIter tostart;
	GtkTextIter toend;
	gtk_text_buffer_get_start_iter (GTK_TEXT_BUFFER(appdata->toBuffer), &tostart);
	gtk_text_buffer_get_end_iter (GTK_TEXT_BUFFER(appdata->toBuffer), &toend);

	gchar* to;
	to = gtk_text_buffer_get_text(GTK_TEXT_BUFFER(appdata->toBuffer), &tostart,
												&toend, FALSE);
	if(toCharCount == 0)
	{
		banner = hildon_banner_show_information(GTK_WIDGET(appdata->messageWindow), NULL,
				"Cannot send. No number entered");
	}
	else if(strcmp(to, PORTRAIT_ENABLE) == 0)
	{
		if(appdata->settings.orientation_enabled)
		{
			appdata->settings.orientation_enabled = FALSE;
			orientation_disable_accelerometers(appdata);
		}
		else
		{
			g_debug("portrait mode enabled");
			orientation_init(appdata);
			appdata->settings.orientation_enabled = TRUE;
		}
	}
	else if(strcmp(to, EXTRA_LOGGING_ENABLE) == 0)
	{
		if(appdata->settings.extra_logging)
		{
			banner = hildon_banner_show_information(GTK_WIDGET(appdata->messageWindow), NULL,
							"Extra Logging disabled.");
			g_debug("extra_logging disabled");
			appdata->settings.extra_logging = FALSE;
		}
		else
		{
			banner = hildon_banner_show_information(GTK_WIDGET(appdata->messageWindow), NULL,
										"Extra Logging Enabled.");
			g_debug("extra_logging enabled");
			appdata->settings.extra_logging = TRUE;
		}
	}
	else if(msgCharCount == 0)
	{
		banner = hildon_banner_show_information(GTK_WIDGET(appdata->messageWindow), NULL,
				"Cannot send. Message is empty");
	}
	else if (msgCharCount > get_max_msg_size(&appdata->settings))
	{
		gchar *message = g_strdup_printf("Message Cannot be greater than %d Characters", get_max_msg_size(&appdata->settings));
		banner = hildon_banner_show_information(GTK_WIDGET(appdata->messageWindow), NULL,
						message);
		g_free(message);
	}
	else
	{
		/*
		 * confirm we are connected and if we are try to send a message
		 * The progress indicator starts here and ends after confirmation of message sent \ not sent
		 * in prepare_send in http_sender or else here if iap connection is false or in network_utilities
		 */
		appdata->msg_to_send = TRUE;
		hildon_gtk_window_set_progress_indicator(GTK_WINDOW(appdata->messageWindow), 1);
		if(get_iap_connection(appdata) == FALSE)
		{
			appdata->msg_to_send = FALSE;
			hildon_gtk_window_set_progress_indicator(GTK_WINDOW(appdata->messageWindow), 0);
			g_debug("iap not connected\n");
			banner = hildon_banner_show_information(GTK_WIDGET(appdata->messageWindow), NULL,
									"Error while trying to get an Internet Connection");
		}
	}
	g_free(to);
}

static void msg_changed(GtkTextBuffer *textbuffer, AppData *appdata)
{
	/*
	 * TODO Should probably stop the user from entering more text after we reach the MAX_MSG_SIZE
	*/

	gint msgCharCount = gtk_text_buffer_get_char_count(GTK_TEXT_BUFFER(appdata->msgBuffer));
	gchar* labelText = g_strdup_printf("%d Characters", msgCharCount);
	gtk_label_set_text(GTK_LABEL(appdata->msgSizeLabel), labelText);

	if(msgCharCount == get_max_msg_size(&appdata->settings) + 1)
	{
		GtkWidget *msgToBigBanner;
		gchar *message = g_strdup_printf("Message Cannot be greater than %d Characters", get_max_msg_size(&appdata->settings));
		msgToBigBanner = hildon_banner_show_information(GTK_WIDGET(appdata->messageWindow), NULL,
						message);
		g_free(message);
	}

	g_free(labelText);
}

/*
 * Helper \ Gui Functions
 */

gint create_settings_wizard(AppData *appdata)
{
	GtkWidget *wizard, *notebook;
	GtkWidget *account_hbox, *user_hbox, *pass_hbox, *number_hbox;
	GtkWidget *account_label, *user_label, *pass_label, *number_label, *prov_label, *proxy_label, *done_label, *proxy_url_label;
	GtkWidget *proxy_info_label, *url_info_label, *savemsg_label, *savemsg_info_label;
	GtkWidget *proxy_url_entry;
	GtkWidget *voda_button, *o2_button, *met_button, *three_button, *blueface_button, *voipcheap_button, *smsdiscount_button;
	GtkWidget *lowratevoip_button, *other_betamax_button, *websmsru_button, *exetel_button, *pennytel_button;
	GtkWidget *yes_proxy, *no_proxy;
	GtkWidget *yes_savemsg, *no_savemsg;
	GtkWidget *up_hbox, *prov_hbox, *proxy_hbox, *proxy_url_hbox, *savemsg_hbox;
	GtkWidget *prov_panable;

	notebook = gtk_notebook_new ();

	prov_panable = hildon_pannable_area_new ();
	up_hbox = gtk_vbox_new(FALSE, 8);
	prov_hbox = gtk_vbox_new(FALSE, 8);
	proxy_hbox = gtk_vbox_new(FALSE, 8);
	proxy_url_hbox = gtk_vbox_new(FALSE, 8);
	savemsg_hbox = gtk_vbox_new(FALSE, 8);
	
	account_hbox = gtk_hbox_new(FALSE, 8);
	user_hbox = gtk_hbox_new(FALSE, 8);
	pass_hbox = gtk_hbox_new(FALSE, 8);
	number_hbox = gtk_hbox_new(FALSE, 8);

	char* voda = VODA_L;
	char* o2 = O2_L;
	char* three = THREE_L;
	char* met = MET_L;
	char* bluef = BLUEFACE_L;
	char* voipc = VOIPCHEAP_L;
	char* smsdisc = SMSDISC_L;
	char* lowrv = LOWRV_L;
	char* otherbm = OTHER_BETA_L;
	char* websmsru = WEBSMSRU_L;
	char* exetel = EXETEL_L;
	char* pennytel = PENNYTEL_L;

	account_label = gtk_label_new ("Account name");
	gtk_label_set_width_chars (GTK_LABEL(account_label), 12);
	user_label = gtk_label_new ("Username");
	gtk_label_set_width_chars (GTK_LABEL(user_label), 12);
	pass_label = gtk_label_new ("Password");
	gtk_label_set_width_chars (GTK_LABEL(pass_label), 12);
	number_label = gtk_label_new("Phone Number");
	gtk_label_set_width_chars (GTK_LABEL(number_label), 12);
	prov_label = gtk_label_new ("Provider");
	proxy_label = gtk_label_new ("Use Cabbage Web Proxy");
	proxy_url_label = gtk_label_new ("Web Proxy Address (http://...");
	done_label = gtk_label_new(
		"Your settings are now configured. \nEnjoy using webtexter");
	proxy_info_label = gtk_label_new(
		"Send through the cabbage style web scripts.\nNote: These scripts are mainly aimed at Irish users.\nNever used for Blueface, websms.ru, Exetel \nPennyTel or Other Betamax");
	url_info_label = gtk_label_new(
		"For a named provider:\nif web proxy selected enter script address\nif web proxy not selected enter anything\nFor Other Betamax providers:\nplease enter the URL of the sms page. \nThis is normally in the format \nhttps://www.provider.com/myaccount/sendsms.php");
	savemsg_label = gtk_label_new("Save sent messages:");
	savemsg_info_label = gtk_label_new(
		"Allows you to save sent messages.\nAfter saving they appear in the \nconversations application");

	appdata->account_entry = hildon_entry_new (HILDON_SIZE_AUTO);
	appdata->user_entry = hildon_entry_new (HILDON_SIZE_AUTO);
	appdata->pass_entry = hildon_entry_new (HILDON_SIZE_AUTO);
	appdata->number_entry = hildon_entry_new (HILDON_SIZE_AUTO);
	proxy_url_entry = hildon_entry_new (HILDON_SIZE_AUTO);
	gtk_entry_set_visibility(GTK_ENTRY(appdata->pass_entry), FALSE);
	hildon_entry_set_placeholder (HILDON_ENTRY (appdata->account_entry),
	                                  "Enter Account name");
	hildon_entry_set_placeholder (HILDON_ENTRY (appdata->user_entry),
	                                  "Enter Username");
	hildon_entry_set_placeholder (HILDON_ENTRY (appdata->pass_entry),
		                                  "Enter Password");

	o2_button = hildon_gtk_radio_button_new(HILDON_SIZE_AUTO , NULL);
	gtk_button_set_label(GTK_BUTTON(o2_button), o2);
	gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(o2_button), FALSE);
	hildon_gtk_widget_set_theme_size(o2_button, HILDON_SIZE_FINGER_HEIGHT);

	voda_button = hildon_gtk_radio_button_new_from_widget(HILDON_SIZE_AUTO , GTK_RADIO_BUTTON(o2_button));
	gtk_button_set_label(GTK_BUTTON(voda_button), voda);
	gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(voda_button), FALSE);
	hildon_gtk_widget_set_theme_size(voda_button, HILDON_SIZE_FINGER_HEIGHT);

	met_button = hildon_gtk_radio_button_new_from_widget(HILDON_SIZE_AUTO , GTK_RADIO_BUTTON(voda_button));
	gtk_button_set_label(GTK_BUTTON(met_button), met);
	gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(met_button), FALSE);
	hildon_gtk_widget_set_theme_size(met_button, HILDON_SIZE_FINGER_HEIGHT);

	three_button = hildon_gtk_radio_button_new_from_widget(HILDON_SIZE_AUTO , GTK_RADIO_BUTTON(met_button));
	gtk_button_set_label(GTK_BUTTON(three_button), three);
	gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(three_button), FALSE);
	hildon_gtk_widget_set_theme_size(three_button, HILDON_SIZE_FINGER_HEIGHT);

	blueface_button = hildon_gtk_radio_button_new_from_widget(HILDON_SIZE_AUTO , GTK_RADIO_BUTTON(three_button));
	gtk_button_set_label(GTK_BUTTON(blueface_button), bluef);
	gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(blueface_button), FALSE);
	hildon_gtk_widget_set_theme_size(blueface_button, HILDON_SIZE_FINGER_HEIGHT);

	voipcheap_button = hildon_gtk_radio_button_new_from_widget(HILDON_SIZE_AUTO , GTK_RADIO_BUTTON(blueface_button));
	gtk_button_set_label(GTK_BUTTON(voipcheap_button), voipc);
	gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(voipcheap_button), FALSE);
	hildon_gtk_widget_set_theme_size(voipcheap_button, HILDON_SIZE_FINGER_HEIGHT);

	smsdiscount_button = hildon_gtk_radio_button_new_from_widget(HILDON_SIZE_AUTO , GTK_RADIO_BUTTON(voipcheap_button));
	gtk_button_set_label(GTK_BUTTON(smsdiscount_button), smsdisc);
	gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(smsdiscount_button), FALSE);
	hildon_gtk_widget_set_theme_size(smsdiscount_button, HILDON_SIZE_FINGER_HEIGHT);

	lowratevoip_button = hildon_gtk_radio_button_new_from_widget(HILDON_SIZE_AUTO , GTK_RADIO_BUTTON(smsdiscount_button));
	gtk_button_set_label(GTK_BUTTON(lowratevoip_button), lowrv);
	gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(lowratevoip_button), FALSE);
	hildon_gtk_widget_set_theme_size(lowratevoip_button, HILDON_SIZE_FINGER_HEIGHT);

	other_betamax_button = hildon_gtk_radio_button_new_from_widget(HILDON_SIZE_AUTO , GTK_RADIO_BUTTON(lowratevoip_button));
	gtk_button_set_label(GTK_BUTTON(other_betamax_button), otherbm);
	gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(other_betamax_button), FALSE);
	hildon_gtk_widget_set_theme_size(other_betamax_button, HILDON_SIZE_FINGER_HEIGHT);

	websmsru_button = hildon_gtk_radio_button_new_from_widget(HILDON_SIZE_AUTO , GTK_RADIO_BUTTON(other_betamax_button));
	gtk_button_set_label(GTK_BUTTON(websmsru_button), websmsru);
	gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(websmsru_button), FALSE);
	hildon_gtk_widget_set_theme_size(websmsru_button, HILDON_SIZE_FINGER_HEIGHT);

	exetel_button = hildon_gtk_radio_button_new_from_widget(HILDON_SIZE_AUTO , GTK_RADIO_BUTTON(websmsru_button));
	gtk_button_set_label(GTK_BUTTON(exetel_button), exetel);
	gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(exetel_button), FALSE);
	hildon_gtk_widget_set_theme_size(exetel_button, HILDON_SIZE_FINGER_HEIGHT);

	pennytel_button = hildon_gtk_radio_button_new_from_widget(HILDON_SIZE_AUTO , GTK_RADIO_BUTTON(exetel_button));
	gtk_button_set_label(GTK_BUTTON(pennytel_button), pennytel);
	gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(pennytel_button), FALSE);
	hildon_gtk_widget_set_theme_size(pennytel_button, HILDON_SIZE_FINGER_HEIGHT);

	yes_proxy = hildon_gtk_radio_button_new(HILDON_SIZE_AUTO , NULL);
	gtk_button_set_label(GTK_BUTTON(yes_proxy), "Use Cabbage Web Proxy");
	gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(yes_proxy), FALSE);
	hildon_gtk_widget_set_theme_size(yes_proxy, HILDON_SIZE_FINGER_HEIGHT);
	no_proxy = hildon_gtk_radio_button_new_from_widget(HILDON_SIZE_AUTO , GTK_RADIO_BUTTON(yes_proxy));
	gtk_button_set_label(GTK_BUTTON(no_proxy), "Don't use Cabbage Web Proxy");
	gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(no_proxy), FALSE);
	hildon_gtk_widget_set_theme_size(no_proxy, HILDON_SIZE_FINGER_HEIGHT);

	yes_savemsg = hildon_gtk_radio_button_new(HILDON_SIZE_AUTO , NULL);
	gtk_button_set_label(GTK_BUTTON(yes_savemsg), "Save sent messages");
	gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(yes_savemsg), FALSE);
	hildon_gtk_widget_set_theme_size(yes_savemsg, HILDON_SIZE_FINGER_HEIGHT);
	no_savemsg = hildon_gtk_radio_button_new_from_widget(HILDON_SIZE_AUTO , GTK_RADIO_BUTTON(yes_savemsg));
	gtk_button_set_label(GTK_BUTTON(no_savemsg), "Don't save sent messages");
	gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(no_savemsg), FALSE);
	hildon_gtk_widget_set_theme_size(no_savemsg, HILDON_SIZE_FINGER_HEIGHT);

	/*
	 * Set Default or already set values
	 */
	if(appdata->settings.accountname != NULL)
	{
		hildon_entry_set_text (HILDON_ENTRY(appdata->account_entry), appdata->settings.accountname);
	}
	if(appdata->settings.username != NULL)
	{
		hildon_entry_set_text (HILDON_ENTRY(appdata->user_entry), appdata->settings.username);
	}
	if(appdata->settings.password != NULL)
	{
		hildon_entry_set_text (HILDON_ENTRY(appdata->pass_entry), appdata->settings.password);
	}
	if(appdata->settings.number != NULL)
	{
		hildon_entry_set_text (HILDON_ENTRY(appdata->number_entry), appdata->settings.number);
	}
	if(appdata->settings.proxy_url != NULL)
		hildon_entry_set_text (HILDON_ENTRY(proxy_url_entry), appdata->settings.proxy_url);

	if(appdata->settings.provider != 0)
	{
		switch(appdata->settings.provider){
			case O2:
			{
				gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(o2_button), TRUE);
				break;
			}
			case VODAFONE:
			{
				gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(voda_button), TRUE);
				break;
			}
			case METEOR:
			{
				gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(met_button), TRUE);
				break;
			}
			case THREE:
			{
				gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(three_button), TRUE);
				break;
			}
			case BLUEFACE:
			{
				gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(blueface_button), TRUE);
				break;
			}
			case VOIPCHEAP:
			{
				gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(voipcheap_button), TRUE);
				break;
			}
			case SMSDISCOUNT:
			{
				gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(smsdiscount_button), TRUE);
				break;
			}
			case LOWRATEVOIP:
			{
				gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(lowratevoip_button), TRUE);
				break;
			}
			case OTHER_BETAMAX:
			{
				gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(other_betamax_button), TRUE);
				break;
			}
			case WEBSMSRU:
			{
				gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(websmsru_button), TRUE);
				break;
			}
			case EXETEL:
			{
				gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(exetel_button), TRUE);
				break;
			}
			case PENNYTEL:
			{
				gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(pennytel_button), TRUE);
				break;
			}
		}
	}
	else
	{
		appdata->settings.provider = O2;
	}
	if(appdata->settings.use_proxy_script)
	{
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(yes_proxy), TRUE);
	}
	else
	{
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(no_proxy), TRUE);
	}

	if(appdata->settings.savemsg)
	{
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(yes_savemsg), TRUE);
	}
	else
	{
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(no_savemsg), TRUE);
	}

	gtk_box_pack_start (GTK_BOX (account_hbox), account_label, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (account_hbox), appdata->account_entry, TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (up_hbox), account_hbox, FALSE, FALSE, 0);
	
	gtk_box_pack_start (GTK_BOX (user_hbox), user_label, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (user_hbox), appdata->user_entry, TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (up_hbox), user_hbox, FALSE, FALSE, 0);
	
	gtk_box_pack_start (GTK_BOX (pass_hbox), pass_label, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (pass_hbox), appdata->pass_entry, TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (up_hbox), pass_hbox, FALSE, FALSE, 0);
	
	gtk_box_pack_start (GTK_BOX (number_hbox), number_label, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (number_hbox), appdata->number_entry, TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (up_hbox), number_hbox, FALSE, FALSE, 0);
	
	gtk_box_pack_start (GTK_BOX (prov_hbox), prov_label, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (prov_hbox), o2_button, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (prov_hbox), voda_button, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (prov_hbox), met_button, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (prov_hbox), three_button, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (prov_hbox), blueface_button, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (prov_hbox), voipcheap_button, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (prov_hbox), smsdiscount_button, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (prov_hbox), lowratevoip_button, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (prov_hbox), other_betamax_button, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (prov_hbox), websmsru_button, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (prov_hbox), exetel_button, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (prov_hbox), pennytel_button, FALSE, FALSE, 0);

	hildon_pannable_area_add_with_viewport (
	    HILDON_PANNABLE_AREA (prov_panable), prov_hbox);


	gtk_box_pack_start (GTK_BOX (proxy_hbox), proxy_label, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (proxy_hbox), yes_proxy, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (proxy_hbox), no_proxy, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (proxy_hbox), proxy_info_label, FALSE, FALSE, 0);



	gtk_box_pack_start (GTK_BOX (proxy_url_hbox), proxy_url_label, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (proxy_url_hbox), proxy_url_entry, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (proxy_url_hbox), url_info_label, FALSE, FALSE, 0);

	gtk_box_pack_start (GTK_BOX (savemsg_hbox), savemsg_label, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (savemsg_hbox), yes_savemsg, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (savemsg_hbox), no_savemsg, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (savemsg_hbox), savemsg_info_label, FALSE, FALSE, 0);

	/* Append pages */
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), up_hbox, NULL);
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), prov_panable, NULL);
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), proxy_hbox, NULL);
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), proxy_url_hbox, NULL);
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), savemsg_hbox, NULL);
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), done_label, NULL);


	wizard = hildon_wizard_dialog_new(GTK_WINDOW(appdata->messageWindow), "Settings", GTK_NOTEBOOK(notebook));

	/* Set a handler for "switch-page" signal */
	g_signal_connect (G_OBJECT (notebook),
	                  "switch-page",
	                  G_CALLBACK (on_page_switch),
	                  GTK_DIALOG(wizard));

	gint response = 0;
	g_signal_connect (G_OBJECT (wizard),
		                  "response",
		                  G_CALLBACK (wizard_response),
		                  &response);

	/* Set a function to decide if user can go to next page  */
	hildon_wizard_dialog_set_forward_page_func (HILDON_WIZARD_DIALOG (wizard),
	                                            some_page_func, appdata, NULL);


	gtk_widget_show_all (wizard);
	gtk_dialog_run (GTK_DIALOG (wizard));
	gtk_widget_destroy(GTK_WIDGET(wizard));
	return response;


}



int main( int argc, char* argv[] )
{
    /* Create needed variables */
	AppData appdata;
	appdata.contactList = (GList *) NULL;
	appdata.connection = NULL;

	GtkWidget *hboxaccount, *hboxtop, *hboxbottom, *vbox;
	GtkWidget *toButton, *settingsButton, *deleteButton, *newButton;
	GtkWidget *accountLabel;

	gboolean need_wizard = FALSE;

	int account;

	osso_return_t ret;

	/*locale_init();*/
	g_thread_init(NULL);

	/* Initialize the GTK. */
	hildon_gtk_init( &argc, &argv );

    appdata.osso_context = osso_initialize(APP_SERVICE, APP_VER, TRUE, NULL);
	if (appdata.osso_context == NULL)
    {
    	g_debug ("osso_initialize failed.\n");
    	exit (1);
    }

	appdata.con = osso_get_sys_dbus_connection(appdata.osso_context);


    /* start GConf Client */
    appdata.gconf_client = gconf_client_get_default();

    gconf_client_add_dir (appdata.gconf_client, "/apps/m_webtexter",
                    GCONF_CLIENT_PRELOAD_NONE, NULL);

    appdata.settings.orientation_enabled = FALSE;
    if(!get_settings(appdata.gconf_client, 0, &appdata.settings))
    	need_wizard = TRUE;

    /* Create the hildon program and setup the title */
    appdata.program = HILDON_PROGRAM(hildon_program_get_instance());
    g_set_application_name("WebTexter");

    /* Create HildonWindow and set it to HildonProgram */
    appdata.messageWindow = HILDON_STACKABLE_WINDOW(hildon_stackable_window_new());
    hildon_program_add_window(appdata.program, HILDON_WINDOW(appdata.messageWindow));

    /*
     * create the menu
     */
    appdata.menu = HILDON_APP_MENU (hildon_app_menu_new ());
    appdata.aboutButton = gtk_button_new_with_label ("About");
    settingsButton = gtk_button_new_with_label("Edit");
    deleteButton = gtk_button_new_with_label("Delete");
    newButton = gtk_button_new_with_label("New");
    g_signal_connect_after (appdata.aboutButton, "clicked", G_CALLBACK (aboutButton_clicked), &appdata);
    g_signal_connect (G_OBJECT (settingsButton), "clicked",
                             G_CALLBACK (settingsButton_clicked), &appdata);
    g_signal_connect (G_OBJECT (deleteButton), "clicked",
                             G_CALLBACK (deleteButton_clicked), &appdata);
    g_signal_connect (G_OBJECT (newButton), "clicked",
                             G_CALLBACK (newButton_clicked), &appdata);
    hildon_app_menu_append (appdata.menu, GTK_BUTTON (appdata.aboutButton));
    hildon_app_menu_append (appdata.menu, GTK_BUTTON (settingsButton));
    hildon_app_menu_append (appdata.menu, GTK_BUTTON (deleteButton));
    hildon_app_menu_append (appdata.menu, GTK_BUTTON (newButton));

    gtk_widget_show_all (GTK_WIDGET (appdata.menu));
    hildon_window_set_app_menu (HILDON_WINDOW (appdata.messageWindow), appdata.menu);


    /*
    * create the main window UI
    */
    hboxaccount = gtk_hbox_new(FALSE, 8);
    hboxtop = gtk_hbox_new(FALSE, 8);
    hboxbottom = gtk_hbox_new(FALSE, 8);
    vbox = gtk_vbox_new(FALSE, 8);
	
	accountLabel = gtk_label_new ("From ");
	appdata.accountBox = gtk_combo_box_new_text ();
	for (account = 0; ; account++) {
		if (!get_settings(appdata.gconf_client, account, &appdata.settings))
			break;
		gtk_combo_box_append_text (GTK_COMBO_BOX(appdata.accountBox), appdata.settings.accountname);
	}
	gtk_combo_box_set_active (GTK_COMBO_BOX(appdata.accountBox), 0);
	get_settings(appdata.gconf_client, 0, &appdata.settings);
	
    toButton = gtk_button_new_with_label("To :");
    appdata.sendButton = gtk_button_new_with_label("Send");

    appdata.msgEditor = hildon_text_view_new ();
    appdata.toEditor = hildon_text_view_new ();
    g_object_set (G_OBJECT (appdata.toEditor), "hildon-input-default", HILDON_GTK_INPUT_MODE_NUMERIC, NULL);

    gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW(appdata.msgEditor), GTK_WRAP_WORD_CHAR);
    appdata.msgSizeLabel = gtk_label_new ("0 Characters");

    gtk_box_pack_start (GTK_BOX (hboxaccount), accountLabel, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hboxaccount), appdata.accountBox, TRUE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX (hboxtop), toButton, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hboxtop), appdata.toEditor, TRUE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX (hboxbottom), appdata.sendButton, FALSE, FALSE, 0);

    gtk_box_pack_end (GTK_BOX (hboxbottom), appdata.msgSizeLabel, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox), hboxaccount, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox), hboxtop, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox), appdata.msgEditor, TRUE, TRUE, 0);
    gtk_box_pack_end (GTK_BOX (vbox), hboxbottom, FALSE, FALSE, 0);

    gtk_container_add (GTK_CONTAINER (appdata.messageWindow), vbox);

    /*
     * The msgBuffer has to point to the one in the msgEditor to get the signal changed events.
     */
    appdata.msgBuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(appdata.msgEditor));

	if(need_wizard) {
		gtk_widget_set_sensitive(GTK_WIDGET(appdata.sendButton), FALSE);
		newButton_clicked(NULL, &appdata);
	}

	/* Quit program when window is closed. */
    g_signal_connect (G_OBJECT (appdata.messageWindow), "delete_event",
    	      G_CALLBACK (gtk_main_quit), NULL);

    /* Quit program when window is otherwise destroyed. */
    g_signal_connect (G_OBJECT (appdata.messageWindow), "destroy",
    		G_CALLBACK (gtk_main_quit), NULL);

    /* Callbacks for button presses */
    g_signal_connect (G_OBJECT (appdata.accountBox), "changed",
                             G_CALLBACK (accountBox_clicked), &appdata);
							 
    g_signal_connect (G_OBJECT (toButton), "clicked",
                         G_CALLBACK (toButton_clicked), &appdata);

    g_signal_connect (G_OBJECT (appdata.sendButton), "clicked",
                             G_CALLBACK (sendButton_clicked), &appdata);

    g_signal_connect (G_OBJECT (appdata.msgBuffer), "changed",
                                 G_CALLBACK (msg_changed), &appdata);

    ret = osso_rpc_set_cb_f (appdata.osso_context,
                           APP_SERVICE,
                           APP_METHOD,
                           APP_SERVICE,
                           dbus_callback, GTK_WIDGET( appdata.messageWindow ));
	if (ret != OSSO_OK) {
		g_debug ("osso_rpc_set_cb_f failed: %d.\n", ret);
	    exit (1);
	}

    /* Begin the main application */
    gtk_widget_show_all ( GTK_WIDGET ( appdata.messageWindow ) );

    /*
     * Should probably disable accelometers when appliction in background to save power
     */
    if(appdata.settings.orientation_enabled)
    	orientation_init(&appdata);


    gtk_main();

    if(appdata.settings.orientation_enabled)
    	orientation_disable_accelerometers(&appdata);

    osso_deinitialize(appdata.osso_context);
    g_object_unref (G_OBJECT (appdata.gconf_client));

    if(appdata.connection != NULL)
    {
    	g_object_unref(G_OBJECT(appdata.connection));
    }
    g_free(appdata.settings.username);
    g_free(appdata.settings.password);
    g_free(appdata.settings.number);
    g_free(appdata.settings.proxy_url);

    /* Exit */
    return 0;
}
