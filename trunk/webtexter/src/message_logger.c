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
#include <rtcom-eventlogger-plugins/sms.h>

gboolean save_message(gchar* to, gchar* msg)
{
	/*
	 * Need to remove trailing comma if it's there
	 * Should maybe save each message seperately
	 * Need to sort out group_id
	 *
	 * This function shouldn't be used on a device at the moment as it will probably break the conversations
	 * app. I'm not adding the data correctly
	 */
	RTComEl *el = rtcom_el_new ();
	if(!RTCOM_IS_EL(el))
	{
		g_debug("couldn't setup RTComEl\n");
		return FALSE;
	}

	/*Add a new read SMS event to the database*/

	RTComElEvent *ev = rtcom_el_event_new ();
	if(!ev)
	{
		g_debug("couldn't setup RTComElEvent\n");
		return FALSE;
	}
	RTCOM_EL_EVENT_SET_FIELD (ev, service, SERVICE);
	RTCOM_EL_EVENT_SET_FIELD (ev, event_type, EVENT_OUTBOUND);
	RTCOM_EL_EVENT_SET_FIELD (ev, is_read, TRUE);
	RTCOM_EL_EVENT_SET_FIELD (ev, local_uid, LOCAL_UID);
	RTCOM_EL_EVENT_SET_FIELD (ev, remote_uid, to);
	RTCOM_EL_EVENT_SET_FIELD (ev, start_time, time(NULL));
	/*RTCOM_EL_EVENT_SET_FIELD (ev, remote_ebook_uid, 1);*/
	RTCOM_EL_EVENT_SET_FIELD (ev, free_text, msg);

	rtcom_el_add_event (el, ev, NULL);
	rtcom_el_event_free (ev);
	g_object_unref(el);
	return TRUE;

}
