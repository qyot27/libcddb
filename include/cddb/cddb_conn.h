#ifndef CDDB_CONN_H
#define CDDB_CONN_H 1


#include <netinet/in.h>


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
    int sockfd;                 /**< the socket file descriptor */
    FILE *fp;                   /**< a FILE pointer for easier reading
                                     and writing on the socket */
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

    int use_cache;              /**< boolean to specify whether to read/write
                                     data in cache, enabled by default */
    char *cache_dir;            /**< CDDB slave cache, defaults to 
                                     '~/.cddbslave' */

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
 * to be passed to all libcddb functions.
 *
 * @return The CDDB connection structure or NULL if something went
 *         wrong.
 */
cddb_conn_t *cddb_new(void);

/**
 * Free all resources associated with the given CDDB connection
 * structure.
 */
void cddb_destroy(cddb_conn_t *c);


/* --- getters & setters --- */


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
 * Enable HTTP tunneling to connect the CDDB server.  By default this
 * option is disabled.
 *
 * @param c      The CDDB connection structure.
 * @param enable 0 to disable, anything else to enable.
 */
void cddb_http_enable(cddb_conn_t *c, int enable);

/**
 * Enable HTTP tunneling through an HTTP proxy server to connect the
 * CDDB server.  The usage of an HTTP proxy implies normal HTTP
 * tunneling instead of connecting directly to the CDDB server.  By
 * default this option is disabled.
 *
 * @param c      The CDDB connection structure.
 * @param enable 0 to disable, anything else to enable.
 */
void cddb_http_proxy_enable(cddb_conn_t *c, int enable);

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
 * Get the error number returned by the last libcddb command.
 *
 * @param c      The CDDB connection structure.
 */
#define cddb_errno(c) (c)->errnum

/**
 * Sets the user name and host name of the local machine.  This
 * function will parse out the user name and host name from the e-mail
 * address.
 *
 * @param c     The connection structure.
 * @param email The e-mail address of the user.
 */
int cddb_set_email_address(cddb_conn_t *c, const char *email);


#endif /* CDDB_CONN_H */
