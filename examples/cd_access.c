/*
    $Id: cd_access.c,v 1.5 2003/04/21 17:17:29 airborne Exp $

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
#include "main.h"

#ifdef HAVE_LIBCDIO
#include <cdio/cdio.h>
#endif

#define libcdio_error_exit(...) error_exit(GENERIC_ERROR, "libcdio: " __VA_ARGS__)


cddb_disc_t *cd_read(char *device)
{
    cddb_disc_t *disc = NULL;   /* libcddb disc structure */
    
#ifndef HAVE_LIBCDIO

    fprintf(stderr, "*** libcdio not installed, CD access not supported   ***\n");
    fprintf(stderr, "*** see http://savannah.nongnu.org/projects/libcdio/ ***\n");

#else

    CdIo *cdio;                 /* libcdio CD access structure */
    track_t cnt, t;             /* track counters */
    lsn_t lsn;                  /* Logical Sector Number */
    int *foffset = NULL;        /* list of frame offsets */

    /* Get the name of the default CD-ROM device. */
    if (!device) {
        device = cdio_get_default_device(NULL);
        if (!device) {
            libcdio_error_exit("unable to get default CD device");
        }
    }
    printf("CD-ROM device: %s\n", device);

    /* Load the appropriate driver and open the CD-ROM device for reading. */
    cdio = cdio_open(device, DRIVER_UNKNOWN);
    if (!cdio) {
        libcdio_error_exit("unable to open CD device");
    }

    /* Get the track count for the CD. */
    cnt = cdio_get_num_tracks(cdio);
    if (cnt == 0) {
        libcdio_error_exit("no audio tracks on CD");
    }
    printf("CD contains %d track(s)\n", cnt);

    /* Reserve some memory for the frame offsets. */
    foffset = calloc(cnt, sizeof(int));
    
    /* Now we go and fetch the track data. */
    for (t = 1; t <= cnt; t++) {

        /* We only want to process audio CDs. */
        if (cdio_get_track_format(cdio, t) != TRACK_FORMAT_AUDIO) {
            libcdio_error_exit("track %d is not an audio track");
        }

        /* Get frame offset of next track. */
        lsn = cdio_get_track_lsn(cdio, t);
        if (lsn == CDIO_INVALID_LSN) {
            libcdio_error_exit("track %d has invalid Logical Sector Number", t);
        }

        /* Add this offset to teh list.  We have to make sure that we
           add two seconds of lead-in.*/
        foffset[t - 1] = lsn + 2 * FRAMES_PER_SECOND;
    }

    /* Now all we still have to do, is calculate the length of the
       disc in seconds.  We use the LEADOUT_TRACK for this. */
    lsn = cdio_get_track_lsn(cdio, CDIO_CDROM_LEADOUT_TRACK);
    if (lsn == CDIO_INVALID_LSN) {
        libcdio_error_exit("LEADOUT_TRACK has invalid Logical Sector Number");
    }

    /* Now we have to create the libcddb disc structure. */
    disc = cd_create(FRAMES_TO_SECONDS(lsn), cnt, foffset);

    /* Free all resources held by libcdio CD access structure. */
    cdio_destroy(cdio);

    /* more clean up */
    FREE_NOT_NULL(foffset);
    FREE_NOT_NULL(device);

#endif

    return disc;
}

cddb_disc_t *cd_create(int dlength, int tcount, int *foffset)
{
    int i;
    cddb_disc_t *disc;
    cddb_track_t *track;

    /* Create a new disc structure. */
    disc = cddb_disc_new();

    /* If the pointer is NULL then an error occured (out of memory).
       Otherwise we continue. */
    if (disc) { 
        /* Initialize the disc length in the structure. */
        cddb_disc_set_length(disc, dlength);

        /* Now we have to add the basic track data. */
        for (i = 0; i < tcount; i++) {
            /* Create a new libcddb track structure for this track. */
            track = cddb_track_new();

            /* If the pointer is NULL then an error occured (out of
               memory).  Otherwise we continue. */
            if (!track) { 
                /* Destroy the disc because we can not return half of
                   it.  Return NULL to signal failure. */
                cddb_disc_destroy(disc);
                return NULL;
            }

            /* Set frame offset in track structure. */
            cddb_track_set_frame_offset(track, foffset[i]);

            /* And add the track to the disc. */
            cddb_disc_add_track(disc, track);
        }
    }
    return disc;
}
