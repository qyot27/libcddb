/*
    $Id: cddb_net.c,v 1.4 2003/04/14 22:25:50 airborne Exp $

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

#include <sys/socket.h>
#include "cddb/cddb_ni.h"


/* Socket-based work-alikes */

char *sock_fgets(char *s, int size, int sock)
{
    char *p = s;

    dlog("sock_fgets()");
    size--;                      /* save one for terminating null */
    while (size) {
        /* read one byte */
        if (recv(sock, p, 1, 0) != 1) {
            /* EOF or error */
            break;
        }
        /* XXX: how should we handle CRLF or CR? */
        if (*p == CHR_LF) {
            /* EOL reached, stop reading */
            p++;
            break;
        }
        p++;
        size--;
    }
    if (p == s) {
        dlog("\tread = Empty");
        return NULL;
    }
    *p = CHR_EOS;
    dlog("\tread = '%s'", s);
    return s;
}

size_t sock_fwrite(const void *ptr, size_t size, size_t nmemb, int sock)
{
    size_t total_size;
    int rv;

    dlog("sock_fwrite()");
    total_size = size * nmemb;
    rv = send(sock, (const char *)ptr, total_size, 0);
    return rv/size;
}

int sock_fprintf(int sock, const char *format, ...)
{
    int rv;
    va_list args;

    dlog("sock_fprintf()");
    va_start(args, format);
    rv = sock_vfprintf(sock, format, args);
    va_end(args);
    return rv;
}

int sock_vfprintf(int sock, const char *format, va_list ap)
{
    char buf[1024];
    int rv;
   
    dlog("sock_vfprintf()");
    rv = vsnprintf(buf, sizeof(buf), format, ap);
    dlog("\tbuf = '%s'", buf);
    if (rv >= sizeof(buf)) {
        /* buffer too small */
        dlog("\t!!! buffer too small");
        return -1;
    }

    rv = send(sock, buf, rv, 0);
    return rv;
}
