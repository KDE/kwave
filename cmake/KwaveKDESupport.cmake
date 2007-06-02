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

# these install paths are KDE specific:
#
# KDE3_HTMLDIR       Where your docs should go to. (contains lang subdirs)
# KDE3_APPSDIR       Where your application file (.kdelnk) should go to.
# KDE3_EXEDIR        Where your application binary should go to.
# KDE3_ICONDIR       Where your icon should go to.
# KDE3_DATADIR       Where you install application data. (Use a subdir)
# KDE3_LOCALE        Where translation files should go to.(contains lang subdirs)
# KDE3_MIMEDIR       Where mimetypes should go to.
# KDE3_MODULEDIR     Where loadable modules should go to.

#############################################################################
EXECUTE_PROCESS(
    COMMAND
        ${KDECONFIG_EXECUTABLE} --expandvars --install html
    OUTPUT_VARIABLE
        KDE3_HTMLDIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

#############################################################################
EXECUTE_PROCESS(
    COMMAND
        ${KDECONFIG_EXECUTABLE} --expandvars --install apps
    OUTPUT_VARIABLE
        KDE3_APPSDIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

#############################################################################
EXECUTE_PROCESS(
    COMMAND
        ${KDECONFIG_EXECUTABLE} --expandvars --install exe
    OUTPUT_VARIABLE
        KDE3_EXEDIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

#############################################################################
EXECUTE_PROCESS(
    COMMAND
        ${KDECONFIG_EXECUTABLE} --expandvars --install icon
    OUTPUT_VARIABLE
        KDE3_ICONDIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

#############################################################################
EXECUTE_PROCESS(
    COMMAND
        ${KDECONFIG_EXECUTABLE} --expandvars --install data
    OUTPUT_VARIABLE
        KDE3_DATADIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

#############################################################################
EXECUTE_PROCESS(
    COMMAND
        ${KDECONFIG_EXECUTABLE} --expandvars --install locale
    OUTPUT_VARIABLE
        KDE3_LOCALEDIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

#############################################################################
EXECUTE_PROCESS(
    COMMAND
        ${KDECONFIG_EXECUTABLE} --expandvars --install mime
    OUTPUT_VARIABLE
        KDE3_MIMEDIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

#############################################################################
EXECUTE_PROCESS(
    COMMAND
        ${KDECONFIG_EXECUTABLE} --expandvars --install module
    OUTPUT_VARIABLE
        KDE3_MODULEDIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

#############################################################################
EXECUTE_PROCESS(
    COMMAND
        ${KDECONFIG_EXECUTABLE} --expandvars --install config
    OUTPUT_VARIABLE
        KDE3_CONFDIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)


#############################################################################
EXECUTE_PROCESS(
    COMMAND
        ${KDECONFIG_EXECUTABLE} --expandvars --install html
    OUTPUT_VARIABLE
        KDE3_DOCDIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
STRING(REGEX REPLACE "HTML$" "" KDE3_DOCDIR ${KDE3_DOCDIR})

#############################################################################
#############################################################################
