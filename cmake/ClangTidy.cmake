#############################################################################
##    Kwave                - cmake/ClangTidy.cmake
##                           -------------------
##    begin                : Fri Sep 11 2026
##    copyright            : (C) 2026 by Thomas Eschenbacher
##    email                : Thomas.Eschenbacher@gmx.de
#############################################################################
#
#############################################################################
# SPDX-License-Identifier: BSD-3-Clause                                     #
#############################################################################

INCLUDE(FindRequiredProgram)

# enable export of compile_commands.json for clang-tidy and LSP servers
SET(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "Export compile commands" FORCE)

# option to enable or disable clang-tidy during build
OPTION(WITH_CLANG_TIDY "Enable clang-tidy static analysis during build" OFF)

IF(WITH_CLANG_TIDY)
    FIND_REQUIRED_PROGRAM(CLANG_TIDY_EXE NAMES "clang-tidy")
    SET(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
ENDIF(WITH_CLANG_TIDY)

#############################################################################
#############################################################################
