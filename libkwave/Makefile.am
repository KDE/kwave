INCLUDES= @all_includes@

BUILTSOURCES=""

# Which sources should be compiled for kwave.
libkwavestatic_a_SOURCES =\
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
 WindowFunction.cpp \
 ${BUILTSOURCES}

noinst_LIBRARIES = libkwavestatic.a

all: libkwave.so

libkwave.so: libkwavestatic.a ../mt/libkwavemt.a libkwavestatic.a ../libgui/libkwavegui.a
	ld -o libkwave.so \
	--whole-archive ../mt/libkwavemt.a \
	--whole-archive libkwavestatic.a \
	--whole-archive ../libgui/libkwavegui.a \
	--shared

#libkwave_la_LIBADD = ../mt/libkwavemt.a ../libgui/libkwavegui.a
#
#libkwave_la_LDFLAGS = -version-info @LD_VERSION_INFO@ \
#	-dy --dynamic -export-dynamic -whole-archive \
#	-dynamic --export-dynamic --whole-archive
#
#lib_LTLIBRARIES = libkwave.la

# if you "make distclean", this files get removed. If you want to remove
# them while "make clean", use CLEANFILES

DISTCLEANFILES = $(BUILTSOURCES) core *~ .deps/*

%moc.cpp: %h
	${MOC} $< -o $@

# end of Makefile.am
