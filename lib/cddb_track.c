

#include <stdlib.h>
#include "cddb/cddb_ni.h"


cddb_track_t *cddb_track_new(void)
{
    cddb_track_t *track;

    track = (cddb_track_t*)calloc(1, sizeof(cddb_track_t));
    track->frame_offset = -1;

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

void cddb_track_print(cddb_track_t *track)
{
    printf("    frame offset: %d\n", track->frame_offset);
    if (track->artist) printf("    artist: '%s'\n", track->artist);
    if (track->title) printf("    title: '%s'\n", track->title);
}
