#!/bin/sh
#
# $Id: check_server.sh,v 1.5 2003/05/25 13:14:34 airborne Exp $

. ./settings.sh

# This script implements a set of tests to check whether the CDDB
# server access functionality is working correctly.  It tests all
# three protocols: CDDBP, HTTP and HTTP via a proxy.  We set the local
# cache dir to the directory with the test data to make sure we do not
# read the requested entries from the cache.

# Check whether we have proxy settings
if test -z $http_proxy; then
    NO_PROXY=1
    NO_PROXY_REASON='$http_proxy not defined'
else 
    NO_PROXY=0
fi

#
# Query disc with single match
#
QUERY_DATA='2259 8 155 25947 47357 66630 91222 110355 124755 148317'
QUERY_HASH='b2bba00e1890d659'

start_test 'CDDBP disc query '${QUERY_HASH}
cddb_query -c off -D $CDDB_CACHE -P cddbp query $QUERY_DATA
check_query $? $QUERY_HASH

start_test 'HTTP  disc query '${QUERY_HASH}
cddb_query -c off -D $CDDB_CACHE -P http  query $QUERY_DATA
check_query $? $QUERY_HASH

start_test 'PROXY disc query '${QUERY_HASH}
if test $NO_PROXY -eq 1; then
    skip $NO_PROXY_REASON
else
    cddb_query -c off -D $CDDB_CACHE -P proxy query $QUERY_DATA
    check_query $? $QUERY_HASH
fi

#
# Query disc with multiple matches
#
QUERY_DATA='3822 11 150 28690 51102 75910 102682 121522 149040 175772 204387 231145 268065'
QUERY_HASH='56c2171b89496518'

start_test 'CDDBP disc query '${QUERY_HASH}
cddb_query -c off -D $CDDB_CACHE -P cddbp query $QUERY_DATA
check_query $? $QUERY_HASH

start_test 'HTTP  disc query '${QUERY_HASH}
cddb_query -c off -D $CDDB_CACHE -P http  query $QUERY_DATA
check_query $? $QUERY_HASH

start_test 'PROXY disc query '${QUERY_HASH}
if test $NO_PROXY -eq 1; then
    skip $NO_PROXY_REASON
else
    cddb_query -c off -D $CDDB_CACHE -P proxy query $QUERY_DATA
    check_query $? $QUERY_HASH
fi

#
# Read disc data from server
#
DISCID='920ef00b'

start_test 'CDDBP disc read '${DISCID}
cddb_query -c off -D $CDDB_CACHE -P cddbp read misc $DISCID
check_read $? $DISCID

start_test 'HTTP  disc read '${DISCID}
cddb_query -c off -D $CDDB_CACHE -P http  read misc $DISCID
check_read $? $DISCID

start_test 'PROXY disc read '${DISCID}
cddb_query -c off -D $CDDB_CACHE -P proxy read misc $DISCID
if test $NO_PROXY -eq 1; then
    skip $NO_PROXY_REASON
else
    check_read $? $DISCID
fi

#
# Print results and exit accordingly
#
finalize
