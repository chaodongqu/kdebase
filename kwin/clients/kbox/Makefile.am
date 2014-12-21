# this has all of the subdirectories that make will recurse into.  if
# there are none, comment this out
SUBDIRS = . config styles

# this is the program that gets installed.  it's name is used for all
# of the other Makefile.am variables
kde_module_LTLIBRARIES = kwin3_blackbox.la

# set the include path for X, qt and KDE
INCLUDES = $(all_includes) -I$(srcdir)/../../lib -I$(top_srcdir)/kwin/include

# the library search path.
kwin3_blackbox_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -lkio -module

# the libraries to link against.
kwin3_blackbox_la_LIBADD = ../../lib/libkdecorations.la

# which sources should be compiled for kwin_blackbox_la
kwin3_blackbox_la_SOURCES = blackbox.cpp

# these are the headers for your project that won't be installed
noinst_HEADERS = blackbox.h

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/blackbox.pot

# this is where the desktop file will go
linkdir = $(kde_datadir)/kwin/
link_DATA  = blackbox.desktop
EXTRA_DIST = $(link_DATA)
