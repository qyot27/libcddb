#!/bin/sh
#
# $Id: check_parse.sh,v 1.1 2003/04/21 10:20:33 airborne Exp $

. settings.sh

#
# 
#
$CDDB_QUERY -c only -D $CDDB_CACHE read misc 12345678 | check_read '12345678'

#
# Fell through all tests
#
exit $SUCCESS
