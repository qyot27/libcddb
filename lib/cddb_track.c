

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

cddb_track_t *cddb_track_clone(cddb_track_t *track)
{
    cddb_track_t *clone;

    dlog("cddb_track_clone()");
    clone = cddb_track_new();
    clone->num = track->num;
    clone->frame_offset = track->frame_offset;
    clone->length = track->length;
    clone->title = (track->title ? strdup(track->title) : NULL);
    clone->artist = (track->artist ? strdup(track->artist) : NULL);
    clone->ext_data = (track->ext_data ? strdup(track->ext_data) : NULL);
    return clone;
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


void cddb_track_copy(cddb_track_t *dst, cddb_track_t *src)
{
    dlog("cddb_track_copy()");
    if (src->num != -1) {
        dst->num = src->num;
    }
    if (src->frame_offset != -1) {
        dst->frame_offset = src->frame_offset;
    }
    if (src->length != -1) {
        dst->length = src->length;
    }
    if (src->title != NULL) {
        FREE_NOT_NULL(dst->title);
        dst->title = strdup(src->title);
    }
    if (src->artist) {
        FREE_NOT_NULL(dst->artist);
        dst->artist = strdup(src->artist);
    }
    if (src->ext_data != NULL) {
        FREE_NOT_NULL(dst->ext_data);
        dst->ext_data = strdup(src->ext_data);
    }
}

void cddb_track_print(cddb_track_t *track)
{
    printf("    number: %d\n", track->num);
    printf("    frame offset: %d\n", track->frame_offset);
    printf("    length: %d seconds\n", track->length);
    printf("    artist: '%s'\n", STR_OR_NULL(track->artist));
    printf("    title: '%s'\n", STR_OR_NULL(track->title));
}
