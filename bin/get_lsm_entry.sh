#!/bin/sh
############################################################################
#   get_lsm_entry - gets the value of an entry in an lsm file
#                            -------------------
#   begin                : Sun Mar 05 2000
#   copyright            : (C) 2000 by Thomas Eschenbacher
#   email                : Thomas.Eschenbacher@gmx.de
############################################################################
#
############################################################################
#                                                                          #
#    This program is free software; you can redistribute it and/or modify  #
#    it under the terms of the GNU General Public License as published by  #
#    the Free Software Foundation; either version 2 of the License, or     #
#    (at your option) any later version.                                   #
#                                                                          #
############################################################################
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
    { if (p==1) print substr($0,length(field)+1) }' | sed s/^\[\ \]\*//g

# | awk '{ print $1 }'

