/*
    $Id: cddb_conn.h,v 1.27 2005/05/29 08:09:52 airborne Exp $

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

#ifndef CDDB_CONN_H
#define CDDB_CONN_H 1

#ifdef __cplusplus
    extern "C" {
#endif


#include <stdio.h>
#include <netinet/in.h>

#include "cddb/ll.h"
#include "cddb/cddb_site.h"


#define CACHE_OFF  0            /**< do not use local CDDB cache, network
                                     only */
#define CACHE_ON   1            /**< use local CDDB cache, if possible */
#define CACHE_ONLY 2            /**< only use local CDDB cache, no network
                                     access */

/**
 * Forward declaration of opaque structure used for character set
 * conversions.
 */
typedef struct cddb_iconv_s *cddb_iconv_t;

/**
 * An opaque structure for keeping state about the connection to a
 * CDDB server.
 */
typedef struct cddb_conn_s 
{
    unsigned int buf_size;      /**< maximum line/buffer size, defaults to 1024
                                     (see DEFAULT_BUF_SIZE) */
    char *line;                 /**< last line read */

    int is_connected;           /**< are we already connected to the server? */
    struct sockaddr_in sa;      /**< the socket address structure for
                                     connecting to the CDDB server */
    int socket;                 /**< the socket file descriptor */
    char *server_name;          /**< host name of the CDDB server, defaults
                                     to 'freedb.org' (see DEFAULT_SERVER) */
    int server_port;            /**< port of the CDDB server, defaults to 888 
                                     (see DEFAULT_PORT) */
    int timeout;                /**< time out interval (in seconds) used during
                                     network operations, defaults to 10 seconds
                                     (see DEFAULT_TIMEOUT) */

    char *http_path_query;      /**< URL for querying the server through HTTP,
                                     defaults to /~cddb/cddb.cgi'
                                     (see DEFAULT_PATH_QUERY) */
    char *http_path_submit;     /**< URL for submitting to the server through HTTP,
                                     defaults to /~cddb/submit.cgi'
                                     (see DEFAULT_PATH_SUBMIT) */
    int is_http_enabled;        /**< use HTTP, disabled by default */

    int is_http_proxy_enabled;  /**< use HTTP through a proxy server,
                                     disabled by default */
    char *http_proxy_server;    /**< host name of the HTTP proxy server */
    int http_proxy_server_port; /**< port of the HTTP proxy server,
                                     defaults to 8080 (see DEFAULT_PROXY_PORT) */
    char *http_proxy_username;  /**< HTTP proxy user name */
    char *http_proxy_password;  /**< HTTP proxy password */
    char *http_proxy_auth;      /**< Base64 encoded username:password */

    FILE *cache_fp;             /**< a file pointer to a cached CDDB entry or
                                     NULL if no cached version is available */
    int use_cache;              /**< field to specify local CDDB cache behaviour, 
                                     enabled by default (CACHE_ON) */
    char *cache_dir;            /**< CDDB slave cache, defaults to 
                                     '~/.cddbslave' (see DEFAULT_CACHE) */
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

    list_t *query_data;         /**< list to keep CDDB query results */
    list_t *sites_data;         /**< list to keep FreeDB mirror sites */

    cddb_iconv_t charset;       /**< character set conversion settings */
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
 * Set the character set.  By default the FreeDB server uses UTF-8 when
 * providing CD data.  When a character set is defined with this function
 * any strings retrieved from or sent to the server will automatically be
 * converted.
 *
 * @param c The connection structure.
 * @param cs The character set that will be used.
 * @return FALSE if the specified character set is unknown, or no conversion
 *         from/to UTF-8 is available.  TRUE otherwise.
 */
int cddb_set_charset(cddb_conn_t *c, const char *cs);

/**
 * Change the size of the internal buffer.
 *
 * @param c The connection structure.
 * @param size The new buffer size.
 */
void cddb_set_buf_size(cddb_conn_t *c, unsigned int size);

/**
 * Get the host name of the CDDB server that is currently being used.
 *
 * @see cddb_set_server_name
 *
 * @param c The connection structure.
 * @return The server host name.
 */
#define cddb_get_server_name(c) (c)->server_name

/**
 * Set the host name of the CDDB server.  The default value for the
 * server is 'freedb.org'.
 *
 * @see cddb_get_server_name
 *
 * @param c      The connection structure.
 * @param server The server host name.
 */
void cddb_set_server_name(cddb_conn_t *c, const char *server);

/**
 * Get the port of the CDDB server that is currently being used.
 *
 * @see cddb_set_server_port
 *
 * @param c The connection structure.
 * @return The server port.
 */
#define cddb_get_server_port(c) (c)->server_port

/**
 * Set the port of the CDDB server.  The default value is 888.
 *
 * @see cddb_get_server_port
 *
 * @param c    The connection structure.
 * @param port The server port.
 */
void cddb_set_server_port(cddb_conn_t *c, int port);

/**
 * Get the network time out value (in seconds).
 *
 * @see cddb_set_timeout
 *
 * @param c The connection structure.
 * @return The current time out in seconds.
 */
#define cddb_get_timeout(c) (c)->timeout

/**
 * Set the network time out value (in seconds).  The default is 10
 * seconds.
 *
 * @see cddb_get_timeout
 *
 * @param c The connection structure.
 * @param t The new time out in seconds.
 */
#define cddb_set_timeout(c, t) (c)->timeout = t

/**
 * Get the URL path for querying a CDDB server through HTTP.
 *
 * @see cddb_set_http_path_query
 *
 * @param c The connection structure.
 * @return The URL path.
 */
#define cddb_get_http_path_query(c) (c)->http_path_query

/**
 * Set the URL path for querying a CDDB server through HTTP.  The
 * default value is '/~cddb/cddb.cgi'.
 *
 * @see cddb_get_http_path_query
 *
 * @param c    The connection structure.
 * @param path The URL path.
 */
void cddb_set_http_path_query(cddb_conn_t *c, const char *path);

/**
 * Get the URL path for submitting to a CDDB server through HTTP.
 *
 * @see cddb_set_http_path_submit
 *
 * @param c The connection structure.
 * @return The URL path.
 */
#define cddb_get_http_path_submit(c) (c)->http_path_submit

/**
 * Set the URL path for submitting to a CDDB server through HTTP.  The
 * default value is '/~cddb/submit.cgi'.
 *
 * @see cddb_get_http_path_submit
 *
 * @param c The connection structure.
 * @param path The URL path.
 */
void cddb_set_http_path_submit(cddb_conn_t *c, const char *path);

/**
 * Returns true if the HTTP protocol is currently enabled and false if
 * CDDBP is enabled.
 *
 * @see cddb_http_enable
 * @see cddb_http_disable
 *
 * @param c The CDDB connection structure.
 * @return True or false.
 */
#define cddb_is_http_enabled(c) (c)->is_http_enabled

/**
 * Enable HTTP tunneling to connect to the CDDB server.  By default
 * this option is disabled.
 *
 * @see cddb_is_http_enabled
 * @see cddb_http_disable
 *
 * @param c The CDDB connection structure.
 */
void cddb_http_enable(cddb_conn_t *c);

/**
 * Disable HTTP tunneling to connect to the CDDB server.  By default this
 * option is disabled.
 *
 * @see cddb_is_http_enabled
 * @see cddb_http_enable
 *
 * @param c The CDDB connection structure.
 */
void cddb_http_disable(cddb_conn_t *c);

/**
 * Returns true if the proxy support is currently enabled and false if
 * it is not.  This fucntion does not check whether HTTP is enabled.
 * So it is possible that true will be returned while in reality the
 * CDDBP protocol is being used (no proxy support).
 *
 * @see cddb_http_proxy_enable
 * @see cddb_http_proxy_disable
 *
 * @param c The CDDB connection structure.
 * @return True or false.
 */
#define cddb_is_http_proxy_enabled(c) (c)->is_http_proxy_enabled

/**
 * Enable HTTP tunneling through an HTTP proxy server to connect to
 * the CDDB server.  The usage of an HTTP proxy implies normal HTTP
 * tunneling instead of connecting directly to the CDDB server.  By
 * default this option is disabled.
 *
 * @see cddb_is_http_proxy_enabled
 * @see cddb_http_proxy_disable
 *
 * @param c The CDDB connection structure.
 */
void cddb_http_proxy_enable(cddb_conn_t *c);

/**
 * Disable HTTP tunneling through an HTTP proxy server to connect to
 * the CDDB server.  By default this option is disabled.
 *
 * @see cddb_is_http_proxy_enabled
 * @see cddb_http_proxy_enable
 *
 * @param c The CDDB connection structure.
 */
void cddb_http_proxy_disable(cddb_conn_t *c);

/**
 * Get the host name of the HTTP proxy server.
 *
 * @see cddb_set_http_proxy_server_name
 *
 * @param c The connection structure.
 * @return The proxy server host name.
 */
#define cddb_get_http_proxy_server_name(c) (c)->http_proxy_server

/**
 * Set the host name of the HTTP proxy server.  There is no default
 * value.
 *
 * @see cddb_get_http_proxy_server_name
 *
 * @param c      The connection structure.
 * @param server The server host name.
 */
void cddb_set_http_proxy_server_name(cddb_conn_t *c, const char *server);

/**
 * Get the port of the HTTP proxy server.
 *
 * @see cddb_set_http_proxy_server_port
 *
 * @param c The connection structure.
 * @return The proxy server port.
 */
#define cddb_get_http_proxy_server_port(c) (c)->http_proxy_server_port

/**
 * Set the port of the HTTP proxy server.  The default value is 8080.
 *
 * @see cddb_get_http_proxy_server_port
 *
 * @param c    The connection structure.
 * @param port The server port.
 */
void cddb_set_http_proxy_server_port(cddb_conn_t *c, int port);

/**
 * Set the HTTP proxy user name which is used when Basic Authentication
 * is required.
 *
 * @param c        The connection structure.
 * @param username The user name.
 */
 
void cddb_set_http_proxy_username(cddb_conn_t* c, const char* username);

/**
 * Get the HTTP proxy user name.
 *
 * @param c The connection structure.
 * @return The user name.
 */
 
#define cddb_get_http_proxy_username(c) (c)->http_proxy_username

/**
 * Set the HTTP proxy password which is used when Basic Authentication
 * is required.
 *
 * @param c      The connection structure.
 * @param passwd The password.
 */
 
void cddb_set_http_proxy_password(cddb_conn_t* c, const char* passwd);

/**
 * Get the HTTP proxy password. 
 *
 * @param c The connection structure.
 * @return The password.
 */
 
#define cddb_get_http_proxy_password(c) (c)->http_proxy_password

/**
 * Set the HTTP proxy user name and password in one go.  These
 * credentials are used when Basic Authentication is required.  The
 * advantage of using this function over setting the user name and
 * password seperately is that the cleartext user name and password
 * are not kept in memory longer than needed.
 *
 * @param c        The connection structure.
 * @param username The user name.
 * @param passwd   The password.
 */
void cddb_set_http_proxy_credentials(cddb_conn_t* c,
                                     const char *username, const char* passwd);

/**
 * Get the error number returned by the last libcddb command.
 *
 * @param c The CDDB connection structure.
 * @return The error number.
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
 * @see cddb_cache_enable
 * @see cddb_cache_only
 * @see cddb_cache_disable
 *
 * @param c The connection structure.
 */
#define cddb_cache_mode(c) (c)->use_cache

/**
 * Enable caching of CDDB entries locally.  Caching is enabled by
 * default.  The cache directory can be changed with the
 * cddb_cache_set_dir function.
 *
 * @see cddb_cache_mode
 * @see cddb_cache_disable
 * @see cddb_cache_only
 *
 * @param c The connection structure.
 */
#define cddb_cache_enable(c) (c)->use_cache = CACHE_ON

/**
 * Only use the local CDDB cache.  Never contact a server to retrieve
 * any data.  The cache directory can be changed with the
 * cddb_cache_set_dir function.
 *
 * @see cddb_cache_mode
 * @see cddb_cache_enable
 * @see cddb_cache_disable
 *
 * @param c The connection structure.
 */
#define cddb_cache_only(c) (c)->use_cache = CACHE_ONLY

/**
 * Disable caching of CDDB entries locally.  All data will be fetched
 * from a CDDB server everytime and the retrieved data will not be
 * cached locally.
 *
 * @see cddb_cache_mode
 * @see cddb_cache_enable
 * @see cddb_cache_only
 *
 * @param c The connection structure.
 */
#define cddb_cache_disable(c) (c)->use_cache = CACHE_OFF

/**
 * Return the directory currently being used for caching.
 *
 * @see cddb_cache_set_dir
 *
 * @param c The connection structure.
 * @return The directory being used for caching.
 */
#define cddb_cache_get_dir(c) (c)->cache_dir;

/**
 * Change the directory used for caching CDDB entries locally.  The
 * default location of the cached entries is a subdirectory
 * (.cddbslave) of the user's home directory.  If the first character
 * of the directory is '~', then it will be expanded to the contents
 * of $HOME.
 *
 * @see cddb_cache_get_dir
 *
 * @param c   The connection structure.
 * @param dir The directory to use for caching.
 */
int cddb_cache_set_dir(cddb_conn_t *c, const char *dir);

/**
 * Retrieve the first CDDB mirror site.
 *
 * @param c The connection structure.
 * @return The first mirror site or NULL if not found.
 */
cddb_site_t *cddb_first_site(cddb_conn_t *c);

/**
 * Retrieve the next CDDB mirror site.
 *
 * @param c The connection structure.
 * @return The next mirror site or NULL if not found.
 */
cddb_site_t *cddb_next_site(cddb_conn_t *c);


#ifdef __cplusplus
    }
#endif

#endif /* CDDB_CONN_H */
