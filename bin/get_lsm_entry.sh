#!/bin/sh
#
# get_lsm_entry - gets the value of an entry in an lsm file
# 
# 05.03.2000 by Thomas Eschenbacher <Thomas.Eschenbacher@gmx.de>
#
# parameters:
# stdin 
# $1 full path to the lsm file
# $2 name of the lsm entry
#
# NOTE: this should be regarded to be a quick hack, only few error checking 
#       is performed !!!

# uncomment this for debugging
# set -x

cat $1 | awk -v field=$2: ' \
    { p=index($0, field) } \
    { if (p==1) print substr($0,length(field)+1) }' 

# | awk '{ print $1 }'

