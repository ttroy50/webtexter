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

#ifndef _NETWORK_UTILITIES_H
#define _NETWORK_UTILITIES_H

#include "main.h"
#include <libosso.h>
#include <conicconnection.h>
#include <conicconnectionevent.h>

G_BEGIN_DECLS

typedef struct
{
	gint proxy_type;
	gboolean use_proxy;
	GSList *ignore_hosts;
	const gchar *proxy_host;
	gint *proxy_port;
	const gchar *proxy_config_url;
}HTTP_Proxy;

gboolean get_iap_connection(AppData *appdata);
void get_iap_details(ConIcConnection *connection, ConIcConnectionEvent *event,AppData *appdata);


G_END_DECLS

#endif
