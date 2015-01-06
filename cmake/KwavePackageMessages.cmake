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

FIND_REQUIRED_PROGRAM(MSGCAT_EXECUTABLE msgcat)
FIND_REQUIRED_PROGRAM(XGETTEXT_EXECUTABLE xgettext)
FIND_REQUIRED_PROGRAM(FIND_EXECUTABLE find)

# SET(KDE_POT_FILE ${KDE4_INCLUDE_DIR}/kde.pot)

SET(MENUSCONFIG2POT    ${CMAKE_SOURCE_DIR}/bin/menusconfig2pot.pl)
SET(KWAVE_MENUS_CONFIG ${CMAKE_SOURCE_DIR}/kwave/menus.config)
SET(KWAVE_GUI_POT      ${CMAKE_BINARY_DIR}/po/kwave_gui.pot)
SET(KWAVE_MENU_POT     ${CMAKE_BINARY_DIR}/po/kwave_menu.pot)
SET(KWAVE_POT          ${CMAKE_SOURCE_DIR}/po/kwave.pot)

#############################################################################
### generate kwave_menu.pot                                               ###

ADD_CUSTOM_COMMAND(OUTPUT ${KWAVE_MENU_POT}
    COMMAND ${MENUSCONFIG2POT} ${KWAVE_MENUS_CONFIG} ${KWAVE_MENU_POT}
    DEPENDS ${KWAVE_MENUS_CONFIG}
    DEPENDS ${MENUSCONFIG2POT}
)

#############################################################################
### generate kwave_gui.pot and merge with kwave_menu.pot into kwave.pot   ###

ADD_CUSTOM_TARGET(package-messages
    COMMAND $(MAKE) update-handbook # update the handbook
    COMMAND $(MAKE) all # first make sure all generated source exists
    COMMAND ${RM_EXECUTABLE} -f po/*.gmo
    COMMAND ${XGETTEXT_EXECUTABLE} --from-code=UTF-8 -C --kde
        -ci18n -ki18n:1 -ki18nc:1c,2 -ki18np:1,2 -ki18ncp:1c,2,3 -ktr2i18n:1
        -kI18N_NOOP:1 -kI18N_NOOP2:1c,2 -kaliasLocale -kki18n:1 -kki18nc:1c,2
        -kki18np:1,2 -kki18ncp:1c,2,3
        -D ${CMAKE_CURRENT_SOURCE_DIR}
        -D ${CMAKE_CURRENT_BINARY_DIR}
        `cd ${CMAKE_CURRENT_SOURCE_DIR} && ${FIND_EXECUTABLE} . -name \\*.h -o -name \\*.cpp`
        `cd ${CMAKE_CURRENT_BINARY_DIR} && ${FIND_EXECUTABLE} . -name \\*.h -o -name \\*.cpp`
        -o ${KWAVE_GUI_POT}
    COMMAND ${MSGCAT_EXECUTABLE} ${KWAVE_GUI_POT} ${KWAVE_MENU_POT} -o ${KWAVE_POT}
    COMMAND $(MAKE) translations
    DEPENDS ${KWAVE_MENU_POT}
)

#############################################################################
#############################################################################
