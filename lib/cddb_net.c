/*
    $Id: cddb_net.c,v 1.6 2003/05/12 19:17:53 airborne Exp $

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

#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <setjmp.h>
#include <signal.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
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

/* Time-out enabled work-alikes */

/* time-out jump buffer */
static jmp_buf timeout_expired;

/* time-out signal handler */
static void alarm_handler(int signum)
{
    longjmp(timeout_expired, 1);
}

struct hostent *timeout_gethostbyname(const char *hostname, int timeout)
{
    struct hostent *he = NULL;
    struct sigaction action;
    struct sigaction old;

    /* no signal before setjmp */
    alarm(0);

    /* register signal handler */
    memset(&action, 0, sizeof(action));
    action.sa_handler = alarm_handler;
    sigaction(SIGALRM, &action, &old);

    /* save stack state */
    if (!setjmp(timeout_expired)) {
        alarm(timeout);         /* set time-out alarm */
        he = gethostbyname(hostname); /* execute DNS query */
        alarm(0);               /* reset alarm timer */
    } else {
        errno = ETIMEDOUT;
    }
    sigaction(SIGALRM, &old, NULL); /* restore previous signal handler */

    return he;
}

int timeout_connect(int sockfd, const struct sockaddr *addr, 
                    socklen_t len, int timeout)
{
    int got_error = 0;

    /* set socket to non-blocking */
#ifdef BEOS
    int on = 1;

    if (setsockopt(sockfd, SOL_SOCKET, SO_NONBLOCK, &on, sizeof(on)) == -1) {
        /* error while trying to set socket to non-blocking */
        return -1;
    }
#else
    int flags;

    flags = fcntl(sockfd, F_GETFL, 0);
    flags |= O_NONBLOCK;        /* add non-blocking flag */
    if (fcntl(sockfd, F_SETFL, flags) == -1) {
        return -1;
    }
#endif /* BEOS */

    /* try connecting */
    if (connect(sockfd, addr, len) == -1) {
        /* check whether we can continue */
        if (errno == EINPROGRESS) {
            int rv;
            fd_set wfds;
            struct timeval tv;
            socklen_t l;

            /* set up select time out */
            tv.tv_sec = timeout;
            tv.tv_usec = 0;

            /* set up file descriptor set */
            FD_ZERO(&wfds);
            FD_SET(sockfd, &wfds);

            /* wait for connect to finish */
            rv = select(sockfd + 1, NULL, &wfds, NULL, &tv);
            switch (rv) {
            case 0:             /* time out */
                errno = ETIMEDOUT;
            case -1:            /* select error */
                got_error = -1;
            default:
                /* we got connected, check error condition */
                l = sizeof(rv);
                getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &rv, &l);
                if (rv) {
                    /* something went wrong, simulate normal connect behaviour */
                    errno = rv;
                    got_error = -1;
                }
            }
        }
    } else {
        /* connect failed */
        got_error = -1;
    }
    return got_error;
}
