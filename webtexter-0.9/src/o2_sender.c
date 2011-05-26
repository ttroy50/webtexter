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
 Name        : o2_sender.c
 Author      : Matrim
 Version     : 0.1
 Description : Send messages directly to o2
 ============================================================================
 */

#include "http_sender.h"
#include "o2_sender.h"
#include "settings.h"
#include "network_utilities.h"


gboolean o2_login(AppSettings *settings, HTTP_Proxy *proxy)
{
	http_sender *sender;
	sender = g_new0(http_sender, 1);
	sender->timeout = settings->curl_timeout;
	gchar *user_encoded = url_encode(settings->username);
	gchar *pass_encoded = url_encode(settings->password);
	gchar *url = g_strdup_printf("https://www.o2online.ie/amserver/UI/Login?org=o2ext&IDToken1=%s&IDToken2=%s",
			user_encoded, pass_encoded);

	g_free(user_encoded);
	g_free(pass_encoded);

	http_send_curl(url, sender, HTTP_GET, NULL, proxy);

	gchar* left = g_strstr_len(sender->buffer->str, sender->buffer->len, "Thank you for logging in");
	if((left) != NULL)
	{
		g_free(url);
		g_string_free(sender->buffer, TRUE);
		return TRUE;
	}
	else
	{
		g_debug("O2 failed to login\n");
		g_free(url);
		g_string_free(sender->buffer, TRUE);
		return FALSE;
	}

}


gboolean o2_text_page(AppSettings *settings, HTTP_Proxy *proxy)
{
	http_sender *sender;
	sender = g_new0(http_sender, 1);
	sender->timeout = settings->curl_timeout;
	http_sender *sender2;
	sender2 = g_new0(http_sender, 1);
	sender2->timeout = settings->curl_timeout;
	gchar *domain_url = "http://messaging.o2online.ie/ssomanager.osp?APIID=AUTH-WEBSSO";
	gchar *message_url = "http://messaging.o2online.ie/o2om_smscenter_new.osp?SID=73055_otfeuwos&REF=1226337573&MsgContentID=-1";

	http_send_curl(domain_url, sender, HTTP_GET, NULL, proxy);
	http_send_curl(message_url, sender2, HTTP_GET, NULL, proxy);

	if((g_strstr_len(sender2->buffer->str, sender->buffer->len, "Number of free text messages remaining this month: <strong>0")) != NULL)
	{
		g_debug("O2 no messages left\n");
		g_string_free(sender->buffer, TRUE);
		g_string_free(sender2->buffer, TRUE);
		return FALSE;
	}
	else
	{
		g_string_free(sender->buffer, TRUE);
		g_string_free(sender2->buffer, TRUE);
		return TRUE;
	}
	return TRUE;
}

gboolean o2_send_message(AppSettings *settings, gchar *to, gchar *message, HTTP_Proxy *proxy)
{
	http_sender *sender;
	sender = g_new0(http_sender, 1);
	sender->timeout = settings->curl_timeout;
	gchar *to_encoded = url_encode(to);
	gchar *msg_encoded = url_encode(message);

	gchar *url = g_strdup_printf("http://messaging.o2online.ie/smscenter_send.osp?SID=73055_otfeuwos&MsgContentID=-1&FlagDLR=1&RepeatStartDate=2008%%2C11%%2C10%%2C17%%2C15%%2C00&RepeatEndDate=2008%%2C11%%2C10%%2C17%%2C15%%2C00&RepeatType=0&RepeatEndType=0&FolderID=0&SMSToNormalized=&FID=&RURL=o2om_smscenter_new.osp%%3FSID%%3D73055_otfeuwos%%26MsgContentID%%3D-1%%26REF%%3D1226337214&REF=1226337214&SMSTo=%s&selcountry=00378&SMSText=%s&Frequency=5&StartDateDay=10&StartDateMonth=11&StartDateYear=2008&StartDateHour=17&StartDateMin=15&EndDateDay=10&EndDateMonth=11&EndDateYear=2008&EndDateHour=17&EndDateMin=15",
			to_encoded, msg_encoded);

	g_free(to_encoded);
	g_free(msg_encoded);

	http_send_curl(url, sender, HTTP_GET, NULL, proxy);

	if((g_strstr_len(sender->buffer->str, sender->buffer->len, "successfully submitted")) != NULL)
	{
		g_free(url);
		g_string_free(sender->buffer, TRUE);
		return TRUE;
	}
	else if((g_strstr_len(sender->buffer->str, sender->buffer->len, "isSuccess : true")) != NULL)
	{
		g_free(url);
		g_string_free(sender->buffer, TRUE);
		return TRUE;
	}
	else
	{
		g_debug("O2 Message Not sent\n");
		g_free(url);
		g_string_free(sender->buffer, TRUE);
		return FALSE;
	}
	return TRUE;
}
