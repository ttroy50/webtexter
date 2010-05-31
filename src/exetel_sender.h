/* This file is the exetel addition for WebTexter
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

#ifndef _EXETEL_SENDER_H
#define _EXETEL_SENDER_H

#include "http_sender.h"
#include "settings.h"
#include "network_utilities.h"
#include <libosso.h>



G_BEGIN_DECLS

gchar *filter_to_exetel(gchar *str);
gint exetel_send_message(AppSettings *settings, gchar* to, gchar* message, HTTP_Proxy *proxy);


G_END_DECLS

#endif
