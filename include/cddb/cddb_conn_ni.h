#ifndef CDDB_CONN_NI_H
#define CDDB_CONN_NI_H 1

#ifdef __cplusplus
    extern "C" {
#endif


/* --- connecting / disconnecting --- */


/**
 */
int cddb_connect(cddb_conn_t *c);

/**
 */
void cddb_disconnect(cddb_conn_t *c);


#ifdef __cplusplus
    }
#endif

#endif /* CDDB_CONN_NI_H */
