/*
    $Id: cddb_track.c,v 1.10 2003/04/17 17:28:41 airborne Exp $

    Copyright (C) 2003 Kris Verbeeck <airborne@advalvas.be>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the
    Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA  02111-1307, USA.
*/

#include <stdlib.h>
#include <string.h>
#include "cddb/cddb_ni.h"


/* --- construction / destruction */


cddb_track_t *cddb_track_new(void)
{
    cddb_track_t *track;

    track = (cddb_track_t*)calloc(1, sizeof(cddb_track_t));
    if (track) {
        track->num = -1;
        track->frame_offset = -1;
        track->length = -1;
        track->disc = NULL;
    }

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
    clone->disc = NULL;
    return clone;
}


/* --- getters & setters --- */


void cddb_track_set_title(cddb_track_t *track, const char *title)
{
    if (track) {
        FREE_NOT_NULL(track->title);
        if (title) {
            track->title = strdup(title);
        }
    }
}

int cddb_track_get_length(cddb_track_t *track)
{
    cddb_track_t *next;
    int start, end;

    if (track) {
        if (track->length == -1) {
            start = track->frame_offset;
            next = track->next;
            if (next != NULL) {
                /* not last track on disc, use frame offset of next track */
                end = next->frame_offset;
                if (end > start) {
                    /* XXX: rounding errors */
                    track->length = (end - start) / FRAMES_PER_SECOND;
                }
            } else {
                /* last track on disc, use disc length */
                if (track->disc != NULL) {
                    /* XXX: rounding errors */
                    start /= FRAMES_PER_SECOND;
                    end = cddb_disc_get_length(track->disc);
                    if (end > start) {
                        track->length = end - start;
                    }
                }
            }
        }
        return track->length;
    }
    return -1;
}

void cddb_track_append_title(cddb_track_t *track, const char *title)
{
    int old_len = 0, len;

    if (track && title) {
        /* only append if there is something to append */
        if (track->title) {
            old_len = strlen(track->title);
        }
        len = strlen(title);
        track->title = realloc(track->title, old_len+len+1);
        strcpy(track->title+old_len, title);
        track->title[old_len+len] = '\0';
    }
}

const char *cddb_track_get_artist(cddb_track_t *track)
{
    const char *artist = NULL;

    if (track) {
        if (track->artist) {
            artist = track->artist;
        } else {
            artist =  track->disc->artist; /* might be NULL */
        }
    }
    return artist;
}

void cddb_track_set_artist(cddb_track_t *track, const char *artist)
{
    if (track) {
        FREE_NOT_NULL(track->artist);
        if (artist) {
            track->artist = strdup(artist);
        }
    }
}

void cddb_track_append_artist(cddb_track_t *track, const char *artist)
{
    int old_len = 0, len;

    if (track && artist) {
        /* only append if there is something to append */
        if (track->artist) {
            old_len = strlen(track->artist);
        }
        len = strlen(artist);
        track->artist = realloc(track->artist, old_len+len+1);
        strcpy(track->artist+old_len, artist);
        track->artist[old_len+len] = '\0';
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
    printf("    length: %d seconds\n", cddb_track_get_length(track));
    printf("    artist: '%s'\n", STR_OR_NULL(cddb_track_get_artist(track)));
    printf("    title: '%s'\n", STR_OR_NULL(track->title));
}
