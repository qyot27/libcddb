/**
 * This example will read a CDDB disc entry from the server at
 * freedb.org using the CDDBP protocol.  It expects two command-line
 * parameters; i.e.:
 * 1) the CDDB category
 * 2) the CDDB disc ID
 */

#include <stdio.h>
#include <string.h>
#include <cddb/cddb.h>

static char *category;
static unsigned int discid;

#define STR_OR_NULL(s) ((s) ? s : "(null)")

static void usage(void)
{
    fprintf(stderr, "Usage: do_read <category> <hex disc ID>\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Available CDDB categories are:\n");
    fprintf(stderr, "\tdata, folk, jazz, misc, rock, country, blues,\n");
    fprintf(stderr, "\tnewage, reggae, classical, and soundtrack,\n");
    fprintf(stderr, "If you specify an invalid category then 'misc'\n");
    fprintf(stderr, "will be used\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "An example:\n");
    fprintf(stderr, "\tMezzanine from Massive Attack:\n");
    fprintf(stderr, "\t\tcategory = misc\n");
    fprintf(stderr, "\t\tdisc ID  = 0x920ef00b\n");
}

static void error(const char *msg)
{
    fprintf(stderr, "\nerror: %s\n", msg);
    exit(-1);
}

static int parse_cmdline(int argc, char **argv)
{
    if (argc != 3) {
        usage();
        return 0;
    }
    category = strdup(argv[1]);
    sscanf(argv[2], "%x", &discid);

    return 1;
}

int main(int argc, char **argv)
{
    cddb_conn_t *conn = NULL;   /* libcddb connection structure */
    cddb_disc_t *disc = NULL;   /* libcddb disc structure */
    cddb_track_t *track = NULL; /* libcddb track structure */
    int length, success;

    /* Check command-line parameters. */
    if (!parse_cmdline(argc, argv)) {
		error("invalid command-line options");
    }
    printf("Parameters: category = %s and disc ID = %08x\n", category, discid);

    /* Create a new connection structure.  You will have to use this
       structure in most other calls to the libcddb library. */
    conn = cddb_new();

    /* If the pointer is NULL then an error occured (out of memory). */
    if (!conn) {
		error("unable to create connection structure");
    }

    /* Initialize the CDDB server name.  Actually this call is
       optional because the default is 'freedb.org'.  So if the
       default is OK for you, then you can skip this step. */
    cddb_set_server_name(conn, "freedb.org");

    /* Initialize the CDDB server port.  This step can also be skipped
       if the default port (888) is good enough.  This default also
       matches the port of the FreeDB server. */
    cddb_set_server_port(conn, 888);

    /* We disable the usage of the local CDDB cache.  This cache can
       be used to speed things up drastically. */
    //cddb_cache_disable(conn);

    /* Create a new disc structure. */
    disc = cddb_disc_new();

    /* If the pointer is NULL then an error occured (out of memory). */
    if (!disc) {
		error("unable to create disc structure");
    }

    /* Initialize the ID of the disc. */
    cddb_disc_set_discid(disc, discid);

    /* Initialize the category of the disc.  This function converts a
       string into a category ID as used by libcddb.  If the specified
       string does not match any of the known categories, then the
       category is set to 'misc'. */
    cddb_disc_set_category(disc, category);

    /* Try reading the rest of the disc data.  This information will
       be retreived from the server specified above. */
    success = cddb_read(conn, disc);

    /* If an error occured then the return value will be false and the
       internal libcddb error number will be set. */
    if (!success) {
        /* Print an explanatory message on stderr.  Other routines are
           available for retrieving the message without printing it or
           printing it on a stream other than stderr. */
        cddb_error_print(cddb_errno(conn));
        exit(-1);
    }

    /* Let's print out some information about the disc. */
    /* 1. The artist name and disc title. */
    printf("Artist:   %s\n", STR_OR_NULL(cddb_disc_get_artist(disc)));
    printf("Title:    %s\n", STR_OR_NULL(cddb_disc_get_title(disc)));

    /* 2. The music genre.  This field is similar to the category
       field initialized above.  It can be the same but it does not
       have to be.  The category can only be come from a set of CDDB
       predefined categories.  The genre can be any string. */
    printf("Genre:    %s\n", STR_OR_NULL(cddb_disc_get_genre(disc)));

    /* 3. The disc year. */
    printf("Year:     %d\n", cddb_disc_get_year(disc));

    /* 4. The disc length and the number of tracks.  The length field
       is given in seconds. */
    length = cddb_disc_get_length(disc);
    printf("Length:   %d:%02d (%d seconds)\n", (length / 60), (length % 60), length);
    printf("%d tracks\n", cddb_disc_get_track_count(disc));

    /* 5. The tracks.  Track iteration can either be done by counting
       from 0 to (track_count - 1) and using the cddb_disc_get_track
       function.  Or by using the built-in iterator functions
       cddb_disc_get_track_first and cddb_disc_get_track_next.  We'll
       use the latter approach in this example. */

    for (track = cddb_disc_get_track_first(disc); 
         track != NULL; 
         track = cddb_disc_get_track_next(disc)) {

        /* 5.a. The track number on the disc.  This track number
           starts counting at 1.  So this is not the same number as
           the one used in cddb_disc_get_track. */
        printf("  [%02d]", cddb_track_get_number(track));

        /* 5.b. The track artist name and title. */
        printf(" '%s' by %s", cddb_track_get_title(track), 
               cddb_track_get_artist(track));

        /* 5.c. The track length. */
        length = cddb_track_get_length(track);
        if (length != -1) {
            printf(" (%d:%02d)\n", (length / 60), (length % 60));
        } else {
            printf(" (unknown)\n");
        }
    }

    /* Finally, we have to clean up.  With the cddb_disc_destroy
       function we can easily free all memory used by a single disc.
       This function will free the memory used by the individual
       tracks of the disc and also free the memory of the disc
       structure itself. Next we destroy the connection structrue in a
       similar way. */
    cddb_disc_destroy(disc);
    cddb_destroy(conn);

    return 0;
}
