/* acconfig.h
   This file is in the public domain.

   Descriptive text for the C preprocessor macros that
   the distributed Autoconf macros can define.
   No software package will use all of them; autoheader copies the ones
   your configure.in uses into your configuration header file templates.

   The entries are in sort -df order: alphabetical, case insensitive,
   ignoring punctuation (such as underscores).  Although this order
   can split up related entries, it makes it easier to check whether
   a given entry is in the file.

   Leave the following blank line there!!  Autoheader needs it.  */



/* check for demangle.h */
#undef HAVE_DEMANGLE_H

/* check for geteuid */
#undef HAVE_GETEUID

/* check for sys/resource.h and getrlimit */
#undef HAVE_GETRLIMIT

/* check for kdeversion.h */
#undef HAVE_KDEVERSION_H

/* check for ability to create temporary files with mkstemp(...) */
#undef HAVE_MKSTEMP

/* check for sysinfo to query total ram */
#undef HAVE_MEMINFO

/* check for unlink */
#undef HAVE_UNLINK


/* Leave that blank line there!!  Autoheader needs it.
   If you're adding to this file, keep in mind:
   The entries are in sort -df order: alphabetical, case insensitive,
   ignoring punctuation (such as underscores).  */
