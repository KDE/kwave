#############################################################################
##    Kwave                - plugins/CMakeLists.txt
##                           -------------------
##    begin                : Mon May 28 2007
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

    ADD_LIBRARY(plugin_${_plugin} MODULE ${plugin_${_plugin}_LIB_SRCS})
    TARGET_LINK_LIBRARIES(plugin_${_plugin}
        libkwavegui
        libkwave
        ${plugin_${_plugin}_LIBS}
        ${SAMPLERATE_LIBS}
        Qt::Core
        Qt::Concurrent
        Qt::Widgets
        KF6::ConfigCore
        KF6::CoreAddons
        KF6::I18n
        KF6::IconThemes
        KF6::KIOFileWidgets
        KF6::TextWidgets
        KF6::WidgetsAddons
        KF6::XmlGui
    )

    SET_TARGET_PROPERTIES(plugin_${_plugin} PROPERTIES
        OUTPUT_NAME "${_plugin}"
        LINK_FLAGS "-Wl,--export-dynamic"
        ENABLE_EXPORTS TRUE
        POSITION_INDEPENDENT_CODE ON
    )
    SET(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

    CONFIGURE_FILE(
        ${CMAKE_CURRENT_SOURCE_DIR}/kwaveplugin_${_plugin}.json.in
        ${CMAKE_CURRENT_BINARY_DIR}/kwaveplugin_${_plugin}.json
    )

    #########################################################################
    ### install                                                           ###

    INSTALL(TARGETS
        plugin_${_plugin}
        LIBRARY
        DESTINATION ${KDE_INSTALL_PLUGINDIR}/kwave
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
ADD_SUBDIRECTORY( export_k3b )
ADD_SUBDIRECTORY( fileinfo )
ADD_SUBDIRECTORY( goto )
ADD_SUBDIRECTORY( lowpass )
ADD_SUBDIRECTORY( newsignal )
ADD_SUBDIRECTORY( noise )
ADD_SUBDIRECTORY( normalize )
ADD_SUBDIRECTORY( notch_filter )
ADD_SUBDIRECTORY( pitch_shift )
ADD_SUBDIRECTORY( playback )        # needs one of: OSS/ALSA/PulseAudio
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
