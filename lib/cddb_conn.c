
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "cddb/cddb_ni.h"


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

    cddb_regex_init();

    c = (cddb_conn_t*)malloc(sizeof(cddb_conn_t));
    if (c) {
        c->line = (char*)malloc(LINE_SIZE);

        c->is_connected = FALSE;
        c->sockfd = -1;
        c->fp = NULL;
        c->server_name = strdup(DEFAULT_SERVER);
        c->server_port = DEFAULT_PORT;

        c->http_path_query = strdup(DEFAULT_PATH_QUERY);
        c->http_path_submit = strdup(DEFAULT_PATH_SUBMIT);

        c->is_http_enabled = FALSE;
        c->is_http_proxy_enabled = FALSE;
        c->http_proxy_server = NULL;
        c->http_proxy_server_port = DEFAULT_PROXY_PORT;

        c->use_cache = TRUE;
        /* construct cache dir '$HOME/[DEFAULT_CACHE]' */
        s = getenv("HOME");
        c->cache_dir = (char*)malloc(strlen(s) + 1 + sizeof(DEFAULT_CACHE) + 1);
        sprintf(c->cache_dir, "%s/%s", s, DEFAULT_CACHE);

        /* retrieve user and host name from environment */
        s = getenv("USER");
        c->user = s ? strdup(s) : strdup(DEFAULT_USER);
        s = getenv("HOSTNAME");
        c->hostname = s ? strdup(s) : strdup(DEFAULT_HOST);

        c->errnum = CDDB_ERR_OK;

        c->query_data = NULL;
        c->query_idx = 0;
        c->query_cnt = 0;
    }

    return c;
}

void cddb_destroy(cddb_conn_t *c)
{
    if (c) {
        cddb_disconnect(c);
        FREE_NOT_NULL(c->line);
        FREE_NOT_NULL(c->server_name);
        FREE_NOT_NULL(c->http_path_query);
        FREE_NOT_NULL(c->http_path_submit);
        FREE_NOT_NULL(c->cache_dir);
        FREE_NOT_NULL(c->user);
        FREE_NOT_NULL(c->hostname);
        cddb_query_clear(c);
        free(c);
    }
}


/* --- getters & setters --- */


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

void cddb_http_enable(cddb_conn_t *c, int enable)
{
    c->is_http_enabled = enable;
    c->errnum = CDDB_ERR_OK;
}

void cddb_http_proxy_enable(cddb_conn_t *c, int enable)
{
    if (enable) {
        /* enabling HTTP proxy implies HTTP, but not vice versa */
        cddb_http_enable(c, enable);
    }
    c->is_http_proxy_enabled = enable;
    c->errnum = CDDB_ERR_OK;
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

int cddb_set_email_address(cddb_conn_t *c, const char *email)
{
    char *at;
    int len;

    dlog("cddb_set_email_address()");
    if ((email == NULL) ||
        ((at = strchr(email, '@')) == NULL) ||
        (at == email) || 
        (*(at+1) == '\0')) {
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
    dlog("\tuser name = '%s'", c->user);
    dlog("\thost name = '%s'", c->hostname);

    return TRUE;
}


/* --- connecting / disconnecting --- */


int cddb_handshake(cddb_conn_t *c)
{
    const char *msg;
    int code;

    dlog("cddb_handshake()");
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
        c->errnum = CDDB_ERR_PERMISSION_DENIED;
        return FALSE;
    }

    /* send hello and check response */
    if (!cddb_send_cmd(c, CMD_HELLO, c->user, c->hostname, CLIENT_NAME, CLIENT_VERSION)) {
        return FALSE;
    }
    switch (code = cddb_get_response_code(c, &msg)) {
    case  -1:
        return FALSE;
    case 200:                   /* ok */
    case 402:                   /* already shook hands */
        break;
    case 431:
        c->errnum = CDDB_ERR_PERMISSION_DENIED;
        return FALSE;
    }

    c->errnum = CDDB_ERR_OK;
    return TRUE;
}

int cddb_connect(cddb_conn_t *c)
{
    int rv = TRUE;

    dlog("cddb_connect()");
    if (!CONNECTION_OK(c)) {
        struct hostent *he;

        /* resolve host name */
        if (c->is_http_proxy_enabled) {
            /* use HTTP proxy server name */
            he = gethostbyname(c->http_proxy_server);
            c->sa.sin_port = htons(c->http_proxy_server_port);
        } else {
            /* use CDDB server name */
            he = gethostbyname(c->server_name);
            c->sa.sin_port = htons(c->server_port);
        }
        if (he == NULL) {
            c->errnum = CDDB_ERR_UNKNOWN_HOST_NAME;
            return FALSE;
        }
        /* initialize socket address */
        c->sa.sin_family = AF_INET;
        c->sa.sin_addr = *((struct in_addr*)he->h_addr);
        bzero(&(c->sa.sin_zero), 8); /* zero the rest of the struct */

        if ((c->sockfd  = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            c->errnum = CDDB_ERR_CONNECT;
            return FALSE;
        }

        rv =  connect(c->sockfd, (struct sockaddr*)&(c->sa), sizeof(struct sockaddr));
        if (rv == -1) {
            c->errnum = CDDB_ERR_CONNECT;
            return FALSE;
        } 

        c->fp = fdopen(c->sockfd, "w+");
        if (c->fp == NULL) {
            c->errnum = CDDB_ERR_CONNECT;
            return FALSE;
        }

        if (!c->is_http_enabled) {
            /* send handshake message to CDDB server (CDDBP only) */
            return cddb_handshake(c);
        }
    }

    c->errnum = CDDB_ERR_OK;
    return TRUE;
}

void cddb_disconnect(cddb_conn_t *c)
{
    const char *msg;
    int code;

    dlog("cddb_disconnect()");
    if (CONNECTION_OK(c)) {
        /*
        if (!c->is_http_enabled) {
            void *old_handler = signal(SIGPIPE, SIG_IGN);
            cddb_send_cmd(c, CMD_QUIT);        
            code = cddb_get_response_code(c, &msg);
            signal(SIGPIPE, old_handler);
        }
        */
        fclose(c->fp);
        close(c->sockfd);
        c->sockfd = -1;
        c->fp = NULL;
    }
    c->errnum = CDDB_ERR_OK;
}




