#!/bin/sh
#
# $Id: check_parse.sh,v 1.4 2003/05/08 19:40:40 airborne Exp $

. ./settings.sh

# Test parsing of some locally cached entries.  These entries are
# designed to test the parsing of all supported fields.  Mutli-line
# fields are also tested in every possible way.

for id in 12345678 12345679 1234567a 1234567b 1234567c 1234567d \
          1234567e 1234567f 12345680 12345681 12345682 12345683 ; do
    cddb_query -c only -D $CDDB_CACHE read misc $id
    check_read $? $id
done

#
# Print results and exit accordingly
#
finalize
