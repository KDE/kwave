
INCLUDES= @all_includes@ @ARTS_INCLUDES@

BUILTSOURCES=\
 KwaveDrag.moc.cpp\
 MultiTrackReader.moc.cpp\
 MultiTrackWriter.moc.cpp\
 SampleReader.moc.cpp\
 SampleWriter.moc.cpp\
 Selection.moc.cpp\
 Signal.moc.cpp\
 Stripe.moc.cpp\
 Track.moc.cpp\
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
 ArtsSampleSource_impl.cpp\
 ArtsSampleSink_impl.cpp\
 CodecBase.cpp\
 CompressionType.cpp\
 Curve.cpp\
 CurveStreamAdapter_impl.cpp\
 Decoder.cpp\
 Filter.cpp\
 FileInfo.cpp\
 FileLoader.cpp\
 FileFormat.cpp\
 Functions.cpp\
 gsl_fft.cpp\
 Interpolation.cpp\
 KwaveDrag.cpp\
 Label.cpp\
 LabelType.cpp\
 LabelList.cpp\
 LineParser.cpp\
 MultiTrackReader.cpp\
 MultiTrackWriter.cpp\
 Parser.cpp\
 Signal.cpp\
 SampleFormat.cpp\
 SampleReader.cpp\
 SampleWriter.cpp\
 SampleLock.cpp\
 Stripe.cpp\
 Track.cpp\
 VirtualAudioFile.cpp\
 WindowFunction.cpp

libkwave_la_LDFLAGS = -L../libaudiofile -version-info @LD_VERSION_INFO@ \
	--dynamic --export-dynamic

libkwave_la_LIBADD = ../libaudiofile/libaudiofile.la \
	../mt/libkwavemt.la

lib_LTLIBRARIES = libkwave.la

# if you "make distclean", this files get removed. If you want to remove
# them while "make clean", use CLEANFILES

DISTCLEANFILES = $(BUILTSOURCES) core *~ .deps/* Makefile.in Makefile

%moc.cpp: %h
	${MOC} $< -o $@

%cc: %idl
	${MCOPIDL} @ARTS_INCLUDES@ $<

%h: %idl
	${MCOPIDL} @ARTS_INCLUDES@ $<

# end of Makefile.am
