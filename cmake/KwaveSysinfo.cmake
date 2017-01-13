#############################################################################
##    Kwave                - cmake/KwaveSysinfo.cmake
##                           -------------------
##    begin                : Sat Dec 19 2009
##    copyright            : (C) 2009 by Thomas Eschenbacher
##    email                : Thomas.Eschenbacher@gmx.de
#############################################################################
#
#############################################################################
#                                                                           #
# Redistribution and use in source and binary forms, with or without        #
# modification, are permitted provided that the following conditions        #
# are met:                                                                  #
#                                                                           #
# 1. Redistributions of source code must retain the above copyright         #
#    notice, this list of conditions and the following disclaimer.          #
# 2. Redistributions in binary form must reproduce the above copyright      #
#    notice, this list of conditions and the following disclaimer in the    #
#    documentation and/or other materials provided with the distribution.   #
#                                                                           #
# For details see the accompanying cmake/COPYING-CMAKE-SCRIPTS file.        #
#                                                                           #
#############################################################################

SET(_try_src_dir "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp")
SET(_try_sysinfo "${_try_src_dir}/cmake_try_sysinfo.c")
WRITE_FILE("${_try_sysinfo}" "
    #include <sys/sysinfo.h>
    #include <linux/kernel.h>
    int main()
    {
        struct sysinfo info;
        unsigned int total;
        sysinfo(&info);
        total = info.totalram;
        return 0;
    }
")
TRY_COMPILE(HAVE_SYSINFO ${CMAKE_BINARY_DIR} ${_try_sysinfo} OUTPUT_VARIABLE _out)
IF (HAVE_SYSINFO)
    SET(_try_src_dir "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp")
    SET(_try_sysinfo_memunit "${_try_src_dir}/cmake_try_sysinfo_memunit.c")
    WRITE_FILE("${_try_sysinfo_memunit}" "
	#include <sys/sysinfo.h>
	#include <linux/kernel.h>
	int main()
	{
	    struct sysinfo info;
	    unsigned int total;
	    sysinfo(&info);
	    total = info.totalram * info.mem_unit;
	    return 0;
	}
    ")
    TRY_COMPILE(HAVE_SYSINFO_MEMUNIT ${CMAKE_BINARY_DIR} ${_try_sysinfo_memunit} OUTPUT_VARIABLE _out)
ELSE (HAVE_SYSINFO)
    MESSAGE(STATUS "unable to get memory information through 'sysinfo'")
    MESSAGE(STATUS
        "!!! You might get inaccurate behaviour of the memory management !!!")
ENDIF(HAVE_SYSINFO)

CHECK_FUNCTION_EXISTS(getrlimit HAVE_GETRLIMIT)
CHECK_FUNCTION_EXISTS(getpagesize HAVE_GETPAGESIZE)
CHECK_FUNCTION_EXISTS(sysconf HAVE_SYSCONF)

#############################################################################
#############################################################################
