/*
    $Id: cddb_log.h,v 1.1 2003/05/20 20:37:01 airborne Exp $

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

#ifndef CDDB_LOH_H
#define CDDB_LOG_H

#ifdef __cplusplus
    extern "C" {
#endif


typedef enum {
    CDDB_LOG_DEBUG = 1,
    CDDB_LOG_INFO,
    CDDB_LOG_WARN,
    CDDB_LOG_ERROR,
    CDDB_LOG_CRITICAL,
    CDDB_LOG_NONE = 99
} cddb_log_level_t;

    
typedef void (*cddb_log_handler_t)(cddb_log_level_t level, const char *message);

cddb_log_handler_t cddb_log_set_handler(cddb_log_handler_t new_handler);


void cddb_log_set_level(cddb_log_level_t level);

void cddb_log(cddb_log_level_t level, const char *format, ...);

#define cddb_log_debug(...) cddb_log(CDDB_LOG_DEBUG, __VA_ARGS__)

#define cddb_log_info(...) cddb_log(CDDB_LOG_INFO, __VA_ARGS__)

#define cddb_log_warn(...) cddb_log(CDDB_LOG_WARN, __VA_ARGS__)

#define cddb_log_error(...) cddb_log(CDDB_LOG_ERROR, __VA_ARGS__)

#define cddb_log_crit(...) cddb_log(CDDB_LOG_CRITICAL, __VA_ARGS__)


#ifdef __cplusplus
    }
#endif

#endif /* CDDB_LOG_H */
