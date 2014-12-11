INCLUDES = $(all_includes) -I$(top_srcdir)/kwin/include

kde_module_LTLIBRARIES = kwin3_blackbox_config.la

kwin3_blackbox_config_la_SOURCES = config.cpp
kwin3_blackbox_config_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -module
kwin3_blackbox_config_la_LIBADD = $(LIB_KDEUI) $(LIB_KFILE)

METASOURCES = AUTO
noinst_HEADERS = config.h

lnkdir = $(kde_datadir)/kwin/

messages:
	$(XGETTEXT) *.cpp -o $(podir)/kwin_blackbox_config.pot
