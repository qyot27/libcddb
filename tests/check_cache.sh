#!/bin/sh
#
# $Id: check_cache.sh,v 1.5 2003/05/08 19:40:40 airborne Exp $

. ./settings.sh

# This script implements a set of tests to check whether the local
# CDDB cache functionality is working correctly.

# Create/clear cache
CACHE="./tmpcache"
rm -rf $CACHE > /dev/null 2>&1
mkdir $CACHE

#
# Read from cache
#
DISCID1='920ef00b'
DISCID2='920ef00c'

# 1. read from cache only (should fail because cache is empty)
cddb_query -c only -D $CACHE read misc $DISCID1
check_not_found $? $DISCID1

# 2. read from server and disable cache
cddb_query -c off -D $CACHE read misc $DISCID1
check_read $? $DISCID1

# 3. read from cache only (should still fail because previous test did not cache)
cddb_query -c only -D $CACHE read misc $DISCID1
check_not_found $? $DISCID1

# 4. read from server and enable cache
cddb_query -c on -D $CACHE read misc $DISCID1
check_read $? $DISCID1

# 5. read from cache only (should succeed because previous test filled cache)
cddb_query -c only -D $CACHE read misc $DISCID1
check_read $? $DISCID1

# 6. enable cache and try to fetch non-existing disc (should fail)
cddb_query -c on -D $CACHE read misc $DISCID2
check_not_found $? $DISCID2

# 7. create non-existing disc in cache and read again (should succeed now)
cp $CACHE/misc/$DISCID1 $CACHE/misc/$DISCID2
cddb_query -c on -D $CACHE read misc $DISCID2
check_read $? $DISCID2

#
# Clean up, print results and exit accordingly
#
rm -rf $CACHE > /dev/null 2>&1
finalize
