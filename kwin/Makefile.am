INCLUDES = $(all_includes) -I$(srcdir)/include -I$(srcdir)/lib
META_INCLUDES=$(srcdir)/include $(srcdir)/include/core
METASOURCES = AUTO

if include_kompmgr
KOMPMGR=kompmgr
endif

SUBDIRS = $(KOMPMGR) lib . data kcmkwin killer oldheaders pics clients

bin_PROGRAMS =
lib_LTLIBRARIES =
kdeinit_LTLIBRARIES = kwin.la

kwin_la_SOURCES = workspace.cpp client.cpp placement.cpp atoms.cpp \
	utils.cpp layers.cpp main.cpp popupinfo.cpp tabbox.cpp \
	options.cpp plugins.cpp events.cpp KWinInterface.skel \
	killwindow.cpp geometrytip.cpp sm.cpp group.cpp bridge.cpp \
	manage.cpp notifications.cpp activation.cpp useractions.cpp \
	geometry.cpp rules.cpp

kwin_la_LIBADD = $(LIB_KDEUI) lib/libkdecorations.la
kwin_la_LDFLAGS = $(all_libraries) -module -avoid-version

include_HEADERS = include/KWinInterface.h
KWinInterface_DIR = $(srcdir)/include

KDE_ICON = kwin

messages: rc.cpp
	$(XGETTEXT) -kaliasLocal *.h *.cpp killer/*.cpp lib/*.cpp -o $(podir)/kwin.pot

kwin_datadir = $(kde_datadir)/kwin

kwin_data_DATA= eventsrc

kde_kcfg_DATA = kwin.kcfg

include ../admin/Doxyfile.am
