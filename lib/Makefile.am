
INCLUDES	= -I$(top_srcdir)/include
AM_CFLAGS 	= -Wall -Werror -Wchar-subscripts -Wmissing-prototypes \
	-Wmissing-declarations -Wunused -Wpointer-arith -Wwrite-strings \
	-Wnested-externs -Wno-sign-compare

lib_LTLIBRARIES = libcddb.la
libcddb_la_SOURCES = cddb_track.c cddb_disc.c cddb_regex.c cddb_error.c \
					 cddb_conn.c cddb_cmd.c cddb_net.c
libcddb_la_LDFLAGS = -no-undefined -version-info 0:0:0
