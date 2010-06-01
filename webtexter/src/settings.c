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
 Name        : settings.c
 Author      : Matrim
 Version     : 0.1
 Description : Get and set settings to gconf
 ============================================================================
 */

#include "settings.h"
#include <gconf/gconf.h>
#include <gconf/gconf-client.h>

void setup_gconf(GConfClient *client)
{
	/* Get the default client */
	client = gconf_client_get_default();

	/*Add GConf node if absent*/
	/*if(!gconf_client_dir_exists(client, GCONF_NODE, NULL))
	{*/
		gconf_client_add_dir (client, "/apps/m_webtexter",
	                GCONF_CLIENT_PRELOAD_NONE, NULL);

}

gboolean get_settings(GConfClient *client, AppSettings *settings)
{
	settings->username = NULL;
	settings->password =  NULL;
	settings->number = NULL;
	settings->provider = 0;
	settings->use_proxy_script = FALSE;
	settings->savemsg = FALSE;
	settings->orientation_enabled = FALSE;
	settings->emulator = FALSE;
	settings->proxy_url = NULL;
	settings->extra_logging = FALSE;

	settings->username = gconf_client_get_string(client,GCONF_USER_KEY, NULL);
	settings->password = gconf_client_get_string(client,GCONF_PASS_KEY, NULL);
	settings->number = gconf_client_get_string(client,GCONF_NUMBER_KEY, NULL);
	settings->provider = gconf_client_get_int(client,GCONF_PROV_KEY, NULL);
	settings->use_proxy_script = gconf_client_get_bool(client,GCONF_PROXY_KEY, NULL);
	settings->savemsg = gconf_client_get_bool(client,GCONF_SAVEMSG_KEY, NULL);
	settings->emulator = gconf_client_get_bool(client,GCONF_EMULATOR_KEY, NULL);
	settings->proxy_url = gconf_client_get_string(client,GCONF_PROXY_URL_KEY, NULL);
	settings->curl_timeout = gconf_client_get_int(client,GCONF_CURL_TIME_KEY, NULL);


	if(settings->curl_timeout <= 0)
	{
		settings->curl_timeout = 30;
	}

	if(settings->proxy_url == NULL)
	{
		settings->proxy_url = g_strdup("http://www.cabbagetexter.com/send.php");
	}
	if(settings->provider == 0)
	{
		/*
		 * If this is 0, assume this is the first run of the application, so set defaults
		 */
		settings->use_proxy_script = TRUE;
		settings->savemsg = FALSE;
		settings->emulator = FALSE;
		settings->extra_logging = FALSE;

		return FALSE;
	}
	/*Check if any of the settngs are not set, or empty. If they are we should run the wizard*/
	if(settings->username == NULL)
	{
		return FALSE;
	}
	else if(settings->password == NULL)
	{
		return FALSE;
	}
	else if(settings->number == NULL)
	{
		return FALSE;
	}




	return TRUE;
}

gint get_max_msg_size(AppSettings *settings)
{
	if(settings->use_proxy_script && (settings->provider != BLUEFACE && settings->provider != OTHER_BETAMAX
			&& settings->provider != WEBSMSRU))
	{
		return 800;
	}
	else if(settings->provider == BLUEFACE)
	{
		return 160;
	}
	else if(settings->provider == O2)
	{
		return 1000;
	}
	else if(settings->provider == METEOR)
	{
		return 480;
	}
	else if(settings->provider == EXETEL)
	{
		return 612;
	}
	else
	{
		return 160;
	}
}

gboolean set_username(GConfClient *client, const gchar* username)
{
	return gconf_client_set_string (client,
			GCONF_USER_KEY,
            username,
            NULL);
}

gboolean set_password(GConfClient *client, const gchar* password)
{
	return gconf_client_set_string (client,
			GCONF_PASS_KEY,
           	password,
            NULL);
}

gboolean set_number(GConfClient *client, const gchar* number)
{
	return gconf_client_set_string (client,
			GCONF_NUMBER_KEY,
           	number,
            NULL);
}

gboolean set_provider(GConfClient *client, gint provider)
{
	return gconf_client_set_int (client,
			GCONF_PROV_KEY,
           	provider,
            NULL);
}

gboolean set_proxy(GConfClient *client, gboolean use_proxy)
{
	return gconf_client_set_bool (client,
			GCONF_PROXY_KEY,
           	use_proxy,
            NULL);
}

gboolean set_proxy_url(GConfClient *client, const gchar* proxy_url)
{
	return gconf_client_set_string (client,
			GCONF_PROXY_URL_KEY,
            proxy_url,
            NULL);
}

gboolean set_savemsg(GConfClient *client, gboolean savemsg)
{
	return gconf_client_set_bool (client,
			GCONF_SAVEMSG_KEY,
           	savemsg,
            NULL);
}
