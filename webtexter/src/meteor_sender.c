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
 Name        : meteor_sender.c
 Author      : Matrim
 Version     : 0.1
 Description : Send messages directly to meteor
 ============================================================================
 */

#include "http_sender.h"
#include "meteor_sender.h"
#include "settings.h"


gboolean meteor_login(AppSettings *settings)
{
	http_sender *sender;
	sender = g_new0(http_sender, 1);

	gchar *user_encoded = url_encode(settings->username);
	gchar *pass_encoded = url_encode(settings->password);
	gchar *url = "https://www.mymeteor.ie/go/mymeteor-login-manager";
	gchar *post = g_strdup_printf("username=%s&userpass=%s&x=19&y=13&returnTo=%2F",
			user_encoded, pass_encoded);

	g_free(user_encoded);
	g_free(pass_encoded);

	http_send_curl(url, sender, HTTP_POST, post, NULL);

	if((g_strstr_len(sender->buffer->str, sender->buffer->len, "<h3>My Account</h3>")) != NULL)
	{
		g_free(post);
		g_string_free(sender->buffer, TRUE);
		return TRUE;
	}
	else
	{
		g_debug("unable to logon to Meteor\n");
		g_free(post);
		g_string_free(sender->buffer, TRUE);
		return FALSE;
	}

}


gboolean meteor_text_page(AppSettings *settings)
{
	http_sender *sender;
	sender = g_new0(http_sender, 1);

	gchar *url = "https://www.mymeteor.ie/go/freewebtext";
	http_send_curl(url, sender, HTTP_GET, NULL, NULL);

	if((g_strstr_len(sender->buffer->str, sender->buffer->len, "Free web texts left <input type=\"text\" value=\"0")) != NULL)
	{
		g_debug("meteor text page failed\n");
		g_string_free(sender->buffer, TRUE);
		return FALSE;
	}
	else
	{
		g_string_free(sender->buffer, TRUE);
		return TRUE;
	}
	return TRUE;
}

gboolean meteor_send_message(AppSettings *settings, gchar *to, gchar *message)
{
	http_sender *sender;
	sender = g_new0(http_sender, 1);

	gchar *to_encoded = url_encode(to);
	gchar *msg_encoded = url_encode(message);

	gchar *url = g_strdup_printf("https://www.mymeteor.ie/mymeteorapi/index.cfm?event=smsAjax&func=addEnteredMsisdns&ajaxRequest=addEnteredMSISDNs&remove=-&add=111111|%s|no%%20one",
			to_encoded);

	http_send_curl(url, sender, HTTP_GET, NULL, NULL);

	http_sender *sender_msg;
	sender_msg = g_new0(http_sender, 1);

	gchar *msg_url = g_strdup_printf("https://www.mymeteor.ie/mymeteorapi/index.cfm?event=smsAjax&func=sendSMS&ajaxRequest=sendSMS&messageText=%s",
			msg_encoded);

	http_send_curl(msg_url, sender_msg, HTTP_GET, NULL, NULL);

	g_free(to_encoded);
	g_free(msg_encoded);

	if((g_strstr_len(sender_msg->buffer->str, sender_msg->buffer->len, "showEl(\"sentTrue\")")) != NULL)
	{
		g_free(url);
		g_free(msg_url);
		g_string_free(sender->buffer, TRUE);
		g_string_free(sender_msg->buffer, TRUE);
		return TRUE;
	}
	else
	{
		g_debug("Meteor Message Not sent\n");
		g_free(url);
		g_free(msg_url);
		g_string_free(sender->buffer, TRUE);
		g_string_free(sender_msg->buffer, TRUE);
		return FALSE;
	}
	return TRUE;
}
