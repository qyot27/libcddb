/*
    $Id: debug.h,v 1.3 2003/04/14 21:27:14 airborne Exp $

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

#ifndef CDDB_DEBUG_H
#define CDDB_DEBUG_H 1

#ifdef __cplusplus
    extern "C" {
#endif


#ifdef DEBUG

#    define dlog(...) fprintf(stderr, "%s(%d): ", __FILE__, __LINE__); \
                      fprintf(stderr, __VA_ARGS__); \
                      fprintf(stderr,"\n")

#else

#    define dlog(...)

#endif


#ifdef __cplusplus
    }
#endif

#endif /* CDDB_DEBUG_H */
