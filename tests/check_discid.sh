#!/bin/sh
#
# $Id: check_discid.sh,v 1.4 2003/05/08 19:40:40 airborne Exp $

. ./settings.sh

#
# Check disc ID calculation of empty disc
#
cddb_query calc 0 0
check_discid $? '00000000'

#
# Check disc ID calculation of real disc
#
cddb_query calc 3826 11 150 28615 51027 75835 102620 121460 148977 \
                175697 204322 231082 268002
check_discid $? '920ef00b'

#
# Print results and exit accordingly
#
finalize
