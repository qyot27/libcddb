/*
    $Id: cddb_conn.c,v 1.31 2005/03/11 21:17:00 airborne Exp $

    Copyright (C) 2003, 2004, 2005 Kris Verbeeck <airborne@advalvas.be>

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

#include "cddb/cddb_ni.h"

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif

#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif 

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif


/* --- prototypes --- */


/**
 * Send handshake to CDDB server.
 */
int cddb_handshake(cddb_conn_t *c);


/* --- construction / destruction --- */


cddb_conn_t *cddb_new(void)
{
    cddb_conn_t *c;
    const char *s;

    libcddb_init();             /* initialize globals if not yet done */
    c = (cddb_conn_t*)malloc(sizeof(cddb_conn_t));
    if (c) {
        c->buf_size = DEFAULT_BUF_SIZE;
        c->line = (char*)malloc(c->buf_size);

        c->cname = strdup(CLIENT_NAME);
        c->cversion = strdup(CLIENT_VERSION);

        c->is_connected = FALSE;
        c->socket = -1;
        c->cache_fp = NULL;
        c->server_name = strdup(DEFAULT_SERVER);
        c->server_port = DEFAULT_PORT;
        c->timeout = DEFAULT_TIMEOUT;

        c->http_path_query = strdup(DEFAULT_PATH_QUERY);
        c->http_path_submit = strdup(DEFAULT_PATH_SUBMIT);

        c->is_http_enabled = FALSE;
        c->is_http_proxy_enabled = FALSE;
        c->http_proxy_server = NULL;
        c->http_proxy_server_port = DEFAULT_PROXY_PORT;

        c->use_cache = CACHE_ON;
        /* construct cache dir '$HOME/[DEFAULT_CACHE]' */
        s = getenv("HOME");
        c->cache_dir = (char*)malloc(strlen(s) + 1 + sizeof(DEFAULT_CACHE) + 1);
        sprintf(c->cache_dir, "%s/%s", s, DEFAULT_CACHE);
        c->cache_read = FALSE;

        /* use anonymous@localhost */
        c->user = strdup(DEFAULT_USER);
        c->hostname = strdup(DEFAULT_HOST);

        c->errnum = CDDB_ERR_OK;

        c->query_data = NULL;
        c->query_idx = 0;
        c->query_cnt = 0;

        c->charset = malloc(sizeof(struct cddb_iconv_s));
        c->charset->cd_to_freedb = NULL;
        c->charset->cd_from_freedb = NULL;
    } else {
        cddb_log_crit(cddb_error_str(CDDB_ERR_OUT_OF_MEMORY));
    }

    return c;
}

static void cddb_close_iconv(cddb_conn_t *c)
{
    if (c->charset) {
#ifdef HAVE_ICONV_H
        if (c->charset->cd_to_freedb) {
            iconv_close(c->charset->cd_to_freedb);
        }
        if (c->charset->cd_from_freedb) {
            iconv_close(c->charset->cd_from_freedb);
        }
#endif /* HAVE_ICONV_H */
    }
}

void cddb_destroy(cddb_conn_t *c)
{
    if (c) {
        cddb_disconnect(c);
        FREE_NOT_NULL(c->line);
        FREE_NOT_NULL(c->cname);
        FREE_NOT_NULL(c->cversion);
        FREE_NOT_NULL(c->server_name);
        FREE_NOT_NULL(c->http_path_query);
        FREE_NOT_NULL(c->http_path_submit);
        FREE_NOT_NULL(c->cache_dir);
        FREE_NOT_NULL(c->user);
        FREE_NOT_NULL(c->hostname);
        cddb_query_clear(c);
        cddb_close_iconv(c);
        FREE_NOT_NULL(c->charset);
        free(c);
    }
}


/* --- getters & setters --- */


