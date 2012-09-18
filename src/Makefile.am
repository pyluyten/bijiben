## Process this file with automake to produce Makefile.in

SUBDIRS = \
	libbiji


AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	$(BIJIBEN_CFLAGS)

AM_CFLAGS =\
	 -Wall\
	 -g

bin_PROGRAMS = bijiben

INCLUDES = -DDATADIR=\""$(datadir)"\"

bijiben_SOURCES = \
	bjb-app-menu.c \
	bjb-app-menu.h \
	bjb-bijiben.h \
	bjb-bijiben.c \
	bjb-controller.c \
	bjb-controller.h \
	bjb-debug.h \
	bjb-debug.c \
	bjb-editor-toolbar.c \
	bjb-editor-toolbar.h \
	bjb-main.c \
	bjb-main-toolbar.c \
	bjb-main-toolbar.h \
	bjb-main-view.h \
	bjb-main-view.c \
	bjb-note-view.h \
	bjb-note-view.c \
	bjb-rename-note.h \
	bjb-rename-note.c \
	bjb-search-toolbar.c \
	bjb-search-toolbar.h \
	bjb-selection-toolbar.c \
	bjb-selection-toolbar.h \
	bjb-settings.h \
	bjb-settings.c \
	bjb-share.c \
	bjb-share.h \
	bjb-tags-view.h \
	bjb-tags-view.c \
	bjb-tracker.c \
	bjb-tracker.h \
	bjb-window-base.c \
	bjb-window-base.h \
	bjb-zeitgeist-dataprovider.h \
	bjb-zeitgeist-dataprovider.c \
	widgets/gd-main-icon-view.c \
	widgets/gd-main-icon-view.h \
	widgets/gd-main-toolbar.c \
	widgets/gd-main-toolbar.h \
	widgets/gd-main-list-view.c \
	widgets/gd-main-list-view.h \
	widgets/gd-main-view.c \
	widgets/gd-main-view.h \
	widgets/gd-main-view-generic.c \
	widgets/gd-main-view-generic.h \
	widgets/gd-toggle-pixbuf-renderer.c \
	widgets/gd-toggle-pixbuf-renderer.h \
	widgets/gd-two-lines-renderer.c \
	widgets/gd-two-lines-renderer.h \
	utils/bjb-icons-colors.c \
	utils/bjb-icons-colors.h 

bijiben_LDFLAGS = \
	-Wl,--export-dynamic

bijiben_LDADD = libbiji.a \
  -lm \
	$(BIJIBEN_LIBS)

lib_LIBRARIES =  \
	libbiji.a

libbiji_a_SOURCES =  \
	libbiji/libbiji.h \
	libbiji/libbiji.c \
	libbiji/biji-date-time.c \
	libbiji/biji-date-time.h \
	libbiji/biji-note-id.c \
	libbiji/biji-note-id.h \
	libbiji/biji-note-tag.h \
	libbiji/biji-note-tag.c \
	libbiji/biji-note-obj.c \
	libbiji/biji-note-obj.h \
	libbiji/biji-note-book.c \
	libbiji/biji-note-book.h \
	libbiji/biji-note-buffer.h \
	libbiji/biji-note-buffer.c \
	libbiji/biji-note-editor.c \
	libbiji/biji-note-editor.h \
	libbiji/biji-note-watcher.h \
	libbiji/biji-note-watcher.c \
	libbiji/biji-read-tomboy.h \
	libbiji/biji-read-tomboy.c \
	libbiji/biji-serialize.c \
	libbiji/biji-serialize.h
