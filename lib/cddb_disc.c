

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
        FREE_NOT_NULL(disc->genre);
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

cddb_disc_t *cddb_disc_clone(cddb_disc_t *disc)
{
    cddb_disc_t *clone;
    cddb_track_t *track;

    dlog("cddb_disc_clone()");
    clone = cddb_disc_new();
    clone->discid = disc->discid;
    clone->category = disc->category;
    clone->year = disc->year;
    clone->genre = (disc->genre ? strdup(disc->genre) : NULL);
    clone->title = (disc->title ? strdup(disc->title) : NULL);
    clone->artist = (disc->artist ? strdup(disc->artist) : NULL);
    clone->length = disc->length;
    clone->ext_data = (disc->ext_data ? strdup(disc->ext_data) : NULL);
    /* clone the tracks */
    track = disc->tracks;
    while (track) {
        cddb_disc_add_track(clone, cddb_track_clone(track));
        track = track->next;
    }
    return clone;
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
    track->disc = disc;
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

    FREE_NOT_NULL(disc->genre);
    disc->genre = strdup(cat);
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

void cddb_disc_append_title(cddb_disc_t *disc, const char *title)
{
    int old_len = 0, len;

    if (disc && title) {
        /* only append if there is something to append */
        if (disc->title) {
            old_len = strlen(disc->title);
        }
        len = strlen(title);
        disc->title = realloc(disc->title, old_len+len+1);
        strcpy(disc->title+old_len, title);
        disc->title[old_len+len] = '\0';
    }
}

void cddb_disc_set_artist(cddb_disc_t *disc, const char *artist)
{
    if (disc) {
        FREE_NOT_NULL(disc->artist);
        disc->artist = strdup(artist);
    }
}

void cddb_disc_append_artist(cddb_disc_t *disc, const char *artist)
{
    int old_len = 0, len;

    if (disc && artist) {
        /* only append if there is something to append */
        if (disc->artist) {
            old_len = strlen(disc->artist);
        }
        len = strlen(artist);
        disc->artist = realloc(disc->artist, old_len+len+1);
        strcpy(disc->artist+old_len, artist);
        disc->artist[old_len+len] = '\0';
    }
}


/* --- miscellaneous */


void cddb_disc_copy(cddb_disc_t *dst, cddb_disc_t *src)
{
    cddb_track_t *src_track, *dst_track;

    dlog("cddb_disc_copy()");
    if (src->discid != 0) {
        dst->discid = src->discid;
    }
    if (src->category != CDDB_CAT_INVALID) {
        dst->category = src->category;
    }
    if (src->year != 0) {
        dst->year = src->year;
    }
    if (src->genre != NULL) {
        FREE_NOT_NULL(dst->genre);
        dst->genre = strdup(src->genre);
    }
    if (src->title != NULL) {
        FREE_NOT_NULL(dst->title);
        dst->title = strdup(src->title);
    }
    if (src->artist) {
        FREE_NOT_NULL(dst->artist);
        dst->artist = strdup(src->artist);
    }
    if (src->length != 0) {
        dst->length = src->length;
    }
    if (src->ext_data != NULL) {
        FREE_NOT_NULL(dst->ext_data);
        dst->ext_data = strdup(src->ext_data);
    }
    /* copy the tracks */
    src_track = src->tracks;
    dst_track = dst->tracks;
    while (src_track) {
        if (dst_track == NULL) {
            dst_track = cddb_track_new();
            cddb_disc_add_track(dst, dst_track);
        }
        cddb_track_copy(dst_track, src_track);
        src_track = src_track->next;
        dst_track = dst_track->next;
    }
}

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

    printf("Disc ID: %08x\n", disc->discid);
    printf("CDDB category: %s (%d)\n", CDDB_CATEGORY[disc->category], disc->category);
    printf("Music genre: '%s'\n", STR_OR_NULL(disc->genre));
    printf("Year: %d\n", disc->year);
    printf("Artist: '%s'\n", STR_OR_NULL(disc->artist));
    printf("Title: '%s'\n", STR_OR_NULL(disc->title));
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
