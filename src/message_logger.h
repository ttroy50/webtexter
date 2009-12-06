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

#ifndef _MESSAGE_LOGGER_H
#define _MESSAGE_LOGGER_H

#include "http_sender.h"
#include "settings.h"
#include <libosso.h>


#define SERVICE "RTCOM_EL_SERVICE_SMS"
#define EVENT_OUTBOUND "RTCOM_EL_EVENTTYPE_SMS_OUTBOUND"
#define LOCAL_UID "ring/tel/ring"
G_BEGIN_DECLS

gboolean save_message(gchar* to, gchar* msg);

G_END_DECLS

#endif
