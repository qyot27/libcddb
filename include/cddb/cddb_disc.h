#ifndef CDDB_DISC_H
#define CDDB_DISC_H 1

#ifdef __cplusplus
    extern "C" {
#endif


#include <cddb/cddb_track.h>


/**
 * The different CDDB categories.
 */
typedef enum {
    CDDB_CAT_DATA = 0, CDDB_CAT_FOLK, CDDB_CAT_JAZZ, CDDB_CAT_MISC,
    CDDB_CAT_ROCK, CDDB_CAT_COUNTRY, CDDB_CAT_BLUES, CDDB_CAT_NEWAGE,
    CDDB_CAT_REGGAE, CDDB_CAT_CLASSICAL, CDDB_CAT_SOUNDTRACK,
    CDDB_CAT_INVALID,
    CDDB_CAT_LAST
} cddb_cat_t;

/**
 * String values for the CDDB categories.
 */
extern const char *CDDB_CATEGORY[CDDB_CAT_LAST];

/**
 * The CDDB disc structure.  Contains all information associated with
 * a full CD.
 */
typedef struct cddb_disc_s
{
    unsigned int discid;        /**< four byte disc ID */
    cddb_cat_t category;        /**< CDDB category */
    char *genre;                /**< disc genre */
    char *title;                /**< disc title */
    char *artist;               /**< disc artist */
    int length;                 /**< disc length in seconds */
    int year;                   /**< (optional) disc year YYYY */
    char *ext_data;             /**< (optional) extended disc data  */
    int track_cnt;              /**< number of tracks on the disc */
    cddb_track_t *tracks;       /**< pointer to the first track */
    cddb_track_t *iterator;     /**< track iterator */
} cddb_disc_t;


/* --- construction / destruction */


/**
 * Creates a new CDDB disc structure.
 *
 * @return The CDDB disc structure or NULL if memory allocation failed.
 */
cddb_disc_t *cddb_disc_new(void);

/**
 * Free all resources associated with the given CDDB disc structure.
 * The tracks will also be freed automatically.
 *
 * @param disc The CDDB disc structure.
 */
void cddb_disc_destroy(cddb_disc_t *disc);

/**
 * Creates a clone of the given disc.
 *
 * @param disc The CDDB disc structure.
 */
cddb_disc_t *cddb_disc_clone(cddb_disc_t *disc);


/* --- track manipulation */


/**
 * Add a new track to a disc.  The track is added to the end of the
 * existing list of tracks.
 *
 * @param disc The CDDB disc structure.
 * @param track The CDDB track structure.
 */
void cddb_disc_add_track(cddb_disc_t *disc, cddb_track_t *track);

/**
 * Retrieves a numbered track from the disc.  If there is no track
 * with the given number, then NULL will be returned.
 *
 * @param disc The CDDB disc structure.
 * @param track_no The track number; starting at 0.
 */
cddb_track_t *cddb_disc_get_track(cddb_disc_t *disc, int track_no);

/**
 * Returns the first track of the disc.  If there is no such track
 * then NULL will be returned.  The internal track iterator will also
 * be reset.  This function should be called before the first call to
 * cddb_disc_get_track_next.
 *
 * @see cddb_disc_get_track_next
 *
 * @param disc The CDDB disc structure.
 */
cddb_track_t *cddb_disc_get_track_first(cddb_disc_t *disc);

/**
 * Returns the next track on the disc and advances the internal track
 * iterator.  If there is no such track then NULL will be returned.
 * This function should be called after calling
 * cddb_disc_get_track_first.
 *
 * @see cddb_disc_get_track_first
 *
 * @param disc The CDDB disc structure.
 */
cddb_track_t *cddb_disc_get_track_next(cddb_disc_t *disc);


/* --- setters / getters --- */


/**
 * Set the ID of the disc.  The disc ID is not known yet, then it can
 * be calculated with the cddb_disc_calc_discid function.
 *
 * @see cddb_disc_calc_discid
 *
 * @param disc The CDDB disc structure.
 * @param id The disc ID.
 */
#define cddb_disc_set_discid(disc, id) (disc)->discid = id

/**
 * Get the disc CDDB category ID.  If no category is set for this disc
 * then CDDB_CAT_INVALID will be returned.  If you want a string
 * representation of the category use the cddb_disc_get_category_str
 * function.
 *
 * @see cddb_disc_get_category_str
 *
 * @param disc The CDDB disc structure.
 * @return The CDDB category ID.
 */
#define cddb_disc_get_category(disc) (disc)->category

/**
 * Get the disc CDDB category as a string.  If no category is set for
 * this disc then 'invalid' will be returned.  If the disc structure
 * is invalid NULL is returned.  If you only want the ID of the
 * category use the cddb_disc_get_category function.
 *
 * @see cddb_disc_get_category
 *
 * @param disc The CDDB disc structure.
 * @return The CDDB category ID.
 */
const char *cddb_disc_get_category_str(cddb_disc_t *disc);

/**
 * Sets the category of the disc.  If the specified category is
 * an invalid CDDB category, then CDDB_CAT_MISC will be used.
 *
 * @see cddb_cat_t, CDDB_CATEGORY
 *
 * @param disc The CDDB disc structure.
 * @param cat The category string.
 */
void cddb_disc_set_category(cddb_disc_t *disc, const char *cat);

/**
 * Get the disc genre.  If no genre is set for this disc then NULL
 * will be returned.  As opposed to the disc category, this field is
 * not limited to a predefined set.
 *
 * @param disc The CDDB disc structure.
 * @return The disc genre.
 */
#define cddb_disc_get_genre(disc) (disc)->genre

/**
 * Set the disc genre.  As opposed to the disc category, this field is
 * not limited to a predefined set.  If the disc already had a genre,
 * then the memory for that string will be freed.  The new genre will
 * be copied into a new chunk of memory.
 *
 * @see cddb_disc_get_category_str
 *
 * @param disc  The CDDB disc structure.
 * @param genre The disc genre.
 */
void cddb_disc_set_genre(cddb_disc_t *disc, const char *genre);

/**
 * Get the disc length.  If no length is set for this disc then 0 will
 * be returned.
 *
 * @param disc The CDDB disc structure.
 * @return The disc length in seconds.
 */
#define cddb_disc_get_length(disc) (disc)->length

/**
 * Get the year of publication for this disc.  If no year is defined 0
 * is returned.
 *
 * @param disc The CDDB disc structure.
 * @return The disc year.
 */
#define cddb_disc_get_year(disc) (disc)->year

/**
 * Get the number of tracks on the disc.
 *
 * @param disc The CDDB disc structure.
 * @return The number of tracks.
 */
#define cddb_disc_get_track_count(disc) (disc)->track_cnt

/**
 * Get the disc title.  If no title is set for this disc then NULL
 * will be returned.
 *
 * @param disc The CDDB disc structure.
 * @return The disc title.
 */
#define cddb_disc_get_title(disc) (disc)->title

/**
 * Set the disc title.  If the disc already had a title, then the
 * memory for that string will be freed.  The new title will be copied
 * into a new chunk of memory.
 *
 * @param disc The CDDB disc structure.
 * @param title The new disc title.
 */
void cddb_disc_set_title(cddb_disc_t *disc, const char *title);

/**
 * Append to the disc title.  If the disc does not have a title yet,
 * then a new one will be created from the given string, otherwise
 * that string will be appended to the existing title.
 *
 * @param disc The CDDB disc structure.
 * @param title Part of the disc title.
 */
void cddb_disc_append_title(cddb_disc_t *disc, const char *title);

/**
 * Get the disc artist name.  If no artist is set for this disc then
 * NULL will be returned.
 *
 * @param disc The CDDB disc structure.
 * @return The disc artist name.
 */
#define cddb_disc_get_artist(disc) (disc)->artist

/**
 * Set the disc artist name.  If the disc already had an artist name,
 * then the memory for that string will be freed.  The new artist name
 * will be copied into a new chunk of memory.
 *
 * @param disc   The CDDB disc structure.
 * @param artist The new disc artist name.
 */
void cddb_disc_set_artist(cddb_disc_t *disc, const char *artist);

/**
 * Append to the disc artist.  If the disc does not have an artist
 * yet, then a new one will be created from the given string,
 * otherwise that string will be appended to the existing artist.
 *
 * @param disc  The CDDB disc structure.
 * @param artist Part of the artist name.
 */
void cddb_disc_append_artist(cddb_disc_t *disc, const char *artist);


/* --- miscellaneous */


/**
 * Copy all data from one disc to another.  Any fields that are
 * unavailable in the source disc structure will not result in a reset
 * of the same field in the destination disc structure; e.g. if there
 * is no title in the source disc, but there is one in the destination
 * disc, then the destination's title will remain unchanged.
 *
 * @param dst The destination CDDB disc structure.
 * @param src The source CDDB disc structure.
 */
void cddb_disc_copy(cddb_disc_t *dst, cddb_disc_t *src);

/**
 * Calculate the CDDB disc ID.
 *
 * @param disc The CDDB disc structure.
 */
int cddb_disc_calc_discid(cddb_disc_t *disc);

/**
 * Prints information about the disc on stdout.  This is just a
 * debugging routine to display the structure's content.
 *
 * @param disc The CDDB disc structure.
 */
void cddb_disc_print(cddb_disc_t *disc);


#ifdef __cplusplus
    }
#endif

#endif /* CDDB_DISC_H */
