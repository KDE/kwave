############################################################################
#    Kwave                - libkwave/Makefile.am
#			     -------------------
#    begin                : Sat Nov 11 2001
#    copyright            : (C) 2000 by Thomas Eschenbacher
#    email                : Thomas.Eschenbacher@gmx.de
############################################################################

############################################################################
#                                                                          #
#    This program is free software; you can redistribute it and/or modify  #
#    it under the terms of the GNU General Public License as published by  #
#    the Free Software Foundation; either version 2 of the License, or     #
#    (at your option) any later version.                                   #
#                                                                          #
############################################################################

INCLUDES= @all_includes@ @ARTS_INCLUDES@

#if ! USE_SYSTEM_LIB_AUDIOFILE
INCLUDES+=-I ../libaudiofile
#endif

BUILTSOURCES=\
 KwaveDrag.moc.cpp\
 KwaveFilterPlugin.moc.cpp\
 KwavePlugin.moc.cpp\
 MultiTrackReader.moc.cpp\
 MultiTrackWriter.moc.cpp\
 PlayBackDevice.moc.cpp\
 SampleReader.moc.cpp\
 SampleWriter.moc.cpp\
 Selection.moc.cpp\
 Signal.moc.cpp\
 Stripe.moc.cpp\
 Track.moc.cpp\
 TransmissionFunction.moc.cpp\
 ArtsPlaybackSink.h\
 ArtsPlaybackSink.cc\
 ArtsSampleSink.h\
 ArtsSampleSink.cc\
 ArtsSampleSource.h\
 ArtsSampleSource.cc\
 CurveStreamAdapter.h\
 CurveStreamAdapter.cc\
 Decoder.moc.cpp\
 Encoder.moc.cpp

# Which sources should be compiled for kwave.
libkwave_la_SOURCES=\
 ${BUILTSOURCES}\
 ArtsMultiPlaybackSink.cpp\
 ArtsPlaybackSink_impl.cpp\
 ArtsSampleSource_impl.cpp\
 ArtsSampleSink_impl.cpp\
 CodecBase.cpp\
 CompressionType.cpp\
 cputest.c\
 Curve.cpp\
 CurveStreamAdapter_impl.cpp\
 Decoder.cpp\
 Filter.cpp\
 FileInfo.cpp\
 FileLoader.cpp\
 FileFormat.cpp\
 Functions.cpp\
 GenreType.cpp\
 Interpolation.cpp\
 KwaveDrag.cpp\
 KwaveFilterPlugin.cpp\
 KwavePlugin.cpp\
 Label.cpp\
 LabelType.cpp\
 LabelList.cpp\
 LineParser.cpp\
 memcpy.c\
 MultiTrackReader.cpp\
 MultiTrackWriter.cpp\
 Parser.cpp\
 ppcasm_string.S\
 Signal.cpp\
 SampleFIFO.cpp\
 SampleFormat.cpp\
 SampleReader.cpp\
 SampleWriter.cpp\
 SampleLock.cpp\
 StandardBitrates.cpp\
 Stripe.cpp\
 Track.cpp\
 TransmissionFunction.cpp\
 VirtualAudioFile.cpp\
 WindowFunction.cpp

libkwave_la_LDFLAGS = @LIBAUDIOFILE_LDFLAGS@ @GSL_LIBS@ \
	-version-info @LD_VERSION_INFO@ \
	--dynamic --export-dynamic

libkwave_la_LIBADD = @LIBAUDIOFILE_LIBADD@ \
	../mt/libkwavemt.la

lib_LTLIBRARIES = libkwave.la

# if you "make distclean", this files get removed. If you want to remove
# them while "make clean", use CLEANFILES

CLEANFILES = $(BUILTSOURCES)

DISTCLEANFILES = $(BUILTSOURCES) core *~ .deps/* Makefile.in Makefile

%moc.cpp: %h
	${MOC} $< -o $@

%cc: %idl
	${MCOPIDL} @MCOPIDL_INCLUDES@ $<

%h: %idl
	${MCOPIDL} @MCOPIDL_INCLUDES@ $<

# end of Makefile.am
