#ifndef CDDB_NI_H
#define CDDB_NI_H 1

#ifdef __cplusplus
    extern "C" {
#endif


#if HAVE_CONFIG_H
#  include <config.h>
#endif

#include "cddb/debug.h"
#include "cddb/cddb_regex.h"
#include "cddb/cddb_net.h"
#include "cddb/cddb.h"
#include "cddb/cddb_conn_ni.h"
#include "cddb/cddb_cmd_ni.h"


#define FALSE          0
#define TRUE           1

#define CHR_CR         '\r'
#define CHR_LF         '\n'
#define CHR_EOS        '\0'
#define CHR_SPACE      ' '
#define CHR_DOT        '.'

#define LINE_SIZE      256

#define CLIENT_NAME    PACKAGE
#define CLIENT_VERSION VERSION

#define DEFAULT_USER        "anonymous"
#define DEFAULT_HOST        "localhost"
#define DEFAULT_SERVER      "freedb.org"
#define DEFAULT_PORT        888
#define DEFAULT_PATH_QUERY  "/~cddb/cddb.cgi"
#define DEFAULT_PATH_SUBMIT "/~cddb/submit.cgi"
#define DEFAULT_CACHE       ".cddbslave"
#define DEFAULT_PROXY_PORT  8080

#define DEFAULT_PROTOCOL_VERSION 5

#define FREE_NOT_NULL(p) if (p) free(p)
#define CONNECTION_OK(c) (c->socket != -1)
#define STR_OR_NULL(s) ((s) ? s : "NULL")


#ifdef __cplusplus
    }
#endif

#endif /* CDDB_NI_H */
