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
 Name        : betamax_sender.c
 Author      : Matrim
 Version     : 0.3
 Description : Send messages directly to betamax sites e.g. voipcheap
 ============================================================================
 */

#include "betamax_sender.h"
#include "settings.h"
#include "network_utilities.h"
#include "http_sender.h"
#include "string.h"

gint betamax_send_message(AppSettings *settings, gchar* to, gchar* message, HTTP_Proxy *proxy)
{
	http_sender *sender;
	sender = g_new0(http_sender, 1);
	sender->timeout = settings->curl_timeout;

	gchar * baseurl;
	switch(settings->provider)
	{
		case VOIPCHEAP:
		{
			baseurl = "https://www.voipcheap.com/myaccount/sendsms.php";
			break;
		}
		case SMSDISCOUNT:
		{
			baseurl = "https://www.smsdiscount.com/myaccount/sendsms.php";
			break;
		}
		case LOWRATEVOIP:
		{
			baseurl = "https://www.lowratevoip.com/myaccount/sendsms.php";
			break;
		}
		case OTHER_BETAMAX:
		{
			baseurl = g_strdup(settings->proxy_url);
			break;
		}
		default:
		{
			g_debug("provider not supported by betamax");
			return ERROR_OTHER;
		}
	}

	gchar *to_encoded = url_encode(to);
	gchar *msg_encoded = url_encode(message);
	gchar *user_encoded = url_encode(settings->username);
	gchar *pass_encoded = url_encode(settings->password);
	gchar *number_encoded = url_encode(settings->number);
	gchar *url = g_strdup_printf("username=%s&password=%s&from=%s&to=%s&text=%s",
			user_encoded, pass_encoded,
			number_encoded, to_encoded, msg_encoded);

	g_free(to_encoded);
	g_free(msg_encoded);
	g_free(user_encoded);
	g_free(pass_encoded);
	g_free(number_encoded);

	http_send_curl(baseurl, sender, HTTP_POST, url, proxy);

	if(settings->extra_logging)
		g_debug("returned html is %s", sender->buffer->str);

	if(((g_strstr_len(sender->buffer->str, sender->buffer->len, "password")) != NULL))
	{
		g_debug("Message Not sent to betamax. Login error %s", baseurl);
		if(settings->extra_logging)
			g_debug("post data is %s", url);

		g_free(url);
		if(settings->provider == OTHER_BETAMAX)
			g_free(baseurl);

		g_string_free(sender->buffer, TRUE);
		return ERROR_LOGIN;
	}
	else if(((g_strstr_len(sender->buffer->str, sender->buffer->len, "success")) != NULL))
	{
		g_debug("Message sent to betamax");
		g_free(url);
		if(settings->provider == OTHER_BETAMAX)
			g_free(baseurl);
		g_string_free(sender->buffer, TRUE);
		return SUCCESS;
	}
	else
	{
		g_debug("Message Not sent betamax %s",
						baseurl);
		if(settings->extra_logging)
			g_debug("post data is %s", url);

		g_free(url);
		if(settings->provider == OTHER_BETAMAX)
			g_free(baseurl);
		g_string_free(sender->buffer, TRUE);
		return ERROR_OTHER;
	}

	return ERROR_OTHER;
}

