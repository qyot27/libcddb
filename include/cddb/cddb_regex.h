/*
    $Id: cddb_regex.h,v 1.7 2003/05/23 21:11:30 airborne Exp $

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

#ifndef CDDB_REGEX_H
#define CDDB_REGEX_H 1

#ifdef __cplusplus
    extern "C" {
#endif


#include <stdlib.h>
#include <regex.h>


/**
 */
extern regex_t *REGEX_TRACK_FRAME_OFFSETS;
extern regex_t *REGEX_TRACK_FRAME_OFFSET;
extern regex_t *REGEX_DISC_LENGTH;
extern regex_t *REGEX_DISC_TITLE;
extern regex_t *REGEX_DISC_YEAR;
extern regex_t *REGEX_DISC_GENRE;
extern regex_t *REGEX_DISC_EXT;
extern regex_t *REGEX_TRACK_TITLE;
extern regex_t *REGEX_TRACK_EXT;
extern regex_t *REGEX_PLAY_ORDER;
extern regex_t *REGEX_QUERY_MATCH;


/**
 */
void cddb_regex_init(void);

/**
 */
int cddb_regex_get_int(const char *s, regmatch_t matches[], int idx);

/**
 */
char *cddb_regex_get_string(const char *s, regmatch_t matches[], int idx);


#ifdef __cplusplus
    }
#endif

#endif /* CDDB_REGEX_H */
