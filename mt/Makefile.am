# set the include path for X, qt and KDE
INCLUDES= @all_includes@

BUILTSOURCES=\
 AsyncSync.moc.cpp

# Which sources should be compiled for libkwavemt.
libkwavemt_a_SOURCES =\
 AsyncSync.cpp\
 TSS_Object.cpp \
 Mutex.cpp \
 MutexGuard.cpp \
 Thread.cpp \
 $(BUILTSOURCES)

noinst_LIBRARIES = libkwavemt.a

# libkwavemt_a_LDFLAGS = -version-info @LD_VERSION_INFO@ -static
# \ 
#	--export-dynamic -dynamic

# if you "make distclean", this files get removed. If you want to remove
# them while "make clean", use CLEANFILES
DISTCLEANFILES = $(BUILTSOURCES) core *~ .deps/*

%moc.cpp: %h
	${MOC} $< -o $@


# end of Makefile.am
