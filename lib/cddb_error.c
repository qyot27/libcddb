

#include "cddb/cddb_ni.h"


static const char* err_str[CDDB_ERR_LAST] = {
    /* CDDB_ERR_OK */
    "ok",

    /* CDDB_ERR_OUT_OF_MEMORY */
    "out of memory",
    /* CDDB_ERR_NOT_IMPLEMENTED */
    "feature not implemented",
    /* CDDB_ERR_UNKNOWN */
    "problem unknown",

    /* CDDB_ERR_SERVER_ERROR */
    "server error",
    /* CDDB_ERR_UNKNOWN_HOST_NAME */
    "unknown host name",
    /* CDDB_ERR_CONNECT */
    "connection error",
    /* CDDB_ERR_PERMISSION_DENIED */
    "permission denied",
    /* CDDB_ERR_NOT_CONNECTED */
    "not connected",

    /* CDDB_ERR_UNEXPECTED_EOF */
    "unexpected end-of-file",
    /* CDDB_ERR_INVALID_RESPONSE */
    "invalid response data",
    /* CDDB_ERR_DISC_NOT_FOUND */
    "disc not found",

    /* CDDN_ERR_DATA_MISSING */
    "command data missing",
    /* CDDB_ERR_TRACK_NOT_FOUND */
    "track not found",
    /* CDDB_ERR_REJECTED */
    "posted data rejected",
    
    /** CDDB_ERR_LAST */
};

const char *cddb_error_str(cddb_error_t errnum)
{
    if ((errnum < 0) || (errnum >= CDDB_ERR_LAST)) {
        return NULL;
    } else {
        return  err_str[errnum];
    }
}

void cddb_error_stream_print(FILE *stream, cddb_error_t errnum)
{
    fprintf(stream, "libcddb: error: %s\n", cddb_error_str(errnum));
}

void cddb_error_print(cddb_error_t errnum)
{
    cddb_error_stream_print(stderr, errnum);
}
