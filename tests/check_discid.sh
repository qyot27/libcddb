#!/bin/sh
#
# $Id: check_discid.sh,v 1.2 2003/04/20 22:23:44 airborne Exp $

. settings.sh

#
# Check disc ID calculation of empty disc
#
$CDDB_QUERY calc 0 0 | check_discid '00000000'

#
# Check disc ID calculation of real disc
#
$CDDB_QUERY calc 3826 11 150 28615 51027 75835 102620 121460 148977 \
                 175697 204322 231082 268002 | check_discid '920ef00b'

#
# Fell through all tests
#
exit $SUCCESS
