/*
    $Id: main.h,v 1.3 2003/04/17 22:19:11 airborne Exp $

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

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <cddb/cddb.h>


void error_exit(const char *fmt, ...);

cddb_disc_t *do_read(cddb_conn_t *conn, const char *category, int discid);

void do_display(cddb_disc_t *disc);

cddb_disc_t *cd_read(const char *device);
