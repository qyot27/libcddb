
# Only compile if the check library and header fiels are present
if REGRESSION_TESTS

# Some extra options for all test code
INCLUDES = -I$(top_srcdir)/include @CHECK_CFLAGS@
LDADD = $(top_builddir)/lib/libcddb.la @CHECK_LIBS@

# The list of available tests
TESTS = check_discid
noinst_PROGRAMS = check_discid

# Targets for individual tests
check_discid_SOURCES = base.c check_discid.c

endif
