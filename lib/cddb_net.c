/*
    $Id: cddb_net.c,v 1.9 2003/05/20 20:47:29 airborne Exp $

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
#include <time.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "cddb/cddb_ni.h"


/* Utility functions */


/**
 * Checks whether bytes can be read/written from/to the socket within
 * the specified time out period.
 *
 * @param sock     The socket to read from.
 * @param timeout  Number of seconds after which to time out.
 * @param to_write TRUE if we have to check for writing, FALSE for
 *                 reading.
 * @return TRUE if reading/writing is possible, FALSE otherwise.
 */
static int sock_ready(int sock, int timeout, int to_write)
{
    fd_set fds;
    struct timeval tv;
    int rv;

    cddb_log_debug("sock_ready()");
    /* set up select time out */
    tv.tv_sec = timeout;
    tv.tv_usec = 0;
    /* set up file descriptor set */
    FD_ZERO(&fds);
    FD_SET(sock, &fds);
    /* wait for data to become available */
    if (to_write) {
        rv = select(sock + 1, NULL, &fds, NULL, &tv) ;
    } else {
        rv = select(sock + 1, &fds, NULL, NULL, &tv) ;
    }
    if (rv <= 0) {
        if (rv == 0) {
            errno = ETIMEDOUT;
        }
        return FALSE;
    }
    return TRUE;
}
#define sock_can_read(s,t) sock_ready(s, t, FALSE)
#define sock_can_write(s,t) sock_ready(s, t, TRUE)


/* Socket-based work-alikes */


char *sock_fgets(char *s, int size, int sock, int timeout)
{
    int rv;
    time_t now, end;
    char *p = s;

    cddb_log_debug("sock_fgets()");
    end = time(NULL) + timeout;
    size--;                      /* save one for terminating null */
    while (size) {
        now = time(NULL);
        timeout = end - now;
        if (timeout <= 0) {
            errno = ETIMEDOUT;
            return NULL;        /* time out */
        }
        /* can we read from the socket? */
        if (!sock_can_read(sock, timeout)) {
            /* error or time out */
            return NULL;
        }
        /* read one byte */
        rv = recv(sock, p, 1, 0);
        if (rv == -1) {
            /* recv() error */
            return NULL;
        } else if (rv == 0) {
            /* EOS reached */
            break;
        } else if (*p == CHR_LF) {
            /* EOL reached, stop reading */
            p++;
            break;
        }
        p++;
        size--;
    }
    if (p == s) {
        cddb_log_debug("...read = Empty");
        return NULL;
    }
    *p = CHR_EOS;
    cddb_log_debug("...read = '%s'", s);
    return s;
}

size_t sock_fwrite(const void *ptr, size_t size, size_t nmemb, int sock, int timeout)
{
    size_t total_size, to_send;
    time_t now, end;
    int rv;
    const char *p = (const char *)ptr;

    cddb_log_debug("sock_fwrite()");
    total_size = size * nmemb;
    to_send = total_size;
    end = time(NULL) + timeout;
    while (to_send) {
        now = time(NULL);
        timeout = end - now;
        if (timeout <= 0) {
            /* time out */
            errno = ETIMEDOUT;
            break;
        }
        /* can we write to the socket? */
        if (!sock_can_write(sock, timeout)) {
            /* error or time out */
            break;
        }
        /* try sending data */
        rv = send(sock, p, to_send, 0);
        if (rv == -1 && errno != EAGAIN && errno != EWOULDBLOCK) {
            /* error */
            break;
        } else {
            to_send -= rv;
            p += rv;
        }
    }
    return (total_size - to_send) / size;
}

int sock_fprintf(int sock, int timeout, const char *format, ...)
{
    int rv;
    va_list args;

    cddb_log_debug("sock_fprintf()");
    va_start(args, format);
    rv = sock_vfprintf(sock, timeout, format, args);
    va_end(args);
    return rv;
}

int sock_vfprintf(int sock, int timeout, const char *format, va_list ap)
{
    char buf[1024];
    int rv;
   
    cddb_log_debug("sock_vfprintf()");
    rv = vsnprintf(buf, sizeof(buf), format, ap);
    cddb_log_debug("...buf = '%s'", buf);
    if (rv >= sizeof(buf)) {
        /* buffer too small */
        cddb_log_crit("internal sock_vfprintf buffer too small");
        return -1;
    }

    rv = sock_fwrite(buf, sizeof(char), rv, sock, timeout);
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
