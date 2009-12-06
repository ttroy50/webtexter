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
 Name        : three_sender.c
 Author      : Matrim
 Version     : 0.1
 Description : Send messages directly to three
 ============================================================================
 */

#include "http_sender.h"
#include "three_sender.h"
#include "settings.h"


gboolean three_login(AppSettings *settings)
{
	http_sender *sender;
	sender = g_new0(http_sender, 1);

	gchar *user_encoded = url_encode(settings->username);
	gchar *pass_encoded = url_encode(settings->password);

	gchar *url = g_strdup_printf("http://webtext.three.ie/processLogin.jsp?mobile=%s&pin=%s&serviceId=19088&originCountryPrefix=353",
			user_encoded, pass_encoded);

	http_send_curl(url, sender, HTTP_GET, NULL, NULL);

	g_free(user_encoded);
	g_free(pass_encoded);

	g_printf("%s", sender->buffer->str);
	if((g_strstr_len(sender->buffer->str, -1, "Invalid login")) == NULL)
	{
		g_free(url);
		g_string_free(sender->buffer, TRUE);
		return TRUE;
	}
	else
	{
		g_debug("failed to logon to THREE");
		g_free(url);
		g_string_free(sender->buffer, TRUE);
		return FALSE;
	}

}


gboolean three_text_page(AppSettings *settings)
{
	http_sender *sender;
	sender = g_new0(http_sender, 1);

	gchar *domain_url = "http://webtext.three.ie/send.jsp";
	http_send_curl(domain_url, sender, HTTP_GET, NULL, NULL);


	if((g_strstr_len(sender->buffer->str, sender->buffer->len, "Send Your Message")) != NULL)
	{
		g_debug("Unable to get THREE text page");
		g_free(domain_url);
		g_string_free(sender->buffer, TRUE);
		return FALSE;
	}
	else
	{
		g_free(domain_url);
		g_string_free(sender->buffer, TRUE);
		return TRUE;
	}
	return TRUE;
}

gboolean three_send_message(AppSettings *settings, gchar *to, gchar *message)
{
	http_sender *sender;
	sender = g_new0(http_sender, 1);

	gchar *to_encoded = url_encode(to);
	gchar *msg_encoded = url_encode(message);
	gchar *url = g_strdup_printf(
			"http://webtext.three.ie/processSendMessage.jsp?command=send&group=&Msg1=%s&grpSTR=&ConSTR=&msisdn=%s&country=353&NumMessages=",
			msg_encoded, to_encoded);

	g_free(to_encoded);
	g_free(msg_encoded);

	http_send_curl(url, sender, HTTP_GET, NULL, NULL);

	if((g_strstr_len(sender->buffer->str, -1, "Message Sent")) != NULL)
	{
		g_free(url);
		g_string_free(sender->buffer, TRUE);
		return TRUE;
	}
	else
	{
		g_debug("THREE message not sent\n");
		g_free(url);
		g_string_free(sender->buffer, TRUE);
		return FALSE;
	}
	return TRUE;
}
