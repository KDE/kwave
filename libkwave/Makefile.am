
INCLUDES= @all_includes@ -I$(includedir)/arts

BUILTSOURCES=\
 KwaveDrag.moc.cpp\
 MultiTrackWriter.moc.cpp\
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
 Curve.cpp\
 CurveStreamAdapter_impl.cpp\
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
 SampleReader.cpp\
 SampleWriter.cpp\
 SampleLock.cpp\
 Stripe.cpp\
 Track.cpp\
 WindowFunction.cpp

libkwave_la_LDFLAGS = -version-info @LD_VERSION_INFO@ \
	--dynamic --export-dynamic

lib_LTLIBRARIES = libkwave.la

# if you "make distclean", this files get removed. If you want to remove
# them while "make clean", use CLEANFILES

DISTCLEANFILES = $(BUILTSOURCES) core *~ .deps/* Makefile.in Makefile

%moc.cpp: %h
	${MOC} $< -o $@

%cc: %idl
	${MCOPIDL} -I$(includedir)/arts $<

%h: %idl
	${MCOPIDL} -I$(includedir)/arts $<

# end of Makefile.am
