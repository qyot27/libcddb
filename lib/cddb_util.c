/*
    $Id: cddb_util.c,v 1.3 2004/10/16 13:53:38 airborne Exp $

    Copyright (C) 2004 Kris Verbeeck <airborne@advalvas.be>

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

#include "cddb/cddb_ni.h"

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif


int cddb_str_iconv(iconv_t cd, char *in, char **out)
{
#ifdef HAVE_ICONV_H
    int inlen, outlen, buflen, rc;
    int len;                    /* number of chars in buffer */
    char *buf;

    inlen = strlen(in);
    buflen = 0;
    buf = NULL;
    do {
        outlen = inlen * 2;
        buflen += outlen;
        /* iconv() below changes the buf pointer:
         * - decrement to point at beginning of buffer before realloc
         * - re-increment to point at first free position after realloc
         */
        len = buflen - outlen;
        buf = (char*)realloc(buf - len, buflen) + len;
        if (buf == NULL) {
            /* XXX: report out of memory error */
            return FALSE;
        }
        rc = iconv(cd, &in, &inlen, &buf, &outlen);
        if ((rc == -1) && (errno != E2BIG)) {
            free(buf);
            return FALSE;       /* conversion failed */
        }
    } while (inlen != 0);
    len = buflen - outlen;
    buf -= len;                 /* reposition at begin of buffer */
    /* make a copy just big enough for the result */
    *out = malloc(len + 1);
    memcpy(*out, buf, len);
    *(*out + len) = '\0';
    free(buf);
#endif
    return TRUE;
}
