/*
    $Id: cddb_ni.h,v 1.13 2003/05/20 20:41:17 airborne Exp $

    Copyright (C) 2003 Kris Verbeeck <airborne@advalvas.be>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the
    Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA  02111-1307, USA.
*/

#ifndef CDDB_NI_H
#define CDDB_NI_H 1

#ifdef __cplusplus
    extern "C" {
#endif


#if HAVE_CONFIG_H
#  include <config.h>
#endif

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

#define LINE_SIZE      257

#define CLIENT_NAME    PACKAGE
#define CLIENT_VERSION VERSION

#define DEFAULT_USER        "anonymous"
#define DEFAULT_HOST        "localhost"
#define DEFAULT_SERVER      "freedb.org"
#define DEFAULT_PORT        888
#define DEFAULT_TIMEOUT     10
#define DEFAULT_PATH_QUERY  "/~cddb/cddb.cgi"
#define DEFAULT_PATH_SUBMIT "/~cddb/submit.cgi"
#define DEFAULT_CACHE       ".cddbslave"
#define DEFAULT_PROXY_PORT  8080

#define DEFAULT_PROTOCOL_VERSION 5

#define FREE_NOT_NULL(p) if (p) { free(p); p = NULL; }
#define CONNECTION_OK(c) (c->socket != -1)
#define STR_OR_NULL(s) ((s) ? s : "NULL")


#ifdef __cplusplus
    }
#endif

#endif /* CDDB_NI_H */
