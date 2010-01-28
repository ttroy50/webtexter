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

#ifndef _HTTP_SENDER_H
#define _HTTP_SENDER_H

#include "settings.h"
#include "main.h"
#include "network_utilities.h"
#include <libosso.h>

G_BEGIN_DECLS
typedef enum {
  HTTP_POST,
  HTTP_GET,
  HTTP_PUT
} http_method;

typedef enum {
  ERROR_LOGIN = -1,
  ERROR_SEND = -2,
  ERROR_OTHER = -5,
  SUCCESS = 1
} status;

typedef struct
{
	gchar* login_url;
	gchar* message_url;
	gchar* send_url;
	gint messages_left;

} provider_data;

typedef struct
{
	/*gchar* data;*/
	GString *buffer;
	gint timeout;
} http_sender;

void prepare_to_send(AppData *appdata, HTTP_Proxy *proxy);
gint send_msg(AppSettings *appsettings, char* to, char* msg, HTTP_Proxy *proxy);
void http_send_curl(const char* url, http_sender* sender, http_method method,
		gchar* post_str, HTTP_Proxy *proxy);
void testSender();
char *url_encode(char *str);




G_END_DECLS
#endif
