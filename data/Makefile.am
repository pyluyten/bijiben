
SUBDIRS = icons

@INTLTOOL_DESKTOP_RULE@

desktop_in_files = bijiben.desktop.in
desktop_files    = $(desktop_in_files:.desktop.in=.desktop)

desktopdir   = $(datadir)/applications
desktop_DATA = $(desktop_files)

servicedir = $(datadir)/dbus-1/services
service_DATA = $(service_in_files:.service.in=.service)
service_in_files = 				\
	org.gnome.Bijiben.SearchProvider.service.in

org.gnome.Bijiben.SearchProvider.service: org.gnome.Bijiben.SearchProvider.service.in Makefile
	$(AM_V_GEN) sed -e "s|\@libexecdir\@|$(libexecdir)|" $< > $@

searchproviderdir = $(datadir)/gnome-shell/search-providers
searchprovider_DATA = bijiben-search-provider.ini

css_files = Default.css

cssdir = $(DESTDIR)$(pkgdatadir)
css_DATA = $(css_files)

gsettings_SCHEMAS = org.gnome.bijiben.gschema.xml
@INTLTOOL_XML_NOMERGE_RULE@
@GSETTINGS_RULES@

EXTRA_DIST =                            \
	$(desktop_in_files)     	\
	org.gnome.bijiben.gschema.xml.in \
	shell-search-provider-dbus-interfaces.xml \
	$(service_in_files)		\
	$(searchprovider_DATA)

DISTCLEANFILES =                        \
	$(desktop_files)                \
	$(gsettings_SCHEMAS)		\
	$(css_files)			\
	$(service_DATA)
	
