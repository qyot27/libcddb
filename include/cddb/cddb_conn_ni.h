/*
    $Id: cddb_conn_ni.h,v 1.6 2003/05/20 20:42:02 airborne Exp $

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

#ifndef CDDB_CONN_NI_H
#define CDDB_CONN_NI_H 1

#ifdef __cplusplus
    extern "C" {
#endif


/* --- getters & setters --- */


/**
 */
#define cddb_cache_file(c) (c)->cache_fp


/* --- connecting / disconnecting --- */


/**
 */
int cddb_connect(cddb_conn_t *c);

/**
 */
void cddb_disconnect(cddb_conn_t *c);


/* --- error handling --- */


/**
 * Set the error number for the last libcddb command.
 *
 * @param c The CDDB connection structure.
 * @param n The error number
 */
#define cddb_errno_set(c, n) (c)->errnum = n

/**
 * Set the error number for the last libcddb command.  If this number
 * is different from CDDB_ERR_OK, a message is also logged with the
 * level specified.
 *
 * @param c The CDDB connection structure.
 * @param n The error number
 * @param l The log level
 */
#define cddb_errno_log(c, n, l) cddb_errno_set(c, n); cddb_log(l, cddb_error_str(n))

#define cddb_errno_log_debug(c, n) cddb_errno_log(c, n, CDDB_LOG_DEBUG)
#define cddb_errno_log_info(c, n) cddb_errno_log(c, n, CDDB_LOG_INFO)
#define cddb_errno_log_warn(c, n) cddb_errno_log(c, n, CDDB_LOG_WARN)
#define cddb_errno_log_error(c, n) cddb_errno_log(c, n, CDDB_LOG_ERROR)
#define cddb_errno_log_crit(c, n) cddb_errno_log(c, n, CDDB_LOG_CRITICAL)


#ifdef __cplusplus
    }
#endif

#endif /* CDDB_CONN_NI_H */
