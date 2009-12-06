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
 Name        : network_utilities.c
 Author      : Matrim
 Version     : 0.1
 Description : Network Classes to check if network is available
 ============================================================================
 */

#include "network_utilities.h"
#include "http_sender.h"
#include "main.h"
#include <conicconnection.h>

/*
 * Request an iap connecion
 */
gboolean get_iap_connection(AppData *appdata)
{
	g_debug("checking are we connected");
    gboolean success = FALSE;
    /* Create connection object
     */
    if(appdata->connection == NULL)
    	appdata->connection = con_ic_connection_new();

    /* Connect signal to receive connection events */
    g_signal_connect(G_OBJECT(appdata->connection), "connection-event",
                     G_CALLBACK(get_iap_details), appdata);

    /* Request connection and check for the result */
    success = con_ic_connection_connect(appdata->connection, CON_IC_CONNECT_FLAG_NONE);
    if (!success) g_debug("Request for connection failed\n");

    return success;

}

/*
 * Callback from creation of iap connection. Let us know if we are connected
 * and get details of connection
 */
void get_iap_details(ConIcConnection *connection,
                                  ConIcConnectionEvent *event,
                                  AppData *appdata)
{
    ConIcConnectionStatus status = con_ic_connection_event_get_status(event);
    HTTP_Proxy proxy;



    if (status == CON_IC_STATUS_CONNECTED) {
        proxy.proxy_type = con_ic_connection_get_proxy_mode(connection);
        /* Do things based on specified proxy mode */
        switch (proxy.proxy_type) {

            case CON_IC_PROXY_MODE_NONE:
                proxy.use_proxy = FALSE;
                break;

            case CON_IC_PROXY_MODE_MANUAL:
            	proxy.use_proxy = TRUE;
            	proxy.proxy_type = CON_IC_PROXY_MODE_MANUAL;
            	proxy.proxy_host = con_ic_connection_get_proxy_host(connection, CON_IC_PROXY_PROTOCOL_HTTP);
            	proxy.proxy_port =  con_ic_connection_get_proxy_port(connection, CON_IC_PROXY_PROTOCOL_HTTP);

                proxy.ignore_hosts = con_ic_connection_get_proxy_ignore_hosts(connection);
                GSList *iter;
                for (iter = proxy.ignore_hosts; iter != NULL; iter = g_slist_next(iter))
                {
                    g_free(iter->data);
                }
                /*g_slist_free(ignore_hosts);*/
                break;

            case CON_IC_PROXY_MODE_AUTO:
            	/*
            	 * not supported by libcurl

            	proxy.proxy_config_url = con_ic_connection_get_proxy_autoconfig_url(connection);
            	*/
            	g_debug("Not connected to network cannot send\n");
            	hildon_gtk_window_set_progress_indicator(GTK_WINDOW(appdata->messageWindow), 0);
                return;
                break;
        }

        /*
         * We are connected so we can move on to sending the message
         * Some times the callback can be hit twice in one go. Because of this we have a flag that is set when the send button
         * is clicked and unset after trying to send the message.
         */
        if(appdata->msg_to_send)
        {
        	prepare_to_send(appdata, &proxy);
        }
        appdata->msg_to_send = FALSE;

        if(proxy.ignore_hosts != NULL)
        	g_slist_free(proxy.ignore_hosts);


    }
    else
    {
    	/*only in here for debugging and testing on the emulator
    	 * Should not be there when running on a device
    	 */
    	if(appdata->settings.emulator)
    	{
    		g_debug("Not connected to network but in emulator so sending\n");
    		proxy.use_proxy = FALSE;
    		if(appdata->msg_to_send)
    			prepare_to_send(appdata, &proxy);

    		appdata->msg_to_send = FALSE;
    	}
    	else
    	{
    		/*
    		 * In emulator this will be stopped in prepare_to_send
    		 */
    		g_debug("Not connected to network cannot send\n");
    		hildon_gtk_window_set_progress_indicator(GTK_WINDOW(appdata->messageWindow), 0);
    	}


    }
}
