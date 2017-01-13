#############################################################################
##    Kwave                - cmake/KwaveEbuild.cmake
##                           -------------------
##    begin                : Wed May 23 2007
##    copyright            : (C) 2007 by Thomas Eschenbacher
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

SET(_ebuild_version "${KWAVE_VERSION_MAJOR}.${KWAVE_VERSION_MINOR}.${KWAVE_VERSION_RELEASE}")
SET(_ebuild ${DISTFILES_DIR}/kwave-${_ebuild_version}.ebuild)

CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/kwave.ebuild.in" "${_ebuild}" @ONLY)

SET(KWAVE_DISTFILES ${KWAVE_DISTFILES} ${_ebuild})
SET(KWAVE_ADDITIONAL_CLEAN_FILES ${KWAVE_ADDITIONAL_CLEAN_FILES} ${_ebuild})

#############################################################################
#############################################################################
