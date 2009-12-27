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
 Name        : web_proxy_sender.c
 Author      : Matrim
 Version     : 0.1
 Description : Send messages through the cabbage web scripts
 ============================================================================
 */

#include "web_proxy_sender.h"
#include "settings.h"
#include "network_utilities.h"
#include "http_sender.h"
#include "string.h"

gint web_proxy_send_message(AppSettings *settings, gchar* to, gchar* message, HTTP_Proxy *proxy)
{
	http_sender *sender;
	sender = g_new0(http_sender, 1);
	gchar * provider;
	switch(settings->provider)
	{
		case O2:
		{
			provider = O2_S;
			break;
		}
		case VODAFONE:
		{
			provider = VODA_S;
			break;
		}
		case METEOR:
		{
			provider = MET_S;
			break;
		}
		case THREE:
		{
			provider = THREE_S;
			break;
		}
		case VOIPCHEAP:
		{
			provider = VOIPCHEAP_S;
			break;
		}
		case SMSDISCOUNT:
		{
			provider = SMSDISC_S;
			break;
		}
		case LOWRATEVOIP:
		{
			provider = LOWRV_S;
			break;
		}
		default:
		{
			g_debug("provider not supported by web script");
			return ERROR_OTHER;
		}
	}

	gchar *to_encoded = url_encode(to);
	gchar *msg_encoded = url_encode(message);
	gchar *user_encoded = url_encode(settings->username);
	gchar *pass_encoded = url_encode(settings->password);
	gchar *number_encoded = url_encode(settings->number);
	gchar *url = g_strdup_printf("%s?u=%s&p=%s&name=%s&d=%s&m=%s&s=%s",
			settings->proxy_url, number_encoded, pass_encoded,
			user_encoded, to_encoded, msg_encoded, provider);

	g_free(to_encoded);
	g_free(msg_encoded);
	g_free(user_encoded);
	g_free(pass_encoded);
	g_free(number_encoded);

	http_send_curl(url, sender, HTTP_GET, NULL, proxy);

	if(((g_strstr_len(sender->buffer->str, sender->buffer->len, "-1")) != NULL))
	{
		g_debug("Message Not sent to web proxy. Provider is %d \n Result is %s and url is %s",
						provider, sender->buffer->str, settings->proxy_url);
		g_free(url);
		g_string_free(sender->buffer, TRUE);
		return ERROR_LOGIN;
	}
	else if(((g_strstr_len(sender->buffer->str, sender->buffer->len, "-2")) != NULL))
	{
		g_debug("Message Not sent to web proxy. Provider is %d \n Result is %s and url is %s",
				provider, sender->buffer->str, settings->proxy_url);
		g_free(url);
		g_string_free(sender->buffer, TRUE);
		return ERROR_SEND;
	}
	else if(((g_strstr_len(sender->buffer->str, sender->buffer->len, "-5")) != NULL))
	{
		g_debug("Message Not sent to web proxy. Provider is %s \n Result is %s and url is %s",
						provider, sender->buffer->str, settings->proxy_url);
		g_free(url);
		g_string_free(sender->buffer, TRUE);
		return ERROR_OTHER;
	}
	else
	{
		g_free(url);
		g_debug("Message sent but there may still be errors. Result is %s", sender->buffer->str);
		g_string_free(sender->buffer, TRUE);
		return SUCCESS;
	}

	return ERROR_OTHER;
}

