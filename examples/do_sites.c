/*
    $Id: do_sites.c,v 1.1 2005/05/29 08:24:03 airborne Exp $

    Copyright (C) 2005 Kris Verbeeck <airborne@advalvas.be>

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

#include "main.h"

#define STR_OR_NULL(s) ((s) ? s : "(null)")

void do_sites(cddb_conn_t *conn)
{
    int idx = 0;
    cddb_site_t *site = NULL;   /* libcddb site structure */
    float latitude, longitude;
    const char *address, *path, *desc;
    unsigned int port;

    /* Instruct libcddb to query the active server for a list of
       mirror sites. */
    if (!cddb_sites(conn)) {
        error_exit(cddb_errno(conn), "could not read sites data");
    }

    /* XXX: more comments */    
    site = cddb_first_site(conn);
    if (site) {
        do {
            /* XXX: error handling */
            cddb_site_get_address(site, &address, &port);
            cddb_site_get_query_path(site, &path);
            cddb_site_get_description(site, &desc);
            cddb_site_get_location(site, &latitude, &longitude);
            idx++;
            printf("Mirror %d\n", idx);
            printf("  address:     ");
            if (cddb_site_get_protocol(site) == PROTO_HTTP) { 
                printf("http://%s:%d%s\n", address, port, path);
            } else {
                printf("%s:%d\n", address, port);
            }
            printf("  description: %s\n", desc);
            printf("  location:    %-7.2f %-7.2f\n", latitude, longitude);
            site = cddb_next_site(conn);
        } while (site);
    }
}
