
#include "main.h"


cddb_disc_t *do_read(cddb_conn_t *conn, const char *category, int discid)
{
    cddb_disc_t *disc = NULL;   /* libcddb disc structure */
    int success;

    /* Create a new disc structure. */
    disc = cddb_disc_new();

    /* If the pointer is NULL then an error occured (out of memory).
       Otherwise we continue. */
    if (disc) {
        /* Initialize the category of the disc.  This function
           converts a string into a category ID as used by libcddb.
           If the specified string does not match any of the known
           categories, then the category is set to 'misc'. */
        cddb_disc_set_category(disc, category);

        /* Initialize the ID of the disc. */
        cddb_disc_set_discid(disc, discid);

        /* Try reading the rest of the disc data.  This information
           will be retrieved from the server or read from the cache
           depending on the connection settings. */
        success = cddb_read(conn, disc);

        /* If an error occured then the return value will be false and the
           internal libcddb error number will be set. */
        if (!success) {
            /* Print an explanatory message on stderr.  Other routines are
               available for retrieving the message without printing it or
               printing it on a stream other than stderr. */
            cddb_error_print(cddb_errno(conn));
            /* Destroy the disc. */
            cddb_disc_destroy(disc);
            /* And return NULL to signal an error. */
            return NULL;
        }
    }

    return disc;
}
