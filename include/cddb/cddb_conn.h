/*
    $Id: cddb_conn.h,v 1.13 2003/05/01 09:49:38 airborne Exp $

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

#ifndef CDDB_CONN_H
#define CDDB_CONN_H 1

#ifdef __cplusplus
    extern "C" {
#endif


#include <netinet/in.h>

#define CACHE_OFF  0            /**< do not use local CDDB cache, network
                                     only */
#define CACHE_ON   1            /**< use local CDDB cache, if possible */
#define CACHE_ONLY 2            /**< only use local CDDB cache, no network
                                     access */

/**
 * An opaque structure for keeping state about the connection to a
 * CDDB server.
 */
typedef struct cddb_conn_s 
{
    char *line;                 /**< last line read */

    int is_connected;           /**< are we already connected to the server? */
    struct sockaddr_in sa;      /**< the socket address structure for
                                     connecting to the CDDB server */
    int socket;                 /**< the socket file descriptor */
    char *server_name;          /**< host name of the CDDB server, defaults
                                     to 'freedb.org' */
    int server_port;            /**< port of the CDDB server, defaults to 888 */

    char *http_path_query;      /**< URL for querying the server through HTTP,
                                     defaults to /~cddb/cddb.cgi' */
    char *http_path_submit;     /**< URL for submitting to the server through HTTP,
                                     defaults to /~cddb/submit.cgi' */
    int is_http_enabled;        /**< use HTTP, disabled by default */

    int is_http_proxy_enabled;  /**< use HTTP through a proxy server,
                                     disabled by default */
    char *http_proxy_server;    /**< host name of the HTTP proxy server */
    int http_proxy_server_port; /**< port of the HTTP proxy server,
                                     defaults to 8080 */

    FILE *cache_fp;             /**< a file pointer to a cached CDDB entry or
                                     NULL if no cached version is available */
    int use_cache;              /**< field to specify local CDDB cache behaviour, 
                                     enabled by default (CACHE_ON) */
    char *cache_dir;            /**< CDDB slave cache, defaults to 
                                     '~/.cddbslave' */
    int cache_read;             /**< read data from cached file instead of
                                     from the network */

    char *cname;                /**< name of the client program, 'libcddb' by
                                     default */
    char *cversion;             /**< version of the client program, current 
                                     libcddb version by default */
    char *user;                 /**< user name supplied to CDDB server, defaults
                                     to the value of the 'USER' environment 
                                     variable or 'anonymous' if undefined */
    char *hostname;             /**< host name of the local machine, defaults
                                     to the value of the 'HOSTNAME' environment
                                     variable or 'localhost' if undefined */

    cddb_error_t errnum;        /**< error number of last CDDB command */

    cddb_disc_t **query_data;   /**< data structure to keep CDDB query results */
    int query_idx;              /**< iterator index for query result set */
    int query_cnt;              /**< number of entries in query result set */
} cddb_conn_t;


/* --- construction / destruction --- */


/**
 * Creates a new CDDB connection structure.  This structure will have
 * to be passed to all libcddb functions.  Default values will be used
 * for the connection parameters allowing it to contact the CDDB
 * server at freedb.org.
 *
 * @return The CDDB connection structure or NULL if something went wrong.
 */
cddb_conn_t *cddb_new(void);

/**
 * Free all resources associated with the given CDDB connection
 * structure.
 */
void cddb_destroy(cddb_conn_t *c);


/* --- getters & setters --- */


/**
 * Get the host name of the CDDB server that is currently being used.
 *
 * @param c The connection structure.
 * @return The server host name.
 */
#define cddb_get_server_name(c) (c)->server_name;

/**
 * Set the host name of the CDDB server.  The default value for the
 * server is 'freedb.org'.
 *
 * @param c      The connection structure.
 * @param server The server host name.
 */
void cddb_set_server_name(cddb_conn_t *c, const char *server);

/**
 * Set the port of the CDDB server.  The default value is 888.
 *
 * @param c    The connection structure.
 * @param port The server port.
 */
void cddb_set_server_port(cddb_conn_t *c, int port);

/**
 * Set the URL path for querying a CDDB server through HTTP.  The
 * default value is '/~cddb/cddb.cgi'.
 *
 * @param c    The connection structure.
 * @param path The URL path.
 */
void cddb_set_http_path_query(cddb_conn_t *c, const char *path);

/**
 * Set the URL path for submitting to a CDDB server through HTTP.  The
 * default value is '/~cddb/submit.cgi'.
 *
 * @param c The connection structure.
 * @param path The URL path.
 */
void cddb_set_http_path_submit(cddb_conn_t *c, const char *path);

/**
 * Enable HTTP tunneling to connect to the CDDB server.  By default
 * this option is disabled.
 *
 * @param c The CDDB connection structure.
 */
