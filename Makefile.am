ACLOCAL_AMFLAGS = -I m4

SUBDIRS = src data po help

bijibendocdir = ${prefix}/doc/bijiben
bijibendoc_DATA = \
	README\
	COPYING\
	AUTHORS\
	ChangeLog\
	INSTALL\
	NEWS

DISTCHECK_CONFIGURE_FLAGS = --disable-scrollkeeper

INTLTOOL_FILES = intltool-extract.in \
	intltool-merge.in \
	intltool-update.in

EXTRA_DIST = $(bijibendoc_DATA) \
	$(INTLTOOL_FILES) \
	gnome-doc-utils.make

DISTCLEANFILES = intltool-extract \
	intltool-merge \
	intltool-update \
	po/.intltool-merge-cache 


# Remove doc directory on uninstall
uninstall-local:
	-rm -r $(bijibendocdir)
