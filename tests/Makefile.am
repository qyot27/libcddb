
# The list of available tests
check_SCRIPTS = check_discid.sh check_parse.sh
check_DATA = 

EXTRA_DIST = $(check_SCRIPTS) $(check_DATA) settings.sh.in

dist-hook:
	mkdir $(distdir)/testcache
	mkdir $(distdir)/testcache/misc
	cp $(srcdir)/testcache/misc/1234* $(distdir)/testcache/misc/
	mkdir $(distdir)/testdata
	cp $(srcdir)/testdata/1234* $(distdir)/testdata/

TESTS = $(check_SCRIPTS)
