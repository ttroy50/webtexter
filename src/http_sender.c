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
 Name        : http_sender.c
 Author      : Matrim
 Version     : 0.1
 Description : HTTP functions to send messages
 ============================================================================
 */

#include "http_sender.h"
#include "main.h"
#include "o2_sender.h"
#include "vodafone_sender.h"
#include "meteor_sender.h"
#include "three_sender.h"
#include "blueface_sender.h"
#include "web_proxy_sender.h"
#include "betamax_sender.h"
#include "settings.h"
#include "network_utilities.h"
#include "message_logger.h"
#include <curl/curl.h>
#include <curl/easy.h>
#include <string.h>
#include <gtk/gtk.h>

void prepare_to_send(AppData *appdata, HTTP_Proxy *proxy)
{
	GtkWidget *banner;
	GtkTextIter tostart;
	GtkTextIter toend;

	gtk_text_buffer_get_start_iter (GTK_TEXT_BUFFER(appdata->toBuffer), &tostart);
	gtk_text_buffer_get_end_iter (GTK_TEXT_BUFFER(appdata->toBuffer), &toend);

	GtkTextIter msgstart;
	GtkTextIter msgend;
	gtk_text_buffer_get_start_iter (GTK_TEXT_BUFFER(appdata->msgBuffer), &msgstart);
	gtk_text_buffer_get_end_iter (GTK_TEXT_BUFFER(appdata->msgBuffer), &msgend);

	gchar* to;
	to = gtk_text_buffer_get_text(GTK_TEXT_BUFFER(appdata->toBuffer), &tostart,
											&toend, FALSE);
	gchar* msg;
	msg = gtk_text_buffer_get_text(GTK_TEXT_BUFFER(appdata->msgBuffer), &msgstart,
											&msgend, FALSE);

	gint sent = send_msg(&appdata->settings, to, msg, proxy);

	if(sent == SUCCESS)
	{
		/*
		 * Saving messages is still experimental so only available in debug mode
		 */
		if(appdata->settings.savemsg)
			save_message(to, msg);

		banner = hildon_banner_show_information(GTK_WIDGET(appdata->messageWindow), NULL,
						"Message Sent");
		gtk_text_buffer_set_text(GTK_TEXT_BUFFER(appdata->toBuffer), "", -1);
		gtk_text_buffer_set_text(GTK_TEXT_BUFFER(appdata->msgBuffer), "", -1);
	}
	else if(sent == ERROR_LOGIN)
	{
		banner = hildon_banner_show_information(GTK_WIDGET(appdata->messageWindow), NULL,
						"Unable to Login to provider");
	}
	else if(sent == ERROR_SEND)
	{
		banner = hildon_banner_show_information(GTK_WIDGET(appdata->messageWindow), NULL,
			"Unable to send message");
	}
	else
	{
		banner = hildon_banner_show_information(GTK_WIDGET(appdata->messageWindow), NULL,
								"Unspecified error sending message");
	}

	g_debug("hiding progress indicator\n");
	hildon_gtk_window_set_progress_indicator(GTK_WINDOW(appdata->messageWindow), 0);
	g_free(to);
	g_free(msg);

}

gint send_msg(AppSettings *settings, char* to, char* msg, HTTP_Proxy *proxy)
{

	if(settings->use_proxy_script && settings->provider != BLUEFACE)
	{
		return web_proxy_send_message(settings, to, msg, proxy);
	}

	if(settings->provider == VOIPCHEAP || settings->provider == SMSDISCOUNT || settings->provider == LOWRATEVOIP)
	{
		return betamax_send_message(settings, to, msg, proxy);
	}
	else if(settings->provider == O2)
	{
		if(o2_login(settings, proxy))
		{
			if(o2_text_page(settings, proxy))
			{
				if(o2_send_message(settings, to, msg, proxy))
				{
					return SUCCESS;
				}
				else
					return ERROR_SEND;

			}
			else
				return ERROR_OTHER;
		}
		else
		{
			g_debug("Unable to Logon to O2");
			return ERROR_LOGIN;
		}



	}
	else if(settings->provider == VODAFONE)
	{
		if(vodafone_login(settings, proxy))
		{
			gchar* apache_token = vodafone_text_page(settings, proxy);
			if(apache_token != NULL)
			{
				/*
				 * Need to add a sleep here or vodafone will reject the message
				 */
				sleep(2);
				if(vodafone_send_message(settings, apache_token, to, msg, proxy))
				{
					if(apache_token != NULL)
						g_free(apache_token);

					return SUCCESS;
				}
				else
				{
					if(apache_token != NULL)
						g_free(apache_token);

					return ERROR_SEND;
				}
			}
			else
			{
				return ERROR_OTHER;
			}

		}
		else
		{
			g_debug("Unable to Logon to Vodafone");
			return ERROR_LOGIN;
		}
	}
	else if(settings->provider == METEOR)
	{
		if(meteor_login(settings))
		{
			if(meteor_text_page(settings))
			{
				if(meteor_send_message(settings, to, msg))
				{
					return SUCCESS;
				}
				else
				{
					return ERROR_SEND;
				}
			}
			else
			{
				return ERROR_OTHER;
			}
		}
		else
		{
			g_debug("meteor logon error\n");
			return ERROR_LOGIN;
		}
	}
	else if(settings->provider == THREE)
	{
		if(three_login(settings))
		{
			if(three_text_page(settings))
			{
				if(three_send_message(settings, to, msg))
				{
					return SUCCESS;
				}
				else
				{
					return ERROR_SEND;
				}
			}
			else
			{
				return ERROR_OTHER;
			}
		}
		else
		{
			g_debug("Unable to send message to THREE");
			return ERROR_LOGIN;
		}
	}
	else if(settings->provider == BLUEFACE)
	{
		return blueface_send_message(settings, to, msg, proxy);
	}
	else
	{
		/*
		 * invalid provider
		 */
		g_debug("invalid provider\n");
		return ERROR_OTHER;
	}
}


