/*
 *
 * Copyright (C) 2010 Cayden Meyer
 *
 * This is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License (GPL) as published by
 * the Free Software Foundation
 *
 * This is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with webtexter. If not, see <http://www.gnu.org/licenses/>.
 */

#include "exetel_sender.h"
#include "settings.h"
#include "network_utilities.h"
#include "http_sender.h"
#include "string.h"



gint exetel_send_message(AppSettings *settings, gchar* to, gchar* message, HTTP_Proxy *proxy)
{
	http_sender *sender;
	sender = g_new0(http_sender, 1);
	sender->timeout = settings->curl_timeout;
	gchar *to_encoded = url_encode(to);
	gchar *msg_encoded = url_encode(message);
	gchar *user_encoded = url_encode(settings->username);
	gchar *pass_encoded = url_encode(settings->password);
	gchar *url = g_strdup_printf("https://smsgw.exetel.com.au/sendsms/api_sms.php");
	gchar *post = g_strdup_printf("username=%s&password=%s&mobilenumber=%s&message=%s&sender=%s&messagetype=Text",
			user_encoded, pass_encoded,
			to_encoded, msg_encoded, user_encoded);

	g_free(to_encoded);
	g_free(msg_encoded);
	g_free(user_encoded);
	g_free(pass_encoded);

	http_send_curl(url, sender, HTTP_POST, post, proxy);

	if(((g_strstr_len(sender->buffer->str, sender->buffer->len, "OK")) != NULL))
	{
		g_free(url);
		g_free(post);
		g_string_free(sender->buffer, TRUE);
		return SUCCESS;
	}
	else
	{
		g_debug("Message Not sent to exetel. %s",sender->buffer->str);

		g_free(url);
		g_free(post);
		g_string_free(sender->buffer, TRUE);
		return ERROR_SEND;
	}

	return ERROR_OTHER;

}
