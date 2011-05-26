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

#ifndef _SETTINGS_H
#define _SETTINGS_H

#include <libosso.h>
#include <gconf/gconf.h>
#include <gconf/gconf-client.h>


G_BEGIN_DECLS

#define GCONF_NODE "/apps/m_webtexter"
#define GCONF_ACCOUNT_KEY "/accountname"
#define GCONF_USER_KEY "/username"
#define GCONF_PASS_KEY "/password"
#define GCONF_NUMBER_KEY "/number"
#define GCONF_PROV_KEY "/provider"
#define GCONF_PROXY_KEY "/use_proxy"
#define GCONF_SAVEMSG_KEY "/savemsg"
#define GCONF_EMULATOR_KEY "/emulator"
#define GCONF_PROXY_URL_KEY "/proxy_url"
#define GCONF_CURL_TIME_KEY "/curl_time"


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
#define EXETEL_L "Exetel"
#define PENNYTEL_L "PennyTel"

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
  WEBSMSRU = 10,
  EXETEL = 11,
  PENNYTEL = 12

} Providers;

typedef struct
{
	gchar* accountname;
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
gboolean get_settings(GConfClient *client, int account, AppSettings *settings);
gboolean set_settings(GConfClient *client, int account, AppSettings *settings);
gint get_max_msg_size(AppSettings *settings);

G_END_DECLS

#endif