int cddb_set_charset(cddb_conn_t *c, const char *charset)
{
#ifdef HAVE_ICONV_H
    cddb_close_iconv(c);
    c->charset->cd_to_freedb = iconv_open(SERVER_CHARSET, charset);
    if (c->charset->cd_to_freedb == (iconv_t)-1) {
        c->charset->cd_to_freedb = NULL;
        cddb_errno_set(c, CDDB_ERR_INVALID_CHARSET);
        return FALSE;
    }
    c->charset->cd_from_freedb = iconv_open(charset, SERVER_CHARSET);
    if (c->charset->cd_from_freedb == (iconv_t)-1) {
        iconv_close(c->charset->cd_to_freedb);
        c->charset->cd_to_freedb = NULL;
        c->charset->cd_from_freedb = NULL;
        cddb_errno_set(c, CDDB_ERR_INVALID_CHARSET);
        return FALSE;
    }
    cddb_errno_set(c, CDDB_ERR_OK);
    return TRUE;
#else
    cddb_errno_set(c, CDDB_ERR_ICONV_FAIL);
    return FALSE;
#endif /* HAVE_ICONV_H */
}

void cddb_set_buf_size(cddb_conn_t *c, unsigned int size)
{
    FREE_NOT_NULL(c->line);
    c->buf_size = size;
    c->line = (char*)malloc(c->buf_size);
}

void cddb_set_server_name(cddb_conn_t *c, const char *server)
{
    FREE_NOT_NULL(c->server_name);
    c->server_name = strdup(server);
}

void cddb_set_server_port(cddb_conn_t *c, int port)
{
    c->server_port = port;
}

void cddb_set_http_path_query(cddb_conn_t *c, const char *path)
{
    FREE_NOT_NULL(c->http_path_query);
    c->http_path_query = strdup(path);
}

void cddb_set_http_path_submit(cddb_conn_t *c, const char *path)
{
    FREE_NOT_NULL(c->http_path_submit);
    c->http_path_submit = strdup(path);
}

void cddb_http_enable(cddb_conn_t *c)
{
    c->is_http_enabled = TRUE;
    cddb_errno_set(c, CDDB_ERR_OK);
}

void cddb_http_disable(cddb_conn_t *c)
{
    c->is_http_enabled = FALSE;
    cddb_errno_set(c, CDDB_ERR_OK);
}

void cddb_http_proxy_enable(cddb_conn_t *c)
{
    /* enabling HTTP proxy implies HTTP, but not vice versa */
    cddb_http_enable(c);
    c->is_http_proxy_enabled = TRUE;
    cddb_errno_set(c, CDDB_ERR_OK);
}

void cddb_http_proxy_disable(cddb_conn_t *c)
{
    c->is_http_proxy_enabled = FALSE;
    cddb_errno_set(c, CDDB_ERR_OK);
}

void cddb_set_http_proxy_server_name(cddb_conn_t *c, const char *server)
{
    FREE_NOT_NULL(c->http_proxy_server);
    c->http_proxy_server = strdup(server);
}

void cddb_set_http_proxy_server_port(cddb_conn_t *c, int port)
{
    c->http_proxy_server_port = port;
}

void cddb_set_client(cddb_conn_t *c, const char *cname, const char *cversion)
{
    if (cname && cversion) {
        FREE_NOT_NULL(c->cname);
        FREE_NOT_NULL(c->cversion);
        c->cname = strdup(cname);
        c->cversion = strdup(cversion);
    }
}

int cddb_set_email_address(cddb_conn_t *c, const char *email)
{
    char *at;
    int len;

    cddb_log_debug("cddb_set_email_address()");
    if ((email == NULL) ||
        ((at = strchr(email, '@')) == NULL) ||
        (at == email) || 
        (*(at+1) == '\0')) {
        cddb_errno_log_error(c, CDDB_ERR_EMAIL_INVALID);
        return FALSE;
    }
    /* extract user name */
    FREE_NOT_NULL(c->user);
    len = at - email;
    c->user = malloc(len + 1);
    strncpy(c->user, email, len);
    c->user[len] = '\0';
    /* extract host name */
    at++;
    FREE_NOT_NULL(c->hostname);
    c->hostname = strdup(at);
    cddb_log_debug("...user name = '%s'", c->user);
    cddb_log_debug("...host name = '%s'", c->hostname);

    return TRUE;
}

