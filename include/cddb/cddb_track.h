#ifndef CDDB_TRACK_H
#define CDDB_TRACK_H 1


/**
 * The CDDB track structure.  Contains all information associated with
 * a single CD track.  This structure will be used to populate the
 * tracks linked list of the cddb_disc_s structure.
 */
typedef struct cddb_track_s
{
    int num;                    /**< track number on the disc */
    int frame_offset;           /**< frame offset of the track on the disc */
    int length;                 /**< track length in seconds */
    char *title;                /**< track title */
    char *artist;               /**< (optional) track artist */
    char *ext_data;             /**< (optional) extended disc data  */
    struct cddb_track_s *next;  /**< pointer to next track, or NULL  */
} cddb_track_t;


/* --- construction / destruction */


/**
 * Creates a new CDDB track structure.
 *
 * @return The CDDB track structure.
 */
cddb_track_t *cddb_track_new(void);

/**
 * Free all resources associated with the given CDDB track structure.
 * The linked list pointer (next) will not be touched.  So you have to
 * make sure that no other tracks are attached to this one before
 * calling this function.
 *
 * @param track The CDDB track structure.
 */
void cddb_track_destroy(cddb_track_t *track);


/* --- getters & setters --- */


/**
 * Set the track title.  If the track already had a title, then the
 * memory for that string will be freed.  The new title will be copied
 * into a new chunk of memory.
 *
 * @param track The CDDB track structure.
 * @param title The new track title.
 */
void cddb_track_set_title(cddb_track_t *track, const char *title);

/**
 * Set the track artist name.  If the track already had an artist
 * name, then the memory for that string will be freed.  The new
 * artist name will be copied into a new chunk of memory.
 *
 * @param track  The CDDB track structure.
 * @param artist The new track artist name.
 */
void cddb_track_set_artist(cddb_track_t *track, const char *artist);


/* --- miscellaneous */


/**
 * Prints information about the track on stdout.  This is just a
 * debugging routine to display the structure's content.  It is used
 * by cddb_disc_print to print the contents of a complete disc.
 *
 * @param track The CDDB track structure.
 */
void cddb_track_print(cddb_track_t *track);


#endif /* CDDB_TRACK_H */