void cddb_http_enable(cddb_conn_t *c);

/**
 * Disable HTTP tunneling to connect to the CDDB server.  By default this
 * option is disabled.
 *
 * @param c The CDDB connection structure.
 */
void cddb_http_disable(cddb_conn_t *c);

/**
 * Enable HTTP tunneling through an HTTP proxy server to connect to
 * the CDDB server.  The usage of an HTTP proxy implies normal HTTP
 * tunneling instead of connecting directly to the CDDB server.  By
 * default this option is disabled.
 *
 * @param c The CDDB connection structure.
 */
void cddb_http_proxy_enable(cddb_conn_t *c);

/**
 * Disable HTTP tunneling through an HTTP proxy server to connect to
 * the CDDB server.  By default this option is disabled.
 *
 * @param c The CDDB connection structure.
 */
void cddb_http_proxy_disable(cddb_conn_t *c);

/**
 * Set the host name of the HTTP proxy server.  There is no default
 * value.
 *
 * @param c      The connection structure.
 * @param server The server host name.
 */
void cddb_set_http_proxy_server_name(cddb_conn_t *c, const char *server);

/**
 * Set the port of the HTTP proxy server.  The default value is 8080.
 *
 * @param c    The connection structure.
 * @param port The server port.
 */
void cddb_set_http_proxy_server_port(cddb_conn_t *c, int port);

/**
 * Set the port of the HTTP proxy server.  The default value is 8080.
 *
 * @param c    The connection structure.
 * @param port The server port.
 */
void cddb_set_http_proxy_server_port(cddb_conn_t *c, int port);

/**
 * Get the error number returned by the last libcddb command.
 *
 * @param c      The CDDB connection structure.
 */
#define cddb_errno(c) (c)->errnum

/**
 * Set the name and version of the client program overwriting the
 * previous values.  This function will make a copy of the provided
 * strings.  The defaults are 'libcddb' and the version number of the
 * libcddb library in use.  Both parameters must be valid strings.  If
 * any of teh strings is NULL, this fucntion will return without
 * changing anything.
 *
 * @param c        The connection structure.
 * @param cname    The name of the client program.
 * @param cversion The version number of the client program.
 */
void cddb_set_client(cddb_conn_t *c, const char *cname, const char *cversion);

/**
 * Sets the user name and host name of the local machine.  This
 * function will parse out the user name and host name from the e-mail
 * address.
 *
 * @param c     The connection structure.
 * @param email The e-mail address of the user.
 */
int cddb_set_email_address(cddb_conn_t *c, const char *email);

/**
 * Returns the current cache mode.  This can be either on, off or
 * cache only.
 *
 * @see CACHE_ON
 * @see CACHE_ONLY
 * @see CACHE_OFF
 *
 * @param c The connection structure.
 */
#define cddb_cache_mode(c) (c)->use_cache

/**
 * Enable caching of CDDB entries locally.  Caching is enabled by
 * default.  The cache directory can be changed with the
 * cddb_cache_set_dir function.
 *
 * @see cddb_cache_disable
 * @see cddb_cache_only
 * @see cddb_cache_set_dir
 *
 * @param c The connection structure.
 */
#define cddb_cache_enable(c) (c)->use_cache = CACHE_ON

/**
 * Only use the local CDDB cache.  Never contact a server to retrieve
 * any data.  The cache directory can be changed with the
 * cddb_cache_set_dir function.
 *
 * @see cddb_cache_enable
 * @see cddb_cache_disable
 * @see cddb_cache_set_dir
 *
 * @param c The connection structure.
 */
#define cddb_cache_only(c) (c)->use_cache = CACHE_ONLY

/**
 * Disable caching of CDDB entries locally.  All data will be fetched
 * from a CDDB server everytime and the retrieved data will not be
 * cached locally.
 *
 * @see cddb_cache_enable
 * @see cddb_cache_only
 * @see cddb_cache_set_dir
 *
 * @param c The connection structure.
 */
#define cddb_cache_disable(c) (c)->use_cache = CACHE_OFF

/**
 * Return the directory currently being used for caching.
 *
 * @param c The connection structure.
 * @return The directory being used for caching.
 */
#define cddb_cache_get_dir(c) (c)->cache_dir;

/**
 * Change the directory used for caching CDDB entries locally.  The
 * default location of the cached entries is a subdirectory
 * (.cddbslave) of the user's home directory.
 *
 * @param c   The connection structure.
 * @param dir The directory to use for caching.
 */
int cddb_cache_set_dir(cddb_conn_t *c, const char *dir);


#ifdef __cplusplus
    }
#endif

#endif /* CDDB_CONN_H */
