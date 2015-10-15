#############################################################################
##    Kwave                - plugins/CMakeLists.txt
##                           -------------------
##    begin                : Mon May 28 2007
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

MACRO(KWAVE_PLUGIN _plugin)

    #########################################################################
    ### compile stage                                                     ###

    INCLUDE_DIRECTORIES(
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
    )
    KI18N_WRAP_UI(plugin_${_plugin}_LIB_SRCS ${plugin_${_plugin}_LIB_UI})

    #########################################################################
    ### link stage                                                        ###

    KDE4_ADD_LIBRARY(plugin_${_plugin} SHARED ${plugin_${_plugin}_LIB_SRCS})
    TARGET_LINK_LIBRARIES(plugin_${_plugin}
        kwavegui
        kwave
        ${plugin_${_plugin}_LIBS}
        ${SAMPLERATE_LIBS}
        Qt5::Core
        Qt5::Concurrent
        Qt5::Widgets
        KF5::ConfigCore
        KF5::CoreAddons
        KF5::I18n
        KF5::IconThemes
        KF5::KIOFileWidgets
        KF5::TextWidgets
        KF5::WidgetsAddons
        KF5::XmlGui
    )

    SET_TARGET_PROPERTIES(plugin_${_plugin} PROPERTIES PREFIX "")
    SET_TARGET_PROPERTIES(plugin_${_plugin} PROPERTIES SUFFIX "")
    SET_TARGET_PROPERTIES(plugin_${_plugin} PROPERTIES OUTPUT_NAME "${_plugin}")
    SET_TARGET_PROPERTIES(plugin_${_plugin} PROPERTIES LINK_FLAGS
        "-Wl,--export-dynamic")
    SET(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

    #########################################################################
    ### install                                                           ###

    ADD_DEPENDENCIES(plugin_${_plugin} libkwave libkwavegui)

    INSTALL(TARGETS
        plugin_${_plugin}
        LIBRARY
        DESTINATION ${DATA_INSTALL_DIR}/kwave/plugins
        PERMISSIONS OWNER_READ OWNER_EXECUTE
                    GROUP_READ GROUP_EXECUTE
                    WORLD_READ WORLD_EXECUTE
    )

ENDMACRO(KWAVE_PLUGIN)

#############################################################################

ADD_SUBDIRECTORY( about )
ADD_SUBDIRECTORY( amplifyfree )
ADD_SUBDIRECTORY( band_pass )
ADD_SUBDIRECTORY( codec_ascii )
ADD_SUBDIRECTORY( codec_audiofile ) # needs libaudiofile
ADD_SUBDIRECTORY( codec_flac )      # needs >= libflac 1.2.0
ADD_SUBDIRECTORY( codec_mp3 )       # needs libmad + id3lib + "lame"
ADD_SUBDIRECTORY( codec_ogg )       # needs libogg and (libvorbis or libopus)
ADD_SUBDIRECTORY( codec_wav )       # needs libaudiofile
ADD_SUBDIRECTORY( debug )
ADD_SUBDIRECTORY( fileinfo )
ADD_SUBDIRECTORY( goto )
ADD_SUBDIRECTORY( lowpass )
ADD_SUBDIRECTORY( memory )
ADD_SUBDIRECTORY( newsignal )
ADD_SUBDIRECTORY( noise )
ADD_SUBDIRECTORY( normalize )
ADD_SUBDIRECTORY( notch_filter )
ADD_SUBDIRECTORY( pitch_shift )
ADD_SUBDIRECTORY( playback )        # needs one of: OSS/ALSA/PulseAudio/Phonon
ADD_SUBDIRECTORY( record )          # needs OSS and/or ALSA
ADD_SUBDIRECTORY( reverse )
ADD_SUBDIRECTORY( samplerate )      # needs libsamplerate
ADD_SUBDIRECTORY( saveblocks )
ADD_SUBDIRECTORY( selectrange )
ADD_SUBDIRECTORY( sonagram )        # needs fftw >= 3.0
ADD_SUBDIRECTORY( stringenter )
ADD_SUBDIRECTORY( volume )
ADD_SUBDIRECTORY( zero )

#############################################################################
#############################################################################
