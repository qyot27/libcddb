#!/bin/sh
#
# $Id: check_server.sh,v 1.3 2003/05/08 17:39:04 airborne Exp $

. ./settings.sh

# This script implements a set of tests to check whether the CDDB
# server access functionality is working correctly.  It tests all
# three protocols: CDDBP, HTTP and HTTP via a proxy.  We set the local
# cache dir to the directory with the test data to make sure we do not
# read the requested entries from the cache.

#
# Query disc with single match
#
QUERY_DATA='2259 8 155 25947 47357 66630 91222 110355 124755 148317'
QUERY_HASH='b2bba00e1890d659'

$CDDB_QUERY -c off -D $CDDB_CACHE -P cddbp query $QUERY_DATA | check_query $QUERY_HASH
$CDDB_QUERY -c off -D $CDDB_CACHE -P http  query $QUERY_DATA | check_query $QUERY_HASH
$CDDB_QUERY -c off -D $CDDB_CACHE -P proxy query $QUERY_DATA | check_query $QUERY_HASH

#
# Query disc with multiple matches
#
QUERY_DATA='3822 11 150 28690 51102 75910 102682 121522 149040 175772 204387 231145 268065'
QUERY_HASH='56c2171b89496518'

$CDDB_QUERY -c off -D $CDDB_CACHE -P cddbp query $QUERY_DATA | check_query $QUERY_HASH
$CDDB_QUERY -c off -D $CDDB_CACHE -P http  query $QUERY_DATA | check_query $QUERY_HASH
$CDDB_QUERY -c off -D $CDDB_CACHE -P proxy query $QUERY_DATA | check_query $QUERY_HASH

#
# Read disc data from server
#
DISCID='920ef00b'

$CDDB_QUERY -c off -D $CDDB_CACHE -P cddbp read misc $DISCID | check_read $DISCID
$CDDB_QUERY -c off -D $CDDB_CACHE -P http  read misc $DISCID | check_read $DISCID
$CDDB_QUERY -c off -D $CDDB_CACHE -P proxy read misc $DISCID | check_read $DISCID

#
# Fell through all tests
#
exit $SUCCESS
