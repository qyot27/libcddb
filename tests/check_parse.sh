#!/bin/sh
#
# $Id: check_parse.sh,v 1.3 2003/05/08 17:39:04 airborne Exp $

. ./settings.sh

# Test parsing of some locally cached entries.  These entries are
# designed to test the parsing of all supported fields.  Mutli-line
# fields are also tested in every possible way.

for id in 12345678 12345679 1234567a 1234567b 1234567c 1234567d \
          1234567e 1234567f 12345680 12345681 12345682 12345683 ; do
    $CDDB_QUERY -c only -D $CDDB_CACHE read misc $id | check_read $id
done

#
# Fell through all tests
#
exit $SUCCESS
