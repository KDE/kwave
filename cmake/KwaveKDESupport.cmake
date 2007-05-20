#############################################################################
##    Kwave                - cmake/KwaveKDESupport
##                           -------------------
##    begin                : Sun May 20 2007
##    copyright            : (C) 2007 by Thomas Eschenbacher
##    email                : Thomas.Eschenbacher@gmx.de
#############################################################################
#
#############################################################################
##                                                                          #
##    This program is free software; you can redistribute it and/or modify  #
##    it under the terms of the GNU General Public License as published by  #
##    the Free Software Foundation; either version 2 of the License, or     #
##    (at your option) any later version.                                   #
##                                                                          #
#############################################################################

FIND_PACKAGE(KDE3 REQUIRED)
ADD_DEFINITIONS(${QT_DEFINITIONS} ${KDE3_DEFINITIONS})
LINK_DIRECTORIES(${KDE3_LIB_DIR})

#############################################################################

EXECUTE_PROCESS(
    COMMAND
        ${KDECONFIG_EXECUTABLE} --expandvars --install html
    OUTPUT_VARIABLE
        KDE3_HTMLDIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

#############################################################################
#############################################################################
