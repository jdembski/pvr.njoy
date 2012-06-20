addondir = $(prefix)/addons/pvr.njoy

addon_DATA = \
	addon.xml \
	XBMC_Njoy.pvr 

XBMC_Njoy.pvr:
	make -C ../../src

install-data-hook: XBMC_Njoy.pvr
	-cp -Rf $(srcdir)/resources $(DESTDIR)$(addondir)
	chmod -R +w $(DESTDIR)$(addondir)/resources

uninstall-hook:
	rm -Rf $(DESTDIR)$(addondir)/resources

EXTRA_DIST = \
	resources \
	addon.xml 

