INCLUDES= @all_includes@

kwavelibdir=$(libdir)

kwaveincludedir=$(includedir)/libkwave
libkwave_la_LDFLAGS = -version-info 0:3:0

# Which sources should be compiled for kwave.
libkwave_la_SOURCES =functions.cpp parser.cpp interpolation.cpp signal.cpp \
signaledit.cpp windowfunction.cpp filter.cpp gsl_fft.cpp mmap.cpp markers.cpp \
dynamicloader.cpp timeoperation.cpp dialogoperation.cpp curve.cpp string.cpp \
color.cpp fileloader.cpp

kwaveinclude_HEADERS = color.h curve.h dialogoperation.h dynamicloader.h \
fileformat.h fileloader.h filter.h functions.h globals.h gsl_fft.h \
interpolation.h kwavesignal.h kwavestring.h markers.h menuitem.h mmap.h \
parser.h pointset.h timeoperation.h windowfunction.h

lib_LTLIBRARIES = libkwave.la

# if you "make distclean", this files get removed. If you want to remove
# them while "make clean", use CLEANFILES
DISTCLEANFILES = core *~ .deps/*






