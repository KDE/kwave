/* config.h.  Generated automatically by configure.  */
/* config.h.in.  Generated automatically from configure.in by autoheader.  */

/* Define if on AIX 3.
   System headers sometimes define this.
   We just want to avoid a redefinition error message.  */
#ifndef _ALL_SOURCE
/* #undef _ALL_SOURCE */
#endif

/* Define if on MINIX.  */
/* #undef _MINIX */

/* Define if the system does not provide POSIX.1 features except
   with this defined.  */
/* #undef _POSIX_1_SOURCE */

/* Define if you need to in order for stat and other things to work.  */
/* #undef _POSIX_SOURCE */

/* Define if you have the ANSI C header files.  */
#define STDC_HEADERS 1

/* Define if you can safely include both <sys/time.h> and <time.h>.  */
#define TIME_WITH_SYS_TIME 1

/* Define if the C++ compiler supports BOOL */
#define HAVE_BOOL 1

#define VERSION "0.25"

#define PACKAGE "kwave"

/* defines if having libgif (always 1) */
#define HAVE_LIBGIF 1

/* defines if having libjpeg (always 1) */
#define HAVE_LIBJPEG 1

/* defines which to take for ksize_t */
#define ksize_t int

/* define if you have setenv */
#define HAVE_FUNC_SETENV 1

/* Define to 1 if NLS is requested.  */
#define ENABLE_NLS 1

/* Define if you need the GNU extensions to compile */
#define _GNU_SOURCE 1

/* Define if you have the getdomainname function.  */
#define HAVE_GETDOMAINNAME 1

/* Define if you have the socket function.  */
#define HAVE_SOCKET 1

/* Define if you have the vsnprintf function.  */
#define HAVE_VSNPRINTF 1

/* Define if you have the <dirent.h> header file.  */
#define HAVE_DIRENT_H 1

/* Define if you have the <fcntl.h> header file.  */
#define HAVE_FCNTL_H 1

/* Define if you have the <fnmatch.h> header file.  */
#define HAVE_FNMATCH_H 1

/* Define if you have the <ndir.h> header file.  */
/* #undef HAVE_NDIR_H */

/* Define if you have the <strings.h> header file.  */
#define HAVE_STRINGS_H 1

/* Define if you have the <sys/cdefs.h> header file.  */
#define HAVE_SYS_CDEFS_H 1

/* Define if you have the <sys/dir.h> header file.  */
/* #undef HAVE_SYS_DIR_H */

/* Define if you have the <sys/ndir.h> header file.  */
/* #undef HAVE_SYS_NDIR_H */

/* Define if you have the <sys/stat.h> header file.  */
#define HAVE_SYS_STAT_H 1

/* Define if you have the <sys/time.h> header file.  */
#define HAVE_SYS_TIME_H 1

/* Define if you have the <sysent.h> header file.  */
#define HAVE_SYSENT_H 1

/* Define if you have the <unistd.h> header file.  */
#define HAVE_UNISTD_H 1

#ifndef HAVE_BOOL
#define HAVE_BOOL
typedef int bool;
const bool false = 0;
const bool true = 1;
#endif
