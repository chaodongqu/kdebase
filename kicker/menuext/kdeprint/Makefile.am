INCLUDES = $(all_includes)

kde_module_LTLIBRARIES = kickermenu_kdeprint.la

kickermenu_kdeprint_la_SOURCES = print_mnu.cpp
kickermenu_kdeprint_la_LDFLAGS = $(all_libraries) -module -avoid-version
kickermenu_kdeprint_la_LIBADD = $(LIB_KDEUI) $(LIB_KIO) -lkdeprint

kickermenu_kdeprint_la_METASOURCES = AUTO

desktopmenu_DATA = printmenu.desktop
desktopmenudir = $(kde_datadir)/kicker/menuext

messages:
	$(XGETTEXT) *.cpp -o $(podir)/libkickermenu_kdeprint.pot
