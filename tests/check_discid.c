/*
    $Id: check_discid.c,v 1.2 2003/04/14 21:27:17 airborne Exp $

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
