INCLUDES= @all_includes@

kwavelibdir=$(libdir)

kwaveincludedir=$(includedir)/libkwave
libkwave_la_LDFLAGS = -version-info @LD_VERSION_INFO@

# Which sources should be compiled for kwave.
libkwave_la_SOURCES =\
 Curve.cpp\
 Filter.cpp\
 FileLoader.cpp\
 Functions.cpp\
 gsl_fft.cpp\
 Interpolation.cpp\
 Label.cpp\
 LabelType.cpp\
 LabelList.cpp\
 LineParser.cpp\
 mmap.cpp\
 Parser.cpp\
 Signal.cpp\
 SampleInputStream.cpp\
 Stripe.cpp\
 Track.cpp\
 WindowFunction.cpp

kwaveinclude_HEADERS = \
 Curve.h\
 Filter.h\
 FileFormat.h\
 FileLoader.h\
 Functions.h\
 gsl_fft.h\
 InsertMode.h\
 Interpolation.h\
 Label.h\
 LabelList.h\
 LabelType.h\
 LineParser.h\
 MenuItem.h\
 mmap.h\
 Parser.h\
 PointSet.h\
 SampleInputStream.h\
 Signal.h\
 Stripe.h\
 Track.h\
 WindowFunction.h

libkwave_la_LIBADD = $(topdir)/mt/libkwavemt.a 

lib_LTLIBRARIES = libkwave.la

# if you "make distclean", this files get removed. If you want to remove
# them while "make clean", use CLEANFILES
DISTCLEANFILES = core *~ .deps/*

