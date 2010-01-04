# List extras to package
EXTRA_DIST = \
        autogen.sh \
        debian/postinst \
        debian/copyright \
        debian/control \
        debian/rules \
		debian/changelog \
		debian/postinst \
		src/*

# Sub directories
SUBDIRS = data po

# Includes
INCLUDES = $(DEPS_CFLAGS) $(HILDON_CFLAGS) $(OSSO_CFLAGS) $(OSSO_ABOOK_CFLAGS) $(CURL_CFLAGS) $(CONIC_CFLAGS) $(RTCOM_EVENTLOGGER_CFLAGS) $(UUID_CFLAGS)

# Binary
bin_PROGRAMS = \
        webtexter
# /Binary

# Sources
webtexter_SOURCES = \
	src/main.h src/main.c \
	src/http_sender.h src/http_sender.c \
	src/o2_sender.h src/o2_sender.c \
	src/settings.h src/settings.c \
	src/vodafone_sender.h src/vodafone_sender.c \
	src/three_sender.h src/three_sender.c \
	src/meteor_sender.h src/meteor_sender.c \
	src/web_proxy_sender.h src/web_proxy_sender.c \
	src/network_utilities.h src/network_utilities.c \
	src/message_logger.h src/message_logger.c \
	src/blueface_sender.h src/blueface_sender.c \
	src/betamax_sender.h src/betamax_sender.c \
	src/websmsru_sender.h src/websmsru_sender.c \
	src/exetel_sender.h src/exetel_sender.c
# /Sources

# LDADD
webtexter_LDADD = \
        $(DEPS_LIBS) $(HILDON_LIBS) $(OSSO_LIBS) $(OSSO_ABOOK_LIBS) $(CURL_LIBS) $(CONIC_LIBS) $(RTCOM_EVENTLOGGER_LIBS) $(UUID_LIBS)
# /LDADD

deb: dist
	-mkdir $(top_builddir)/debian-build
	cd $(top_builddir)/debian-build && tar zxf ../$(top_builddir)/$(PACKAGE)-$(VERSION).tar.gz
	cd $(top_builddir)/debian-build/$(PACKAGE)-$(VERSION) && dpkg-buildpackage -rfakeroot
	rm -rf $(top_builddir)/debian-build/$(PACKAGE)-$(VERSION)
