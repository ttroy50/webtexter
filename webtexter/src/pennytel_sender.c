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
 Name        : pennytel_sender.c
 Author      : Jonathan Schultz
 Version     : 0.8
 Description : Send messages directly to pennytel sites
 ============================================================================
 */

#include "pennytel_sender.h"
#include "settings.h"
#include "network_utilities.h"
#include "http_sender.h"
#include "string.h"
#include <curl/curl.h>


gint pennytel_send_message(AppSettings *settings, gchar* to, gchar* message, HTTP_Proxy *proxy)
{
    http_sender *sender;
    sender = g_new0(http_sender, 1);
    sender->timeout = settings->curl_timeout;

    gchar *to_encoded;
    if (g_str_has_prefix(to, "+"))
        to_encoded = url_encode(&to[1]);
    else
        to_encoded = url_encode(to);
    gchar *msg_encoded = url_encode(message);
    gchar *user_encoded = url_encode(settings->username);
    gchar *pass_encoded = url_encode(settings->password);
    gchar *number_encoded = url_encode(settings->number);
    gchar *url = g_strdup_printf("https://www.pennytel.com.au/sms?user=%s&password=%s&sender=%s&destination=%s&text=%s",
            user_encoded, pass_encoded,
            number_encoded, to_encoded, msg_encoded);

    g_free(to_encoded);
    g_free(msg_encoded);
    g_free(user_encoded);
    g_free(pass_encoded);
    g_free(number_encoded);

    http_send_curl(url, sender, HTTP_GET, "", proxy);

    if(settings->extra_logging)
        g_debug("returned html is %s", sender->buffer->str);

    if(((g_strstr_len(sender->buffer->str, sender->buffer->len, "Queued")) != NULL))
    {
        g_debug("Message sent to Pennytel");
        g_free(url);
        g_string_free(sender->buffer, TRUE);
        return SUCCESS;
    }
    else
    {
        if(settings->extra_logging)
            g_debug("post data is %s", url);

        g_free(url);
        g_string_free(sender->buffer, TRUE);
        return ERROR_OTHER;
    }

    return ERROR_OTHER;
}