int cddb_cache_set_dir(cddb_conn_t *c, const char *dir)
{
    char *home;

    cddb_log_debug("cddb_cache_set_dir()");
    if (dir) {
        FREE_NOT_NULL(c->cache_dir);
        if (dir[0] == '~') {
            /* expand ~ to $HOME */
            home = getenv("HOME");
            if (home) {
                c->cache_dir = (char*)malloc(strlen(home) + strlen(dir));
                sprintf(c->cache_dir, "%s%s", home, dir + 1);
            }
        } else {
            c->cache_dir = strdup(dir);
        }
    }
    return TRUE;
}


/* --- connecting / disconnecting --- */


int cddb_handshake(cddb_conn_t *c)
{
    char *msg;
    int code;

    cddb_log_debug("cddb_handshake()");
    /* check sign-on banner */
    switch (code = cddb_get_response_code(c, &msg)) {
        case  -1:
            return FALSE;
        case 200:                   /* read/write */
        case 201:                   /* read only */
            break;
        case 432:
        case 433:
        case 434:
            cddb_errno_log_error(c, CDDB_ERR_PERMISSION_DENIED);
            return FALSE;
    }

    /* send hello and check response */
    if (!cddb_send_cmd(c, CMD_HELLO, c->user, c->hostname, c->cname, c->cversion)) {
        return FALSE;
    }
    switch (code = cddb_get_response_code(c, &msg)) {
        case  -1:
            return FALSE;
        case 200:                   /* ok */
        case 402:                   /* already shook hands */
            break;
        case 431:
            cddb_errno_log_error(c, CDDB_ERR_PERMISSION_DENIED);
            return FALSE;
    }

    /* set protocol level */
    if (!cddb_send_cmd(c, CMD_PROTO, DEFAULT_PROTOCOL_VERSION)) {
        return FALSE;
    }
    switch (code = cddb_get_response_code(c, &msg)) {
        case  -1:
            return FALSE;
        case 200:                   /* ok */
        case 201:                   /* ok */
        case 502:                   /* protocol already set */
            break;
        case 501:                   /* illegal protocol level */
            /* ignore */
            break;
    }
    
    cddb_errno_set(c, CDDB_ERR_OK);
    return TRUE;
}

int cddb_connect(cddb_conn_t *c)
{
    int rv = TRUE;

    cddb_log_debug("cddb_connect()");
    if (!CONNECTION_OK(c)) {
        struct hostent *he;

        /* resolve host name */
        if (c->is_http_proxy_enabled) {
            /* use HTTP proxy server name */
            he = timeout_gethostbyname(c->http_proxy_server, c->timeout);
            c->sa.sin_port = htons(c->http_proxy_server_port);
        } else {
            /* use CDDB server name */
            he = timeout_gethostbyname(c->server_name, c->timeout);
            c->sa.sin_port = htons(c->server_port);
        }
        if (he == NULL) {
            cddb_errno_log_error(c, CDDB_ERR_UNKNOWN_HOST_NAME);
            return FALSE;
        }
        /* initialize socket address */
        c->sa.sin_family = AF_INET;
        c->sa.sin_addr = *((struct in_addr*)he->h_addr);
        memset(&(c->sa.sin_zero), 0, sizeof(c->sa.sin_zero));

        if ((c->socket  = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            cddb_errno_log_error(c, CDDB_ERR_CONNECT);
            return FALSE;
        }

        rv =  timeout_connect(c->socket, (struct sockaddr*)&(c->sa), 
                              sizeof(struct sockaddr), c->timeout);
        if (rv == -1) {
            cddb_errno_log_error(c, CDDB_ERR_CONNECT);
            return FALSE;
        } 

        if (!c->is_http_enabled) {
            /* send handshake message to CDDB server (CDDBP only) */
            return cddb_handshake(c);
        }
    }

    cddb_errno_set(c, CDDB_ERR_OK);
    return TRUE;
}

void cddb_disconnect(cddb_conn_t *c)
{
    cddb_log_debug("cddb_disconnect()");
    if (CONNECTION_OK(c)) {
        close(c->socket);
        c->socket = -1;
    }
    cddb_errno_set(c, CDDB_ERR_OK);
}
