
SUBDIRS	= include lib tests examples
EXTRA_DIST = doc/doxygen.conf

dist-hook:
	mkdir $(distdir)/misc
	cp $(top_srcdir)/misc/$(PACKAGE).ebuild $(distdir)/misc/$(PACKAGE)-$(VERSION).ebuild
