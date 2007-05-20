#############################################################################
##    Kwave                - kwave/CMakeLists.txt
##                           -------------------
##    begin                : Wed May 09 2007
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

INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${KDE3_INCLUDE_DIR}
    ${QT_INCLUDE_DIR}
)

FIND_REQUIRED_PROGRAM(CONVERT_EXECUTABLE convert)

#############################################################################
### generate menus_config_i18n.cpp                                        ###

ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/menus_config_i18n.cpp
    COMMAND ${PROJECT_SOURCE_DIR}/bin/menusconfig2cpp.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/menus.config
        ${CMAKE_CURRENT_BINARY_DIR}/menus_config_i18n.cpp
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/menus.config
    DEPENDS ${PROJECT_SOURCE_DIR}/bin/menusconfig2cpp.sh
)

#############################################################################

SET(kwave_SRCS
    main.cpp
    ClipBoard.cpp
    CodecManager.cpp
    KwaveApp.cpp
    KwaveSplash.cpp
    LabelManager.cpp
    MainWidget.cpp
    MemoryManager.cpp
    menus_config_i18n.cpp
    MouseMark.cpp
    PlaybackController.cpp
    PluginManager.cpp
    SignalManager.cpp
    SignalWidget.cpp
    SwapFile.cpp
    TopWidget.cpp
    UndoAddLabelAction.cpp
    UndoDeleteAction.cpp
    UndoDeleteLabelAction.cpp
    UndoDeleteTrack.cpp
    UndoFileInfo.cpp
    UndoInsertAction.cpp
    UndoInsertTrack.cpp
    UndoModifyAction.cpp
    UndoModifyLabelAction.cpp
    UndoSelection.cpp
    UndoTransaction.cpp
    UndoTransactionGuard.cpp
)

#############################################################################
### macro for scaling icons                                               ###

MACRO(KWAVE_SCALE_ICONS _sources)
    FOREACH(_size ${ARGN})
        SET(_orig ${CMAKE_CURRENT_SOURCE_DIR}/pics/kwave.xpm)
        SET(_scaled ${CMAKE_CURRENT_BINARY_DIR}/kwave-${_size}.png)

        # generate the scaled version
        ADD_CUSTOM_COMMAND(OUTPUT ${_scaled}
            COMMAND ${CONVERT_EXECUTABLE}
                -geometry ${_size} ${_orig} ${_scaled}
            DEPENDS ${_orig}
        )
        SET(${_sources} ${${_sources}} ${_scaled})
        SET_SOURCE_FILES_PROPERTIES(${_scaled} PROPERTIES GENERATED true)
        SET_DIRECTORY_PROPERTIES(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES
            "${_scaled}")
    ENDFOREACH(_size)
ENDMACRO(KWAVE_SCALE_ICONS)

#############################################################################
### scaled icons                                                          ###

KWAVE_SCALE_ICONS(kwave_SRCS
    16x16
    22x22
    32x32
    48x48
)

#############################################################################
### some pixmaps to be installed in the data dir                          ###

SET(kwave_PIXMAPS
    amplify_free.png
    fade_in.png
    fade_out.png
    knob.xpm
    kwave-splash.png
    light_off.xpm
    light_on.xpm
    logo.xpm
    lspeaker.xpm
    minifont.xpm
    noise.png
    playback_loop.xpm
    playback_start.xpm
    playback_stop.xpm
    playback_pause.xpm
    playback_pause2.xpm
    rspeaker.xpm
    sound_arts.png
    sound_device.png
    sound_subdevice.png
    sound_card.png
    sound_note.png
    xspeaker.xpm
)

#############################################################################

KDE3_AUTOMOC(${kwave_SRCS})
KDE3_ADD_EXECUTABLE(kwave_core ${kwave_SRCS})

#############################################################################

TARGET_LINK_LIBRARIES(kwave_core
    kwave
    kwavegui
    ${QT_AND_KDECORE_LIBS}
    kdeui
    kio
    artsc
    artsflow
)

ADD_DEPENDENCIES(kwave_core libkwavegui libkwave)

SET_TARGET_PROPERTIES(kwave_core PROPERTIES OUTPUT_NAME "kwave")

#############################################################################
########### install files ###############

# INSTALL_TARGETS(/bin kwave )
# INSTALL_FILES( /share/mimelink/audio FILES  x-aifc.desktop x-ircam.desktop )

#original Makefile.am contents follow:

