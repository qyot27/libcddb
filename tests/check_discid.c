
#include "base.h"


/* --- the tests --- */


/**
 * Test disc ID calculation of an empty disc.
 */
START_TEST(test_discid_empty_disc)
{
    int success;

    success = cddb_disc_calc_discid(empty_disc);
    fail_unless(success, "Disc ID calculation failed");
    fail_unless(empty_disc->discid == 0, NULL);
}
END_TEST

/**
 * Test disc ID calculation of a real disc.
 */
START_TEST(test_discid_real_disc)
{
    int success;

    success = cddb_disc_calc_discid(real_disc);
    fail_unless(success, "Disc ID calculation failed");
    fail_unless(real_disc->discid == 0xfe116410, NULL);
}
END_TEST


/* --- the test suite --- */


Suite *get_suite(void)
{
    Suite *s = suite_create("Disc ID");
    TCase *tc_core = tcase_create("Core");
    tcase_add_checked_fixture(tc_core, setup_discs, teardown_discs);
    suite_add_tcase(s, tc_core);

    tcase_add_test(tc_core, test_discid_empty_disc);
    tcase_add_test(tc_core, test_discid_real_disc);

    return s;
}
