/*
    $Id: cddb_net.h,v 1.3 2003/04/14 22:25:49 airborne Exp $

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

#ifndef CDDB_NET_H
#define CDDB_NET_H 1

#ifdef __cplusplus
    extern "C" {
#endif


#include <stdarg.h>


/* --- socket-based work-alikes --- */

char *sock_fgets(char *s, int size, int sock);

size_t sock_fwrite(const void *ptr, size_t size, size_t nmemb, int sock);

int sock_fprintf(int sock, const char *format, ...);

int sock_vfprintf(int sock, const char *format, va_list ap);


#ifdef __cplusplus
    }
#endif

#endif /* CDDB_NET_H */