size_t write_data(void *dataptr, size_t size, size_t nmemb, void* data)
{
	g_string_append_len (((http_sender *) data)->buffer, (const gchar *)dataptr, size*nmemb);
	return size*nmemb;
}

void http_send_curl(const char* url, http_sender* sender, http_method method,
		gchar* post_str, HTTP_Proxy *proxy)
{
	CURL *handle;
	CURLcode res;

	sender->buffer = g_string_new ("");
	handle = curl_easy_init();

	char* cookiefile = "/opt/maemo/usr/share/webtexter/cookies/cookie";
	char* cookiejar = "/opt/maemo/usr/share/webtexter/cookies/cookie";
	char* referer = "http://www.google.ie";

	gint url_length = strlen(url);

	if(handle)
	{
		curl_easy_setopt(handle, CURLOPT_USERAGENT,
				"Mozilla/5.0 (Windows; U; Windows NT 5.1; en-US; rv:1.8.0.6) Gecko/20060728 Firefox/1.5.0.6");
		curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, 0);
		curl_easy_setopt(handle, CURLOPT_SSL_VERIFYHOST, 0);
		curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, write_data);
		curl_easy_setopt(handle, CURLOPT_WRITEDATA, (void *)sender);
		curl_easy_setopt(handle, CURLOPT_URL, url);
		curl_easy_setopt(handle, CURLOPT_HEADER, 0);
		curl_easy_setopt(handle, CURLOPT_COOKIEFILE, cookiefile);
		/*curl_easy_setopt(handle, CURLOPT_VERBOSE, 1);*/
		curl_easy_setopt(handle, CURLOPT_COOKIEJAR, cookiejar);
		curl_easy_setopt(handle, CURLOPT_TIMEOUT, 15);

		if(post_str != NULL && method == HTTP_POST)
		{
			curl_easy_setopt(handle, CURLOPT_POST, 1);
			curl_easy_setopt(handle, CURLOPT_POSTFIELDS, post_str);
		}

		if(proxy != NULL)
		{
			if(proxy->use_proxy)
			{
				curl_easy_setopt(handle, CURLOPT_PROXY, proxy->proxy_host);
				curl_easy_setopt(handle, CURLOPT_PROXYPORT, proxy->proxy_port);

			}
		}

		curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 1);
		curl_easy_setopt(handle, CURLOPT_REFERER, referer);
		res = curl_easy_perform(handle);
		curl_easy_cleanup(handle);
	}
	else
	{
		g_debug("CURL Handle not allocated\n");
	}
}


void testSender()
{
	g_debug("test HTTP Send\n");
	CURL *handle;
	CURLcode res;

	handle = curl_easy_init();

	if(handle)
	{
		char *url = "http://setkia.com/";
		curl_easy_setopt(handle, CURLOPT_URL, url);

		res = curl_easy_perform(handle);
		curl_easy_cleanup(handle);
	}
}

/* Converts a hex character to its integer value */
char from_hex(char ch) {
  return isdigit(ch) ? ch - '0' : tolower(ch) - 'a' + 10;
}

/* Converts an integer value to its hex character*/
char to_hex(char code) {
  static char hex[] = "0123456789abcdef";
  return hex[code & 15];
}

/* Returns a url-encoded version of str */
/* IMPORTANT: be sure to free() the returned string after use */
char *url_encode(char *str) {
  char *pstr = str, *buf = malloc(strlen(str) * 3 + 1), *pbuf = buf;
  while (*pstr) {
    if (isalnum(*pstr) || *pstr == '-' || *pstr == '_' || *pstr == '.' || *pstr == '~')
      *pbuf++ = *pstr;
    else if (*pstr == ' ')
      *pbuf++ = '+';
    else
      *pbuf++ = '%', *pbuf++ = to_hex(*pstr >> 4), *pbuf++ = to_hex(*pstr & 15);
    pstr++;
  }
  *pbuf = '\0';
  return buf;
}

/* Returns a url-decoded version of str */
/* IMPORTANT: be sure to free() the returned string after use */
char *url_decode(char *str) {
  char *pstr = str, *buf = malloc(strlen(str) + 1), *pbuf = buf;
  while (*pstr) {
    if (*pstr == '%') {
      if (pstr[1] && pstr[2]) {
        *pbuf++ = from_hex(pstr[1]) << 4 | from_hex(pstr[2]);
        pstr += 2;
      }
    } else if (*pstr == '+') {
      *pbuf++ = ' ';
    } else {
      *pbuf++ = *pstr;
    }
    pstr++;
  }
  *pbuf = '\0';
  return buf;
}

