/* config.h.in.  Generated from configure.in by autoheader.  */

/* use ppc target optimizations (from xine) */
#cmakedefine ARCH_PPC

/* use ix86 target optimizations (from xine) */
#cmakedefine ARCH_X86

/* use X86_64 target optimizations (from xine) */
#cmakedefine ARCH_X86_64

/* byte-order of the host machine */
#cmakedefine ENDIANESS_BIG

/* byte-order of the host machine */
#cmakedefine ENDIANESS_LITTLE

/* old API from v1.1.1 and before */
#cmakedefine FLAC_API_VERSION_1_1_1_OR_OLDER

/* new API from v1.1.2 and newer */
#cmakedefine FLAC_API_VERSION_1_1_2

/* support playback/recording via ALSA */
#cmakedefine HAVE_ALSA_SUPPORT

/* Define to 1 if you have the <artsc.h> header file. */
#cmakedefine HAVE_ARTSC_H

/* Define to 1 if you have the <artsmodules.h> header file. */
#cmakedefine HAVE_ARTSMODULES_H

/* support playback via aRts */
#cmakedefine HAVE_ARTS_SUPPORT

/* Define to 1 if you have the <assert.h> header file. */
#cmakedefine HAVE_ASSERT_H

/* You _must_ have bool */
#cmakedefine HAVE_BOOL

/* Define to 1 if you have the <byteswap.h> header file. */
#cmakedefine HAVE_BYTESWAP_H

/* Define if you have the CoreAudio API */
#cmakedefine HAVE_COREAUDIO

/* Define to 1 if you have the <crt_externs.h> header file. */
#cmakedefine HAVE_CRT_EXTERNS_H

/* Defines if your system has the crypt function */
#cmakedefine HAVE_CRYPT

/* Define to 1 if you have the <ctype.h> header file. */
#cmakedefine HAVE_CTYPE_H

/* Define to 1 if you have the <dirent.h> header file, and it defines `DIR'.
   */
#cmakedefine HAVE_DIRENT_H

/* Define to 1 if you have the <dlfcn.h> header file. */
#cmakedefine HAVE_DLFCN_H

/* Define to 1 if you have the <endian.h> header file. */
#cmakedefine HAVE_ENDIAN_H

/* Define to 1 if you have the <errno.h> header file. */
#cmakedefine HAVE_ERRNO_H

/* Define to 1 if you have the <fcntl.h> header file. */
#cmakedefine HAVE_FCNTL_H

/* Define to 1 if you have the <features.h> header file. */
#cmakedefine HAVE_FEATURES_H

/* Define to 1 if you have the <float.h> header file. */
#cmakedefine HAVE_FLOAT_H

/* resource limits for the memory manager */
#cmakedefine HAVE_GETRLIMIT

/* Define to 1 if you have the <inttypes.h> header file. */
#cmakedefine HAVE_INTTYPES_H

/* we should have a file from which we can guess the KDE version */
#cmakedefine HAVE_KDEVERSION_H

/* Define if you have libjpeg */
#cmakedefine HAVE_LIBJPEG

/* Define to 1 if you have the `mad' library (-lmad). */
#cmakedefine HAVE_LIBMAD

/* Define to 1 if you have the `ogg' library (-logg). */
#cmakedefine HAVE_LIBOGG

/* Define if you have libpng */
#cmakedefine HAVE_LIBPNG

/* Define if you have a working libpthread (will enable threaded code) */
#cmakedefine HAVE_LIBPTHREAD

/* Define to 1 if you have the `vorbis' library (-lvorbis). */
#cmakedefine HAVE_LIBVORBIS

/* Define if you have libz */
#cmakedefine HAVE_LIBZ

/* Define to 1 if you have the <limits.h> header file. */
#cmakedefine HAVE_LIMITS_H

/* meminfo is needed for the memory manager */
#cmakedefine HAVE_MEMINFO

/* Define to 1 if you have the <memory.h> header file. */
#cmakedefine HAVE_MEMORY_H

/* used for creating swap files */
#cmakedefine HAVE_MKSTEMP

/* Define to 1 if you have the <ndir.h> header file, and it defines `DIR'. */
#cmakedefine HAVE_NDIR_H

/* Define if your system needs _NSGetEnviron to set up the environment */
#cmakedefine HAVE_NSGETENVIRON

/* support playback/recording via OSS */
#cmakedefine HAVE_OSS_SUPPORT

/* Define if you have the res_init function */
#cmakedefine HAVE_RES_INIT

/* Define to 1 if you have the <sched.h> header file. */
#cmakedefine HAVE_SCHED_H

/* Define if you have a STL implementation by SGI */
#cmakedefine HAVE_SGI_STL

/* Define to 1 if you have the `snprintf' function. */
#cmakedefine HAVE_SNPRINTF

