
#include <stdio.h>
#include <string.h>

#include <cddb/cddb.h>


int do_read(cddb_conn_t *c, cddb_disc_t *d)
{
    d->discid = 0xfe116410;
    d->category = CDDB_CAT_ROCK;
    return cddb_read(c, d);
}

int do_query(cddb_conn_t *c, cddb_disc_t *d)
{
    cddb_track_t *t = NULL;

    d->discid = 0xfe116410;
    d->length = 4454;
    t = cddb_track_new();       /* track  1 */
    t->frame_offset = 150;
    cddb_disc_add_track(d, t);
    t = cddb_track_new();       /* track  2 */
    t->frame_offset = 23627;
    cddb_disc_add_track(d, t);
    t = cddb_track_new();       /* track  3 */
    t->frame_offset = 44723;
    cddb_disc_add_track(d, t);
    t = cddb_track_new();       /* track  4 */
    t->frame_offset = 60527;
    cddb_disc_add_track(d, t);
    t = cddb_track_new();       /* track  5 */
    t->frame_offset = 91311;
    cddb_disc_add_track(d, t);
    t = cddb_track_new();       /* track  6 */
    t->frame_offset = 126360;
    cddb_disc_add_track(d, t);
    t = cddb_track_new();       /* track  7 */
    t->frame_offset = 144562;
    cddb_disc_add_track(d, t);
    t = cddb_track_new();       /* track  8 */
    t->frame_offset = 160236;
    cddb_disc_add_track(d, t);
    t = cddb_track_new();       /* track  9 */
    t->frame_offset = 174360;
    cddb_disc_add_track(d, t);
    t = cddb_track_new();       /* track 10 */
    t->frame_offset = 194624;
    cddb_disc_add_track(d, t);
    t = cddb_track_new();       /* track 11 */
    t->frame_offset = 217273;
    cddb_disc_add_track(d, t);
    t = cddb_track_new();       /* track 12 */
    t->frame_offset = 233402;
    cddb_disc_add_track(d, t);
    t = cddb_track_new();       /* track 13 */
    t->frame_offset = 259181;
    cddb_disc_add_track(d, t);
    t = cddb_track_new();       /* track 14 */
    t->frame_offset = 276582;
    cddb_disc_add_track(d, t);
    t = cddb_track_new();       /* track 15 */
    t->frame_offset = 291679;
    cddb_disc_add_track(d, t);
    t = cddb_track_new();       /* track 16 */
    t->frame_offset = 314363;
    cddb_disc_add_track(d, t);

    return cddb_query(c, d);
}

int do_write(cddb_conn_t *c, cddb_disc_t *d)
{
    cddb_track_t *t = NULL;

    d->category = CDDB_CAT_JAZZ;
    d->length = 3600;
    d->title = strdup("Test 1");
    d->artist = strdup("Test 2");
    t = cddb_track_new();       /* track  1 */
    t->frame_offset = 150;
    t->title = strdup("Test 3");
    cddb_disc_add_track(d, t);
    t = cddb_track_new();       /* track  2 */
    t->frame_offset = 23627;
    t->title = strdup("Test 4");
    t->artist = strdup("Test 5");
    cddb_disc_add_track(d, t);
    cddb_disc_calc_discid(d);
    
    return cddb_write(c, d);
}

int main(int argc, char **argv)
{
    cddb_conn_t *c = NULL;
    cddb_disc_t *d = NULL;

    c = cddb_new();
    if (c) {
        cddb_set_email_address(c, "me@home");
        cddb_set_server_name(c, "my.server.com");
#if 0
        cddb_set_server_port(c, 8880);
        cddb_http_enable(c, 0);
#else
        cddb_set_server_port(c, 80);
        cddb_http_enable(c, 1);
        cddb_set_http_path_query(c, "/~me/cddbd.cgi");
        cddb_set_http_path_submit(c, "/~me/submit.cgi");
#   if 0
        cddb_http_proxy_enable(c, 1);
        cddb_set_http_proxy_server_name(c, "my.proxy.com");
        cddb_set_http_proxy_server_port(c, 3128);
#   endif
#endif
        d = cddb_disc_new();
        if (d) {
            int rv;

            rv = do_read(c, d);
            // rv = do_query(c, d);
            // rv = do_write(c, d);
            if (!rv) {
                printf("CDDB command failed\n");
                cddb_error_print(cddb_errno(c));
                return -1;
            }
            /* print disc info */
            cddb_disc_print(d);
        } else {
			fprintf(stderr, "unable to create disc structure\n");
		}
    } else {
		fprintf(stderr, "unable to initialize libcddb\n");
	}
    cddb_disc_destroy(d);
    cddb_destroy(c);
    return 0;
}
