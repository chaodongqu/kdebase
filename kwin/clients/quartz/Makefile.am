
INCLUDES = -I$(srcdir)/../../lib $(all_includes)

SUBDIRS = . config

kde_module_LTLIBRARIES = kwin3_quartz.la

kwin3_quartz_la_SOURCES = quartz.cpp
kwin3_quartz_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -module
kwin3_quartz_la_LIBADD  = $(LIB_KDEUI)  ../../lib/libkdecorations.la

METASOURCES = AUTO
noinst_HEADERS = quartz.h

lnkdir = $(kde_datadir)/kwin/
lnk_DATA = quartz.desktop

EXTRA_DIST = $(lnk_DATA)


###KMAKE-start (don't edit or delete this block)

###KMAKE-end