/* Define to 1 if you have the <stdarg.h> header file. */
#cmakedefine HAVE_STDARG_H

/* Define to 1 if you have the <stddef.h> header file. */
#cmakedefine HAVE_STDDEF_H

/* Define to 1 if you have the <stdint.h> header file. */
#cmakedefine HAVE_STDINT_H

/* Define to 1 if you have the <stdio.h> header file. */
#cmakedefine HAVE_STDIO_H

/* Define to 1 if you have the <stdlib.h> header file. */
#cmakedefine HAVE_STDLIB_H

/* Define to 1 if you have the <stdsynthmodule.h> header file. */
#cmakedefine HAVE_STDSYNTHMODULE_H

/* Define to 1 if you have the <strings.h> header file. */
#cmakedefine HAVE_STRINGS_H

/* Define to 1 if you have the <string.h> header file. */
#cmakedefine HAVE_STRING_H

/* Define if you have strlcat */
#cmakedefine HAVE_STRLCAT

/* Define if you have the strlcat prototype */
#cmakedefine HAVE_STRLCAT_PROTO

/* Define if you have strlcpy */
#cmakedefine HAVE_STRLCPY

/* Define if you have the strlcpy prototype */
#cmakedefine HAVE_STRLCPY_PROTO

/* Define to 1 if you have the <sys/bitypes.h> header file. */
#cmakedefine HAVE_SYS_BITYPES_H

/* Define to 1 if you have the <sys/dir.h> header file, and it defines `DIR'.
   */
#cmakedefine HAVE_SYS_DIR_H

/* Define to 1 if you have the <sys/ioctl.h> header file. */
#cmakedefine HAVE_SYS_IOCTL_H

/* Define to 1 if you have the <sys/ndir.h> header file, and it defines `DIR'.
   */
#cmakedefine HAVE_SYS_NDIR_H

/* Define to 1 if you have the <sys/param.h> header file. */
#cmakedefine HAVE_SYS_PARAM_H

/* needed for OSS playback and recording */
#cmakedefine HAVE_SYS_SOUNDCARD_H

/* Define to 1 if you have the <sys/stat.h> header file. */
#cmakedefine HAVE_SYS_STAT_H

/* we can include <sys/times.h> */
#cmakedefine HAVE_SYS_TIMES_H

/* Define to 1 if you have the <sys/time.h> header file. */
#cmakedefine HAVE_SYS_TIME_H

/* Define to 1 if you have the <sys/types.h> header file. */
#cmakedefine HAVE_SYS_TYPES_H

/* Define to 1 if you have the <time.h> header file. */
#cmakedefine HAVE_TIME_H

/* Define to 1 if you have the <typeinfo> header file. */
#cmakedefine HAVE_TYPEINFO

/* Define to 1 if you have the <unistd.h> header file. */
#cmakedefine HAVE_UNISTD_H

/* used for unlinking swap files */
#cmakedefine HAVE_UNLINK

/* Define to 1 if you have the `vsnprintf' function. */
#cmakedefine HAVE_VSNPRINTF

/* Suffix for lib directories */
#cmakedefine KDELIBSUFF

/* Name of package */
#cmakedefine PACKAGE

/* Define to the address where bug reports for this package should be sent. */
#cmakedefine PACKAGE_BUGREPORT

/* Define to the full name of this package. */
#cmakedefine PACKAGE_NAME

/* Define to the full name and version of this package. */
#cmakedefine PACKAGE_STRING

/* Define to the one symbol short name of this package. */
#cmakedefine PACKAGE_TARNAME

/* Define to the version of this package. */
#cmakedefine PACKAGE_VERSION

/* The size of `char', as computed by sizeof. */
#cmakedefine SIZEOF_CHAR

/* The size of `char *', as computed by sizeof. */
#cmakedefine SIZEOF_CHAR_P

/* The size of `int', as computed by sizeof. */
#cmakedefine SIZEOF_INT

/* The size of `long', as computed by sizeof. */
#cmakedefine SIZEOF_LONG

/* The size of `short', as computed by sizeof. */
#cmakedefine SIZEOF_SHORT

/* The size of `size_t', as computed by sizeof. */
#cmakedefine SIZEOF_SIZE_T

/* The size of `unsigned long', as computed by sizeof. */
#cmakedefine SIZEOF_UNSIGNED_LONG

/* Define to 1 if you have the ANSI C header files. */
#cmakedefine STDC_HEADERS

/* Define to 1 if you can safely include both <sys/time.h> and <time.h>. */
#cmakedefine TIME_WITH_SYS_TIME

/* use own copy of libaudiofile */
#cmakedefine USE_BUILTIN_LIBAUDIOFILE

/* Version number of package */
#cmakedefine VERSION

#define PACKAGE "kwave"
#define PACKAGE_VERSION "v0.7.9-1"
