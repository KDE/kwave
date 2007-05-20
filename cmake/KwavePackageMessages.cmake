#############################################################################
##    Kwave                - cmake/KwavePackageMessages.cmake
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

FIND_PACKAGE(RequiredProgram REQUIRED)

FIND_REQUIRED_PROGRAM(XGETTEXT_EXECUTABLE xgettext)
FIND_REQUIRED_PROGRAM(FIND_EXECUTABLE find)
SET(KDE_POT_FILE ${KDE3_INCLUDE_DIR}/kde.pot)

ADD_CUSTOM_TARGET(package-messages
    COMMAND $(MAKE) all # first make sure all generated source exists
    COMMAND ${RM_EXECUTABLE} -f po/*.gmo
    COMMAND ${XGETTEXT_EXECUTABLE} -C
        -ki18n -ktr2i18n -kI18N_NOOP
        -x ${KDE_POT_FILE}
        `${FIND_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR} -name \\*.h -o -name \\*.cpp`
        `${FIND_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR} -name \\*.h -o -name \\*.cpp`
        -o ${CMAKE_SOURCE_DIR}/po/kwave.pot
    COMMAND $(MAKE) translations
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/kwave/menus_config_i18n.cpp
)

#############################################################################
#############################################################################
