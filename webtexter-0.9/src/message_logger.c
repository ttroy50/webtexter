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
 Name        : message_logger.c
 Author      : Matrim
 Version     : 0.1
 Description : Log messages to the sent folders
 ============================================================================
 */

#include "message_logger.h"
#include <rtcom-eventlogger/eventlogger.h>
#include <libebook/e-book.h>
#include <libosso-abook/osso-abook.h>
#include <uuid/uuid.h>
/*#include <rtcom-eventlogger-plugins/sms.h>*/

gboolean save_message(gchar* to, gchar* msg)
{
	/*
	 * Save messages to the rtcomm-eventlogger database at ~/.rtcom-eventlogger/el.db
	 * Some of this is modified from the smsimporter tool
	 *
	 * The saving works like
	 * 1. Open the addressbook
	 * 2. Split the to string into each number
	 * 3. Do a fuzzy lookup on each number in contacts to see if it is a contact
	 * 4. Save the message including ebook_uid into the database
	 *
	 */
	g_debug("in save_message");
	EBook *ebook;
	EBookQuery *query;
	GError *error = NULL;
	GList *contact, *c;
	char buf[512];

	/* Open the Ebook */

	ebook = e_book_new_system_addressbook (&error);
	if (!ebook) {
		g_debug ("Error opening system address book: %s", error->message);
		g_error_free (error);
		return FALSE;
	}

	if (!e_book_open (ebook, TRUE, &error)) {
		g_debug ("Error opening system address book: %s", error->message);
		g_error_free (error);
		return FALSE;
	}

	query = e_book_query_any_field_contains ("");

	if (!e_book_get_contacts (ebook, query, &contact, &error)) {
		g_debug ("Error getting contacts: %s", error->message);
		g_error_free (error);
		return FALSE;
	}

	/* prepare the to string */

	gchar** tokenized = g_strsplit(to, ",", -1);
	gchar *created_to;
	guint length = g_strv_length(tokenized);
	g_debug("tokenized length is %d \n", length);
	if(tokenized == NULL)
	{
		g_debug("unable to split to string when sending\n");
		return FALSE;
	}

	/* Open the RTCOM_Eventlogger*/
	RTComEl *el = rtcom_el_new ();
	if(!RTCOM_IS_EL(el))
	{
		g_debug("couldn't setup RTComEl\n");
		return FALSE;
	}

	int i;
	for(i = 0; i<length; i++)
	{
		g_debug("saving message for %s\n", tokenized[i]);
		char *remote_uid, *group_uid;

		remote_uid = g_strdup(tokenized[i]);

		g_debug("remote_uid length is %d", strlen(remote_uid));

		if((int) strlen(remote_uid) <= 1)
		{
			g_debug("remote_uid length is %d should continue", strlen(remote_uid));
			continue;
		}

    	/* group uid is the last 7 numbers from phone number */
		if ((int) (strlen (remote_uid) - 7) >= 0) {
			group_uid = remote_uid + strlen (remote_uid) - 7;
		} else {
			group_uid = remote_uid;
		}

		if(remote_uid == NULL || group_uid == NULL)
		{
			g_debug("either remote or group uid is NULL\n");
			g_strfreev(tokenized);
			g_free(remote_uid);
			g_object_unref(el);
			continue;
		}
		g_debug("remote_uid is %s \n", remote_uid);
		g_debug("group_uid is %s \n", group_uid);

		const gchar *remote_name = NULL, *remote_ebook_uid = NULL;
		for (c = contact; c != NULL; c = c->next) {
			unsigned int num;

			for (num = E_CONTACT_FIRST_PHONE_ID; num <= E_CONTACT_LAST_PHONE_ID; num++)
			{
				const gchar *number = NULL;

				number = e_contact_get_const (E_CONTACT (c->data), num);
				if (!number)
					continue;

				/*
				 * Because the group uid is the last 7 characters of the remote_uid, this allows us
				 * to do a fuzzy lookup of the contact, so that it will match a send to the
				 * international version of the number even if the number is in local format
				 * e.g. +3531234567 will match 01234567
				 *
				 */
				if (g_str_has_suffix (number, group_uid))
				{
					remote_ebook_uid = e_contact_get_const (E_CONTACT (c->data), E_CONTACT_UID);
					if (remote_ebook_uid)
					{
						remote_name = e_contact_get_const (E_CONTACT (c->data), E_CONTACT_FULL_NAME);
						if (!remote_name)
						{
							remote_name = e_contact_get_const (E_CONTACT (c->data), E_CONTACT_GIVEN_NAME);
							if (!remote_name)
							{
								remote_name = e_contact_get_const (E_CONTACT (c->data), E_CONTACT_FAMILY_NAME);
								if (!remote_name)
								{
									remote_name = e_contact_get_const (E_CONTACT (c->data), E_CONTACT_NICKNAME);
									if (!remote_name)
									{
										remote_name = e_contact_get_const (E_CONTACT (c->data), E_CONTACT_ORG);
									} else
									{
										remote_name = NULL;
									}
								}
							}
						}
					}
				}
			}
		}
		g_debug("remote_name is %s", remote_name);
		/*Add a new sent SMS event to the database*/

		RTComElEvent *ev = rtcom_el_event_new ();
		if(!ev)
		{
			g_debug("couldn't setup RTComElEvent\n");
			g_strfreev(tokenized);
			g_free(remote_uid);
			g_object_unref(el);
			continue;
		}
		RTCOM_EL_EVENT_SET_FIELD (ev, service, SERVICE);
		RTCOM_EL_EVENT_SET_FIELD (ev, event_type, EVENT_OUTBOUND);
		RTCOM_EL_EVENT_SET_FIELD (ev, is_read, TRUE);
		RTCOM_EL_EVENT_SET_FIELD (ev, local_uid, LOCAL_UID);
		RTCOM_EL_EVENT_SET_FIELD (ev, local_name, MYSELF);
		RTCOM_EL_EVENT_SET_FIELD (ev, remote_uid, remote_uid);
		RTCOM_EL_EVENT_SET_FIELD (ev, remote_name, remote_name);
		RTCOM_EL_EVENT_SET_FIELD (ev, remote_ebook_uid, remote_ebook_uid);
		RTCOM_EL_EVENT_SET_FIELD (ev, group_uid, group_uid);
		RTCOM_EL_EVENT_SET_FIELD (ev, start_time, time(NULL));
		RTCOM_EL_EVENT_SET_FIELD (ev, free_text, msg);

		gint id = rtcom_el_add_event (el, ev, NULL);

		if(id < 0)
		{
			g_debug("saving message and id is -1. Error encountered.");
		}
		else
		{
			/*
			 * Added because if a message token isn't added it means you can't open the message in conversations
			 * to forward it.
			 */

			uuid_t uuid;
			char key[UUID_STR_LEN + 1];
			uuid_generate_random (uuid);
			uuid_unparse(uuid, key);

			gchar * token = g_strdup_printf("webt-%s", key);
			g_debug("header is %s", token);
			gint header_id = rtcom_el_add_header(el, id, "message-token", token, NULL);
			if(header_id == -1)
				g_debug("header insertion failed");

			g_free(token);
		}
		rtcom_el_event_free (ev);
		g_free(remote_uid);
	}

	g_strfreev(tokenized);

	g_object_unref(el);
	return TRUE;

}
