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
 Name        : localisation.h
 Author      : Matrim
 Version     : 0.1
 Description : Header with localization facilities
 ============================================================================
 */
#ifndef LOCALISATION_H
#define LOCALISATION_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif                          /* HAVE_CONFIG_H */

#ifdef ENABLE_NLS
#include <locale.h>
#include <libintl.h>
#define _(String) gettext(String)
#ifdef gettext_noop
#define N_(String) gettext_noop(String)
#else
#define N_(String) (String)
#endif
#define locale_init() setlocale(LC_ALL, "");\
    bindtextdomain(GETTEXT_PACKAGE, localedir);\
    textdomain(GETTEXT_PACKAGE);
#else                           /* NLS is disabled */
#define locale_init()
#define _(String) (String)
#define N_(String) (String)
#define textdomain(String) (String)
#define gettext(String) (String)
#define dgettext(Domain,String) (String)
#define dcgettext(Domain,String,Type) (String)
#define bindtextdomain(Domain,Directory) (Domain)
#define bind_textdomain_codeset(Domain,Codeset) (Codeset)
#endif                          /* ENABLE_NLS */

#endif /* LOCALISATION_H */
#ifndef LOCALISATION_H
#define LOCALISATION_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif                          /* HAVE_CONFIG_H */

#ifdef ENABLE_NLS
#include <libintl.h>
#define _(String) gettext(String)
#ifdef gettext_noop
#define N_(String) gettext_noop(String)
#else
#define N_(String) (String)
#endif
#define locale_init() setlocale(LC_ALL, "");\
    bindtextdomain(GETTEXT_PACKAGE, localedir);\
    textdomain(GETTEXT_PACKAGE);
#else                           /* NLS is disabled */
#define locale_init()
#define _(String) (String)
#define N_(String) (String)
#define textdomain(String) (String)
#define gettext(String) (String)
#define dgettext(Domain,String) (String)
#define dcgettext(Domain,String,Type) (String)
#define bindtextdomain(Domain,Directory) (Domain)
#define bind_textdomain_codeset(Domain,Codeset) (Codeset)
#endif                          /* ENABLE_NLS */

#endif /* LOCALISATION_H */

