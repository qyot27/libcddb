

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "cddb/cddb_ni.h"


const char *CDDB_CATEGORY[CDDB_CAT_LAST] = {
    "data", "folk", "jazz", "misc", "rock", "country", "blues", "newage",
    "reggae", "classical", "soundtrack",
    "invalid"
};


/* --- construction / destruction */


cddb_disc_t *cddb_disc_new(void)
{
    cddb_disc_t *disc;

    disc = (cddb_disc_t*)calloc(1, sizeof(cddb_disc_t));
    disc->category = CDDB_CAT_INVALID;

    return disc;
}

void cddb_disc_destroy(cddb_disc_t *disc)
{
    cddb_track_t *track, *next;

    if (disc) {
        FREE_NOT_NULL(disc->title);
        FREE_NOT_NULL(disc->artist);
        FREE_NOT_NULL(disc->ext_data);
        track = disc->tracks;
        while (track) {
            next = track->next;
            cddb_track_destroy(track);
            track = next;
        }
        free(disc);
    }
}


/* --- track manipulation */


void cddb_disc_add_track(cddb_disc_t *disc, cddb_track_t *track)
{
    dlog("cddb_disc_add_track()");
    if (!disc->tracks) {
        /* first track on disc */
        disc->tracks = track;
    } else {
        /* next track on disc */
        cddb_track_t *t;

        t = disc->tracks;
        while (t->next) {
            t = t->next;
        }
        t->next = track;
    }
    disc->track_cnt++;
    track->num = disc->track_cnt;
}

cddb_track_t *cddb_disc_get_track(cddb_disc_t *disc, int track_no)
{
    cddb_track_t *track;
    
    if (track_no >= disc->track_cnt) {
        return NULL;
    }

    for (track = disc->tracks; 
         track_no > 0; 
         track_no--, track = track->next) { /* no-op */ }
    /* XXX: should we check track->num?? */
    return track;
}

cddb_track_t *cddb_disc_get_track_first(cddb_disc_t *disc)
{
    disc->iterator = disc->tracks;
    return disc->iterator;
}

cddb_track_t *cddb_disc_get_track_next(cddb_disc_t *disc)
{
    if (disc->iterator != NULL) {
        disc->iterator = disc->iterator->next;
    }
    return disc->iterator;
}

/* --- setters / getters --- */


void cddb_disc_set_category(cddb_disc_t *disc, const char *cat)
{
    int i;

    disc->category = CDDB_CAT_MISC;
    for (i = 0; i < CDDB_CAT_LAST; i++) {
        if (strcmp(cat, CDDB_CATEGORY[i]) == 0) {
            disc->category = i;
            return;
        }
    }
}

void cddb_disc_set_title(cddb_disc_t *disc, const char *title)
{
    if (disc) {
        FREE_NOT_NULL(disc->title);
        disc->title = strdup(title);
    }
}

void cddb_disc_set_artist(cddb_disc_t *disc, const char *artist)
{
    if (disc) {
        FREE_NOT_NULL(disc->artist);
        disc->artist = strdup(artist);
    }
}


/* --- miscellaneous */


int cddb_disc_calc_discid(cddb_disc_t *disc)
{
    long result = 0;
    long tmp;
    cddb_track_t *track, *first;

    dlog("cddb_disc_calc_discid()");
    for (first = track = cddb_disc_get_track_first(disc); 
         track != NULL; 
         track = cddb_disc_get_track_next(disc)) {
        tmp = track->frame_offset / 75;
        do {
            result += tmp % 10;
            tmp /= 10;
        } while (tmp != 0);
    }

    /* first byte is offsets of tracks
     * 2 next bytes total length in seconds
     * last byte is nr of tracks
     */
    disc->discid = (result % 0xff) << 24 | 
                   (disc->length - first->frame_offset/75) << 8 | 
                   disc->track_cnt;
    dlog("\tDisc ID: %08x", disc->discid);

    return TRUE;
}

void cddb_disc_print(cddb_disc_t *disc)
{
    cddb_track_t *track;
    int cnt;

    printf("Artist: '%s'\n", disc->artist);
    printf("Title: '%s'\n", disc->title);
    printf("Length: %d seconds\n", disc->length);
    printf("Number of tracks: %d\n", disc->track_cnt);
    track = disc->tracks;
    cnt = 1;
    while (track) {
        printf("  Track %2d\n", cnt);
        cddb_track_print(track);
        track = track->next;
        cnt++;
    }
}
