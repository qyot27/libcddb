#ifndef CDDB_CMD_H
#define CDDB_CMD_H 1


/* --- accessing data on the CDDB server --- */


/**
 * Retrieve a disc record from the CDDB server.  This function
 * requires that the category and disc ID of the provided disc
 * structure are valid.
 *
 * @param c    The CDDB connection structure.
 * @param disc A non-null CDDB disc structure.
 */
int cddb_read(cddb_conn_t *c, cddb_disc_t *disc);

/**
 * Query the CDDB database for a list of possible disc matches.  This
 * function requires that the disc ID and disc length of the provided
 * disc structure are valid.  The disc should also contain a number of
 * tracks and for each track its frame offset on the CD should be
 * valid.
 *
 * \todo support for multiple matches
 *
 * @param c    The CDDB connection structure.
 * @param disc A non-null CDDB disc structure.
 */
int cddb_query(cddb_conn_t *c, cddb_disc_t *disc);

/**
 * Submit a new or updated disc to the CDDB database.  This function
 * requires that the disc ID, length, category, artist and title of
 * the provided disc structure are valid.  The disc should also
 * contain a number of tracks and for each track its frame offset on
 * the CD and title should be valid.
 *
 * @param c    The CDDB connection structure.
 * @param disc A non-null CDDB disc structure.
 */
int cddb_write(cddb_conn_t *c, cddb_disc_t *disc);


#endif /* CDDB_CMD_H */
