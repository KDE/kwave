#############################################################################
##    Kwave                - plugins/normalize/CMakeLists.txt
##                           -------------------
##    begin                : Fri May 01 2009
##    copyright            : (C) 2009 by Thomas Eschenbacher
##    email                : Thomas.Eschenbacher@gmx.de
#############################################################################
#
#############################################################################
#                                                                           #
# Redistribution and use in source and binary forms, with or without        #
# modification, are permitted provided that the following conditions        #
# are met:                                                                  #
#                                                                           #
# 1. Redistributions of source code must retain the above copyright         #
#    notice, this list of conditions and the following disclaimer.          #
# 2. Redistributions in binary form must reproduce the above copyright      #
#    notice, this list of conditions and the following disclaimer in the    #
#    documentation and/or other materials provided with the distribution.   #
#                                                                           #
# For details see the accompanying cmake/COPYING-CMAKE-SCRIPTS file.        #
#                                                                           #
#############################################################################

SET(plugin_normalize_LIB_SRCS
    NormalizePlugin.cpp
    Normalizer.cpp

    NormalizePlugin.h
    Normalizer.h
)

KWAVE_PLUGIN(normalize)

#############################################################################
#############################################################################
