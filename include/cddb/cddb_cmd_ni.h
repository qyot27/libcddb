#ifndef CDDB_CMD_NI_H
#define CDDB_CMD_NI_H 1

#ifdef __cplusplus
    extern "C" {
#endif


enum { CMD_HELLO = 0,
       CMD_QUIT,
       CMD_READ,
       CMD_QUERY,
       CMD_WRITE,
       CMD_PROTO,
       /* dummy for array size */
       CMD_LAST
};


/* --- utility functions --- */


/**
 * Will read in one line from the response input stream and parse both
 * the code and message in that line.  Errors will be signaled by
 * returning -1.
 *
 * @param c   the CDDB connection structure
 * @param msg the CDDB response msg
 * @return the CDDB response code or -1 on error
 */
int cddb_get_response_code(cddb_conn_t *c, char **msg);

/**
 */
int cddb_send_cmd(cddb_conn_t *c, int cmd, ...);

/**
 */
void cddb_query_clear(cddb_conn_t *c);


#ifdef __cplusplus
    }
#endif

#endif /* CDDB_CMD_H */
