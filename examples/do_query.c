/*
    $Id: do_query.c,v 1.1 2003/04/20 15:04:08 airborne Exp $

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

#include "main.h"


void do_query(cddb_conn_t *conn, cddb_disc_t *disc)
{
    int matches;

    /* Try querying the database for information about the provided
       disc.  This function will return the number of matches that
       were found.  A return value of 0 means that no matches were
       found.  The data of the first match (when found) will be filled
       in into the disc structure passed to it.  The information will
       be retrieved from the server or read from the cache depending
       on the connection settings. */
    matches = cddb_query(conn, disc);

    /* If an error occured then the return value will be -1 and the
       internal libcddb error number will be set. */
    if (matches == -1) {
        /* Print an explanatory message on stderr.  Other routines are
           available for retrieving the message without printing it or
           printing it on a stream other than stderr. */
        cddb_error_print(cddb_errno(conn));
        /* Return to calling fucntion. */
        return;
    }

    printf("Number of matches: %d\n", matches);
    /* XXX: print category, disc ID, disc title and artist name */
}
