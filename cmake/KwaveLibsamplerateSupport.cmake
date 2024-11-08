#############################################################################
##    Kwave                - cmake/KwaveLibsamplerateSupport.txt
##                           -------------------
##    begin                : Sat Jul 04 2009
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

INCLUDE(FindPkgConfig)
INCLUDE(UsePkgConfig)

#############################################################################
### check for libsamplerate headers and library                           ###

PKG_CHECK_MODULES(SAMPLERATE REQUIRED samplerate>=0.1.3)

SET(HAVE_LIBSAMPLERATE ON)

MESSAGE(STATUS "Found samplerate library in ${SAMPLERATE_LIBDIR}")
MESSAGE(STATUS "Found samplerate headers in ${SAMPLERATE_INCLUDEDIR}")

#############################################################################
#############################################################################
