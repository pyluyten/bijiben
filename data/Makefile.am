
SUBDIRS = icons

@INTLTOOL_DESKTOP_RULE@

desktop_in_files = bijiben.desktop.in
desktop_files    = $(desktop_in_files:.desktop.in=.desktop)

desktopdir   = $(datadir)/applications
desktop_DATA = $(desktop_files)

css_files = Default.css

cssdir = $(DESTDIR)$(pkgdatadir)
css_DATA = $(css_files)

gsettings_SCHEMAS = org.gnome.bijiben.gschema.xml
@INTLTOOL_XML_NOMERGE_RULE@
@GSETTINGS_RULES@

EXTRA_DIST =                            \
	$(desktop_in_files)     	\
	org.gnome.bijiben.gschema.xml.in

DISTCLEANFILES =                        \
	$(desktop_files)                \
	$(gsettings_SCHEMAS)		\
	$(css_files)
	
