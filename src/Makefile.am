
INCLUDES	= -I$(top_srcdir)/include
AM_CFLAGS 	= -Wall -Werror -Wchar-subscripts -Wmissing-prototypes \
	-Wmissing-declarations -Wunused -Wpointer-arith -Wwrite-strings \
	-Wnested-externs -Wno-sign-compare

noinst_PROGRAMS = example
example_SOURCES = main.c
example_LDADD = $(top_builddir)/lib/libcddb.la
