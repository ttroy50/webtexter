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

#ifndef _BETAMAX_SENDER_H
#define _BETAMAX_SENDER_H

#include "http_sender.h"
#include "settings.h"
#include "network_utilities.h"
#include <libosso.h>



G_BEGIN_DECLS

gint betamax_send_message(AppSettings *settings, gchar* to, gchar* message, HTTP_Proxy *proxy);


G_END_DECLS

#endif
