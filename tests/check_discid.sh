#!/bin/sh
#
# $Id: check_discid.sh,v 1.5 2003/05/25 13:14:34 airborne Exp $

. ./settings.sh

#
# Check disc ID calculation of empty disc
#
DISCID='00000000'
start_test 'Check disc ID for '${DISCID}
cddb_query calc 0 0
check_discid $? ${DISCID}

#
# Check disc ID calculation of real disc
#
DISCID='920ef00b'
start_test 'Check disc ID calculation for '${DISCID}
cddb_query calc 3826 11 150 28615 51027 75835 102620 121460 148977 \
                175697 204322 231082 268002
check_discid $? ${DISCID}

#
# Print results and exit accordingly
#
finalize