## this 10 paths are KDE specific. Use them:
## kde_htmldir       Where your docs should go to. (contains lang subdirs)
## kde_appsdir       Where your application file (.kdelnk) should go to.
## kde_icondir       Where your icon should go to.
## kde_minidir       Where your mini icon should go to.
## kde_datadir       Where you install application data. (Use a subdir)
## kde_locale        Where translation files should go to.(contains lang subdirs)
## kde_cgidir        Where cgi-bin executables should go to.
## kde_confdir       Where config files should go to.
## kde_mimedir       Where mimetypes should go to.
#
#APPSDIR= $(DESTDIR)$(kde_appsdir)/Multimedia
#CONFDIR= $(DESTDIR)$(kde_datadir)/$(PACKAGE)
#PIXDIR = $(DESTDIR)$(kde_datadir)/$(PACKAGE)/pics
## set the include path for X, qt and KDE
#INCLUDES= -I${topdir} @all_includes@ @ARTS_INCLUDES@
#ICONDIR = $(DESTDIR)$(kde_icondir)
#
#mimedir = $(kde_mimedir)/audio
#mime_DATA = x-aifc.desktop x-ircam.desktop
#EXTRA_DIST = $(mime_DATA)
#
#bin_PROGRAMS  = kwave
#
## specifying the bindir is necessary in order to let make install
## use the correct directory when building a binary rpm package
#bindir=$(prefix)/bin
#
## some pixmaps to be installed in the data dir
#PIXMAPS=...
#
## the library search path
#kwave_LDFLAGS = -L../libkwave -L../libgui \
#	`$(ARTSCCONFIG) --libs` @all_libraries@ \
#	-export-dynamic -dy
#
## the libraries to link against. Be aware of the order. First the libraries,
## that depend on the following ones.
#kwave_LDADD   =  -lkwavegui -lkwave \
#	@KDE_VERSION_SPECIFIC_LIBS@ \
#	-lX11 -lXext -lpthread -ldl -lm
#
#$(APPSDIR)/kwave.desktop:
#
## make messages.po. Move this one to ../po/ and "make merge" in po
#messages:
#	$(XGETTEXT) -C -ktranslate $(kwave_SOURCES)
#
## just install datas here. Use install-exec-data for scripts and etc.
## the binary itself is already installed from automake
## use mkinstalldirs, not "install -d"
## don't install a list of file. Just one file per install.
## if you have more of them, create a subdirectory with an extra Makefile
#
#install-data-local: icons
#	$(mkinstalldirs) $(APPSDIR)
#	$(INSTALL_DATA) kwave.desktop $(APPSDIR)/kwave.desktop
#	$(mkinstalldirs) $(PIXDIR)
#	for pix in $(PIXMAPS); do { $(INSTALL_DATA) pics/$$pix $(PIXDIR) ; }; done;
#	$(mkinstalldirs) $(CONFDIR)
#	$(INSTALL_DATA) menus.config $(CONFDIR)/menus.config
#	for col in locolor hicolor; do { \
#	  for dim in 16x16 22x22 32x32 48x48; do { \
#	    $(mkinstalldirs) $(ICONDIR)/$$col/$$dim/apps/; \
#	    $(INSTALL_DATA) pics/${PACKAGE}-$$dim.png $(ICONDIR)/$$col/$$dim/apps/$(PACKAGE).png; \
#	  }; done; \
#	}; done;
#
## remove ALL you have installed in install-data-local or install-exec-local
#uninstall-local:
#	for col in locolor hicolor; do { \
#	  for dim in 16x16 22x22 32x32 48x48; do { \
#	    rm -f $(ICONDIR)/$$col/$$dim/apps/$(PACKAGE).png || true; \
#	    (rmdir -p --ignore-fail-on-non-empty $(ICONDIR)/$$col/$$dim/apps/ >/dev/null 2>&1); \
#	  }; done; \
#	}; done;
#	for pix in $(PIXMAPS); do { rm -fv $(PIXDIR)/$$pix ; }; done;
#	-rm -f $(APPSDIR)/kwave.desktop
#	-rm -f $(CONFDIR)/menus.config
#	-rmdir -p --ignore-fail-on-non-empty $(APPSDIR) >/dev/null 2>&1
#	-rmdir -p --ignore-fail-on-non-empty $(CONFDIR) >/dev/null 2>&1
#	-rmdir -p --ignore-fail-on-non-empty $(PIXDIR) >/dev/null 2>&1
#	-rmdir -p --ignore-fail-on-non-empty $(DESTDIR)$(bindir) >/dev/null 2>&1

#############################################################################
#############################################################################
