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

#ifndef _SETTINGS_H
#define _SETTINGS_H

#include <libosso.h>
#include <gconf/gconf.h>
#include <gconf/gconf-client.h>


G_BEGIN_DECLS

#define GCONF_NODE "/apps/m_webtexter"
#define GCONF_USER_KEY "/apps/m_webtexter/username"
#define GCONF_PASS_KEY "/apps/m_webtexter/password"
#define GCONF_NUMBER_KEY "/apps/m_webtexter/number"
#define GCONF_PROV_KEY "/apps/m_webtexter/provider"
#define GCONF_PROXY_KEY "/apps/m_webtexter/use_proxy"
#define GCONF_SAVEMSG_KEY "/apps/m_webtexter/savemsg"
#define GCONF_EMULATOR_KEY "/apps/m_webtexter/emulator"
#define GCONF_PROXY_URL_KEY "/apps/m_webtexter/proxy_url"
#define GCONF_CURL_TIME_KEY "/apps/m_webtexter/curl_time"


#define VODA_L "Vodafone Ireland"
#define VODA_S "v"
#define O2_L "o2 Ireland"
#define O2_S "o"
#define THREE_L "3 Ireland"
#define THREE_S "t"
#define MET_L "Meteor"
#define MET_S "m"
#define BLUEFACE_L "Blueface"
#define BLUEFACE_S "bf"
#define VOIPCHEAP_L "VoipCheap"
#define VOIPCHEAP_S "vo"
#define SMSDISC_L "smsdiscount"
#define SMSDISC_S "s"
#define LOWRV_L "Lowratevoip"
#define LOWRV_S "l"
#define OTHER_BETA_L "Other Betamax"
#define WEBSMSRU_L "WebSMS.ru"

typedef enum {
  O2 = 1,
  VODAFONE = 2,
  METEOR = 3,
  THREE = 4,
  BLUEFACE = 5,
  VOIPCHEAP = 6,
  SMSDISCOUNT = 7,
  LOWRATEVOIP = 8,
  OTHER_BETAMAX = 9,
  WEBSMSRU = 10

} Providers;

typedef struct
{
	gchar* username;
	gchar* password;
	gchar* number;
	gint provider;
	gboolean use_proxy_script;
	gboolean savemsg;
	gboolean orientation_enabled;
	gboolean emulator;
	gboolean extra_logging;
	gchar* proxy_url;
	gint curl_timeout;

} AppSettings;

void setup_gconf(GConfClient* client);
gboolean get_settings(GConfClient *client, AppSettings *settings);
gboolean set_username(GConfClient *client, const gchar* username);
gboolean set_number(GConfClient *client, const gchar* number);
gboolean set_password(GConfClient *client, const gchar* password);
gboolean set_provider(GConfClient *client, gint provider);
gboolean set_proxy(GConfClient *client, gboolean use_proxy);
gboolean set_proxy_url(GConfClient *client, const gchar* proxy_url);
gboolean set_savemsg(GConfClient *client, gboolean savemsg);
gint get_max_msg_size(AppSettings *settings);

G_END_DECLS

#endif
