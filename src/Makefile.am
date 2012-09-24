## Process this file with automake to produce Makefile.in

SUBDIRS = \
	libbiji


AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-I$(top_srcdir)/libgd \
	$(BIJIBEN_CFLAGS)

AM_CFLAGS =\
	 -Wall\
	 -g

bin_PROGRAMS = bijiben

INCLUDES = -DDATADIR=\""$(datadir)"\"

bijiben_SOURCES = \
	bjb-app-menu.c \
	bjb-app-menu.h \
	bjb-bijiben.c \
	bjb-bijiben.h \
	bjb-controller.c \
	bjb-controller.h \
	bjb-debug.c \
	bjb-debug.h \
	bjb-editor-toolbar.c \
	bjb-editor-toolbar.h \
	bjb-main.c \
	bjb-main-toolbar.c \
	bjb-main-toolbar.h \
	bjb-main-view.c \
	bjb-main-view.h \
	bjb-note-view.c \
	bjb-note-view.h \
	bjb-rename-note.c \
	bjb-rename-note.h \
	bjb-search-toolbar.c \
	bjb-search-toolbar.h \
	bjb-selection-toolbar.c \
	bjb-selection-toolbar.h \
	bjb-settings.c \
	bjb-settings.h \
	bjb-share.c \
	bjb-share.h \
	bjb-tags-view.c \
	bjb-tags-view.h \
	bjb-tracker.c \
	bjb-tracker.h \
	bjb-window-base.c \
	bjb-window-base.h \
	bjb-zeitgeist-dataprovider.c \
	bjb-zeitgeist-dataprovider.h \
	utils/bjb-icons-colors.c \
	utils/bjb-icons-colors.h

resource_files = \
	app-menu.ui

bjb-resources.c: bjb.gresource.xml $(resource_files)
	glib-compile-resources --target=$@ --sourcedir=$(srcdir) --generate-source --c-name bjb $(srcdir)/bjb.gresource.xml

bjb-resources.h: bjb.gresource.xml
	glib-compile-resources --target=$@ --sourcedir=$(srcdir) --generate-header --c-name bjb $(srcdir)/bjb.gresource.xml

BUILT_SOURCES = \
	bjb-resources.c \
	bjb-resources.h

nodist_bijiben_SOURCES = \
	$(BUILT_SOURCES)

bijiben_LDFLAGS = \
	-Wl,--export-dynamic

bijiben_LDADD = libbiji.a \
	-lm \
	$(top_builddir)/libgd/libgd.la \
	$(BIJIBEN_LIBS)

lib_LIBRARIES =  \
	libbiji.a

libbiji_a_SOURCES =  \
	libbiji/libbiji.h \
	libbiji/biji-date-time.c \
	libbiji/biji-date-time.h \
	libbiji/biji-note-id.c \
	libbiji/biji-note-id.h \
	libbiji/biji-note-tag.c \
	libbiji/biji-note-tag.h \
	libbiji/biji-note-obj.c \
	libbiji/biji-note-obj.h \
	libbiji/biji-note-book.c \
	libbiji/biji-note-book.h \
	libbiji/biji-note-buffer.c \
	libbiji/biji-note-buffer.h \
	libbiji/biji-note-editor.c \
	libbiji/biji-note-editor.h \
	libbiji/biji-note-watcher.c \
	libbiji/biji-note-watcher.h\
	libbiji/biji-read-tomboy.c \
	libbiji/biji-read-tomboy.h \
	libbiji/biji-serialize.c \
	libbiji/biji-serialize.h

