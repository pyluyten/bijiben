SUBDIRS = \
	libbiji

bin_PROGRAMS = bijiben

libexec_PROGRAMS=				\
	bijiben-shell-search-provider		\
	$(NULL)

AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-I$(top_srcdir)/libgd \
	$(BIJIBEN_CFLAGS)

AM_CFLAGS =\
	 -Wall\
	 -g

INCLUDES = -DDATADIR=\""$(datadir)"\"

LDADD = libbiji.a \
	-lm \
	$(top_builddir)/libgd/libgd.la \
	$(BIJIBEN_LIBS)

bijiben_LDFLAGS = \
	-Wl,--export-dynamic

resource_files = \
	app-menu.ui

bjb-resources.c: bjb.gresource.xml $(resource_files)
	glib-compile-resources --target=$@ --sourcedir=$(srcdir) --generate-source --c-name bjb $(srcdir)/bjb.gresource.xml

bjb-resources.h: bjb.gresource.xml
	glib-compile-resources --target=$@ --sourcedir=$(srcdir) --generate-header --c-name bjb $(srcdir)/bjb.gresource.xml

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
	bjb-note-tag-dialog.c \
	bjb-note-tag-dialog.h \
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
	bjb-window-base.c \
	bjb-window-base.h \
	utils/bjb-icons-colors.c \
	utils/bjb-icons-colors.h

dbus_shell_search_provider_built_sources =		\
	bijiben-shell-search-provider-generated.c	\
	bijiben-shell-search-provider-generated.h

$(dbus_shell_search_provider_built_sources) : Makefile.am $(top_srcdir)/data/shell-search-provider-dbus-interfaces.xml
	gdbus-codegen									\
		--interface-prefix org.gnome.						\
		--c-namespace Bijiben							\
		--generate-c-code bijiben-shell-search-provider-generated		\
		$(top_srcdir)/data/shell-search-provider-dbus-interfaces.xml		\
		$(NULL)

bijiben_shell_search_provider_SOURCES = 	\
	bijiben-shell-search-provider.c		\
	$(NULL)

nodist_bijiben_shell_search_provider_SOURCES =		\
	$(dbus_shell_search_provider_built_sources)	\
	$(NULL)

bijiben_built_sources = \
	bjb-resources.c \
	bjb-resources.h

BUILT_SOURCES = \
	$(bijiben_built_sources) \
	$(dbus_shell_search_provider_built_sources) 	\
	$(NULL)

lib_LIBRARIES =  \
	libbiji.a

libbiji_a_SOURCES =  \
	libbiji/libbiji.h \
	libbiji/biji-date-time.c \
	libbiji/biji-date-time.h \
	libbiji/biji-note-id.c \
	libbiji/biji-note-id.h \
	libbiji/biji-note-obj.c \
	libbiji/biji-note-obj.h \
	libbiji/biji-note-book.c \
	libbiji/biji-note-book.h \
	libbiji/biji-string.c \
	libbiji/biji-string.h \
	libbiji/biji-timeout.c \
	libbiji/biji-timeout.h \
	libbiji/biji-tracker.c \
	libbiji/biji-tracker.h \
	libbiji/biji-zeitgeist.c \
	libbiji/biji-zeitgeist.h \
	libbiji/deserializer/biji-lazy-deserializer.c \
	libbiji/deserializer/biji-lazy-deserializer.h \
	libbiji/editor/biji-editor-selection.c \
	libbiji/editor/biji-editor-selection.h \
	libbiji/editor/biji-editor-utils.c \
	libbiji/editor/biji-editor-utils.h \
	libbiji/editor/biji-webkit-editor.c \
	libbiji/editor/biji-webkit-editor.h \
	libbiji/serializer/biji-lazy-serializer.c \
	libbiji/serializer/biji-lazy-serializer.h

CLEANFILES = \
	$(BUILT_SOURCES)			\
	$(NULL)

dist-hook:
	cd $(distdir); rm -f $(CLEANFILES)

