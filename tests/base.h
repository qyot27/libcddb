/*
    $Id: base.h,v 1.2 2003/04/14 21:27:16 airborne Exp $

    Copyright (C) 2003 Kris Verbeeck <airborne@advalvas.be>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef CDDB_CHECK_BASE_H
#define CDDB_CHECK_BASE_H 1

#ifdef __cplusplus
    extern "C" {
#endif


#include <check.h>
#include <cddb/cddb.h>


/* --- global settings --- */


#define NUM_OFFSETS 16
extern const int DISC_OFFSET[NUM_OFFSETS];

cddb_disc_t *empty_disc;
cddb_disc_t *real_disc;

/**
 * Create a series of discs.  This routine can be used in an
 * individual test if it needs some pre-initialized discs.
 */
void setup_discs(void);

/**
 * Destroy the discs created during setup.  When using the setup
 * routine, also make sure to use this tear down routine to free the
 * memory resources used by the discs.
 */
void teardown_discs(void);

/**
 * Function called to retrieve the test suite.
 */
extern Suite *get_suite(void);


#ifdef __cplusplus
    }
#endif

#endif /* CDDB_CHECK_BASE_H */
