
#include "base.h"


#define EXIT_SUCCESS 0
#define EXIT_FAILURE -1


/* --- global settings --- */


const int DISC_OFFSET[NUM_OFFSETS] = {
    150,  23627,  44723,  60527,  91311, 126360, 144562, 160236, 
    174360, 194624, 217273, 233402, 259181, 276582, 291679, 314363 
};


/* --- set up / tear down --- */


void setup_discs(void)
{ 
    unsigned int i;
    cddb_track_t *t = NULL;

    /* create an empty disc */
    empty_disc = cddb_disc_new();

    /* create a disc with data from an actual CD */
    real_disc = cddb_disc_new();
    real_disc->length = 4454;
    for (i=0; i<NUM_OFFSETS; i++) {
        t = cddb_track_new(); 
        t->frame_offset = DISC_OFFSET[i];
        cddb_disc_add_track(real_disc, t);
    }
}

void teardown_discs(void) 
{ 
    cddb_disc_destroy(empty_disc);
    cddb_disc_destroy(real_disc);
}


/* --- main entry point --- */


/**
 * Run a test suite.
 */
int main (void)
{
    int nf;

    Suite *s = get_suite();
    SRunner *sr = srunner_create(s);
    srunner_run_all(sr, CK_NORMAL);
    nf = srunner_ntests_failed(sr);
    srunner_free(sr);
    suite_free(s);

    return (nf == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
