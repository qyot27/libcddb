#ifndef CDDB_CHECK_BASE_H
#define CDDB_CHECK_BASE_H 1

#ifdef __cplusplus
    extern "C" {
#endif


#include <check.h>
#include <cddb/cddb.h>


/* --- global settings --- */


#define NUM_OFFSETS 16
extern const int DISC_OFFSET[NUM_OFFSETS];

cddb_disc_t *empty_disc;
cddb_disc_t *real_disc;

/**
 * Create a series of discs.  This routine can be used in an
 * individual test if it needs some pre-initialized discs.
 */
void setup_discs(void);

/**
 * Destroy the discs created during setup.  When using the setup
 * routine, also make sure to use this tear down routine to free the
 * memory resources used by the discs.
 */
void teardown_discs(void);

/**
 * Function called to retrieve the test suite.
 */
extern Suite *get_suite(void);


#ifdef __cplusplus
    }
#endif

#endif /* CDDB_CHECK_BASE_H */
