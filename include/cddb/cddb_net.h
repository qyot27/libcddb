/*
    $Id: cddb_net.h,v 1.4 2003/05/12 18:47:57 airborne Exp $

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
#include <sys/socket.h>


/* --- socket-based work-alikes --- */

char *sock_fgets(char *s, int size, int sock);

size_t sock_fwrite(const void *ptr, size_t size, size_t nmemb, int sock);

int sock_fprintf(int sock, const char *format, ...);

int sock_vfprintf(int sock, const char *format, va_list ap);

/* --- time-out enabled work-alikes --- */

/**
 * This function performs the same task as the standard gethostbyname
 * except for the fact that it might time-out if the query takes too
 * long.  In case of a time out, errno will be set to ETIMEDOUT.
 *
 * @param hostname The hostname that needs to be resolved.
 * @param timeout  Number of seconds after which to time out.
 * @return The host entity for given host name or NULL if not found or
 *         timed out (errno will be set).
 */
struct hostent *timeout_gethostbyname(const char *hostname, int timeout);

/**
 * This function performs the same task as the standard connect except
 * for the fact that it might time-out if the connect takes too long.
 * In case of a time out, errno will be set to ETIMEDOUT.
 * 
 * @param sockfd   The socket.
 * @param addr     The address to connect to.
 * @param len      The size of the address structure.
 * @param timeout  Number of seconds after which to time out.
 * @return Zero on success, -1 on failure (errno will be set).
 */
int timeout_connect(int sockfd, const struct sockaddr *addr, socklen_t len, int timeout);


#ifdef __cplusplus
    }
#endif

#endif /* CDDB_NET_H */
