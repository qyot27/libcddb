

#include <stdlib.h>
#include <string.h>
#include "cddb/cddb_ni.h"


/* --- construction / destruction */


cddb_track_t *cddb_track_new(void)
{
    cddb_track_t *track;

    track = (cddb_track_t*)calloc(1, sizeof(cddb_track_t));
    track->num = -1;
    track->frame_offset = -1;
    track->length = -1;

    return track;
}

void cddb_track_destroy(cddb_track_t *track)
{
    if (track) {
        FREE_NOT_NULL(track->title);
        FREE_NOT_NULL(track->artist);
        FREE_NOT_NULL(track->ext_data);
        free(track);
    }
}


/* --- getters & setters --- */


void cddb_track_set_title(cddb_track_t *track, const char *title)
{
    if (track) {
        FREE_NOT_NULL(track->title);
        track->title = strdup(title);
    }
}

void cddb_track_set_artist(cddb_track_t *track, const char *artist)
{
    if (track) {
        FREE_NOT_NULL(track->artist);
        track->artist = strdup(artist);
    }
}


/* --- miscellaneous */


void cddb_track_print(cddb_track_t *track)
{
    printf("    number: %d\n", track->num);
    printf("    frame offset: %d\n", track->frame_offset);
    printf("    length: %d seconds\n", track->length);
    if (track->artist) printf("    artist: '%s'\n", track->artist);
    if (track->title) printf("    title: '%s'\n", track->title);
}
