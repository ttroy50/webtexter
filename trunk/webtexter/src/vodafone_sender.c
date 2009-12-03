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
 Name        : vodafone_sender.c
 Author      : Matrim
 Version     : 0.1
 Description : Send a message directly to vodafone
 ============================================================================
 */

#include "http_sender.h"
#include "network_utilities.h"
#include "vodafone_sender.h"
#include "settings.h"
#include "string.h"


gboolean vodafone_login(AppSettings *settings, HTTP_Proxy *proxy)
{
	http_sender *sender;
	sender = g_new0(http_sender, 1);
	gchar * user_encoded = url_encode(settings->username);
	gchar * pass_encoded = url_encode(settings->password);
	gchar *url = g_strdup_printf("https://www.vodafone.ie/myv/services/login/Login.shtml?username=%s&password=%s",
			user_encoded, pass_encoded );

	g_free(user_encoded);
	g_free(pass_encoded);

	http_send_curl(url, sender, HTTP_GET, NULL, proxy);

	if((g_strstr_len(sender->buffer->str, sender->buffer->len, "Sign out")) != NULL)
	{
		g_free(url);
		g_string_free(sender->buffer, TRUE);
		return TRUE;
	}
	else
	{
		g_debug("Vodafone failed to login\n");
		g_free(url);
		g_string_free(sender->buffer, TRUE);
		return FALSE;
	}

}

gchar* vodafone_text_page(AppSettings *settings, HTTP_Proxy *proxy)
{
	gchar* apache_token;
	http_sender *sender;
	sender = g_new0(http_sender, 1);
	gchar *message_url = "https://www.vodafone.ie/myv/messaging/webtext/index.jsp";
	http_send_curl(message_url, sender, HTTP_GET, NULL, proxy);
	gchar* substring = g_strstr_len(sender->buffer->str, sender->buffer->len,
			"org.apache.struts.taglib.html.TOKEN");

	if(substring == NULL)
	{
		g_debug("Vodafone didn't get message page\n");

		return NULL;
	}
	else
	{
		gchar** tokenized = g_strsplit(substring, "\"", 4);
		if(tokenized != NULL)
		{
			apache_token = g_strdup(tokenized[2]);
		}
		else
		{
			g_string_free(sender->buffer, TRUE);
			g_debug("Vodafone tokenized is null");
			return NULL;
		}

		g_strfreev(tokenized);
		g_string_free(sender->buffer, TRUE);
		/*g_print("freeing substring");
		g_free(substring);
		g_print("substring freed");*/
		return apache_token;
	}
	return NULL;
}

gboolean vodafone_send_message(AppSettings *settings,gchar* apache_token, gchar *to,
		gchar *message, HTTP_Proxy *proxy)
{
	gchar* cleaned_to = vodafone_create_to(to);
	if(cleaned_to == NULL)
		return FALSE;

	gint num = 160 - strlen(message);
	http_sender *sender;
	sender = g_new0(http_sender, 1);
	gchar *msg_encoded = url_encode(message);

	gchar *url = "https://www.vodafone.ie/myv/messaging/webtext/Process.shtml";
	gchar *post = g_strdup_printf(
			"org.apache.struts.taglib.html.TOKEN=%s&message=%s&%s&x=47&y=11&futuredate=false&futuretime=false",
			apache_token, msg_encoded, cleaned_to);

	g_free(msg_encoded);
	g_free(cleaned_to);

	http_send_curl(url, sender, HTTP_POST, post, proxy);

	if((g_strstr_len(sender->buffer->str, sender->buffer->len, "Message sent!")) != NULL)
	{
		g_free(post);
		g_string_free(sender->buffer, TRUE);
		return TRUE;
	}
	else
	{
		g_debug("Vodafone Message Not sent\n");
		g_free(post);
		g_string_free(sender->buffer, TRUE);
		return FALSE;
	}
	return TRUE;
}

gchar* vodafone_create_to(char* to)
{
	gchar** tokenized = g_strsplit(to, ",", 5);
	gchar *created_to;
	guint length = g_strv_length(tokenized);
	if(tokenized != NULL)
	{
		/*
		 * There's bound to be a much better way to do this but for now this will do
		 * TODO should do some checks on number format for vodafone e.g. 00353 -> 0, 00 -> +
		 */
		switch(length)
		{
		case 1:
		{
			gchar* temp = url_encode(tokenized[0]);
			created_to = g_strdup_printf(
				"recipients[0]=%s&recipients[1]=&recipients[2]=&recipients[3]=&recipients[4]=",
						temp);
			g_free(temp);
			break;
		}
		case 2:
		{
			gchar* temp = url_encode(tokenized[0]);
			gchar* temp1 = url_encode(tokenized[1]);
			created_to = g_strdup_printf(
				"recipients[0]=%s&recipients[1]=%s&recipients[2]=&recipients[3]=&recipients[4]=",
					temp, temp1);
			g_free(temp);
			g_free(temp1);
			break;
		}
		case 3:
		{
			gchar* temp = url_encode(tokenized[0]);
			gchar* temp1 = url_encode(tokenized[1]);
			gchar* temp2 = url_encode(tokenized[2]);
			created_to = g_strdup_printf(
				"recipients[0]=%s&recipients[1]=%s&recipients[2]=%s&recipients[3]=&recipients[4]=",
					temp, temp1, temp2);
			g_free(temp);
			g_free(temp1);
			g_free(temp2);
			break;
		}
		case 4:
		{
			gchar* temp = url_encode(tokenized[0]);
			gchar* temp1 = url_encode(tokenized[1]);
			gchar* temp2 = url_encode(tokenized[2]);
			gchar* temp3 = url_encode(tokenized[3]);
			created_to = g_strdup_printf(
				"recipients[0]=%s&recipients[1]=%s&recipients[2]=%s&recipients[3]=%s&recipients[4]=",
					temp, temp1, temp2, temp3);
			g_free(temp);
			g_free(temp1);
			g_free(temp2);
			g_free(temp3);
			break;
		}
		case 5:
		{
			gchar* temp = url_encode(tokenized[0]);
			gchar* temp1 = url_encode(tokenized[1]);
			gchar* temp2 = url_encode(tokenized[2]);
			gchar* temp3 = url_encode(tokenized[3]);
			gchar* temp4 = url_encode(tokenized[4]);
			created_to = g_strdup_printf(
				"recipients[0]=%s&recipients[1]=%s&recipients[2]=%s&recipients[3]=%s&recipients[4]=%s",
					temp, temp1, temp2, temp3, temp4);
			g_free(temp);
			g_free(temp1);
			g_free(temp2);
			g_free(temp3);
			g_free(temp4);
			break;
		}
		default:
			created_to = NULL;

		}


		g_strfreev(tokenized);
		return created_to;
	}
	else
	{
		g_debug("Vodafone couldn't create to string as tokenized is null\n");
		return NULL;
	}



}
