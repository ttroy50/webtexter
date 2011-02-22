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
	CURL *curl;
	CURLcode res;
	struct curl_slist *chunk = NULL;

	gchar *to_encoded = url_encode(to);
	gchar *msg_encoded = url_encode(message);
	gchar *user_encoded = url_encode(settings->username);
	gchar *pass_encoded = url_encode(settings->password);

	gchar *post = g_strdup_printf("<SOAP-ENV:Envelope xmlns:SOAP-ENV=\"http://schemas.xmlsoap.org/soap/envelope/\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" SOAP-ENV:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\"> <SOAP-ENV:Header/> "
				      "<SOAP-ENV:Body>  <sendSMS xmlns=\"\">  <ID xsi:type=\"xsd:string\">%s</ID><Password xsi:type=\"xsd:string\">%s</Password><type xsi:type=\"xsd:int\">1</type><To xsi:type=\"xsd:string\">%s</To><Message xsi:type=\"xsd:string\">%s</Message><Date xsi:type=\"xsd:datetime\">1970-01-01T00:00:00</Date></sendSMS> </SOAP-ENV:Body></SOAP-ENV:Envelope>",
				      user_encoded, pass_encoded,
				      to_encoded, msg_encoded);

	g_free(to_encoded);
	g_free(msg_encoded);
	g_free(user_encoded);
	g_free(pass_encoded);

	curl = curl_easy_init();
	if(curl) {
	  curl_easy_setopt(curl, CURLOPT_URL, "http://pennytel.com/pennytelapi/services/PennyTelAPI");
	  chunk = curl_slist_append(chunk, "SOAPAction: \"\"");
	  res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
	  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post);

	  /* try to use the proxy */
	  if(proxy != NULL)
	  {
		  if(proxy->use_proxy)
		  {
			  curl_easy_setopt(curl, CURLOPT_PROXY, proxy->proxy_host);
			  curl_easy_setopt(curl, CURLOPT_PROXYPORT, proxy->proxy_port);
		  }
	  }

	  /* Perform the request, res will get the return code */
	  res = curl_easy_perform(curl);

	  /* always cleanup */
	  curl_easy_cleanup(curl);
	}

	return SUCCESS;
}

