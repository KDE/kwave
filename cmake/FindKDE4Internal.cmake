# - Find the KDE4 include and library dirs, KDE preprocessors and define a some macros
#
# This module defines the following variables:
#
# KDE4_FOUND               - set to TRUE if everything required for building KDE software has been found
#
# KDE4_DEFINITIONS         - compiler definitions required for compiling KDE software
# KDE4_INCLUDE_DIR         - the KDE 4 include directory
# KDE4_INCLUDES            - all include directories required for KDE, i.e.
#                            KDE4_INCLUDE_DIR, but also the Qt4 include directories
#                            and other platform specific include directories
# KDE4_LIB_DIR             - the directory where the KDE libraries are installed,
#                            intended to be used with LINK_DIRECTORIES()
#
# The following variables are defined for the various tools required to
# compile KDE software:
#
# KDE4_KCFGC_EXECUTABLE    - the kconfig_compiler executable
# KDE4_AUTOMOC_EXECUTABLE  - the kde4automoc executable
# KDE4_MEINPROC_EXECUTABLE - the meinproc4 executable
# KDE4_MAKEKDEWIDGETS_EXECUTABLE - the makekdewidgets executable
#
# The following variables point to the location of the KDE libraries,
# but shouldn't be used directly:
#
# KDE4_KDECORE_LIBRARY     - the kdecore library
# KDE4_KDEUI_LIBRARY       - the kdeui library
# KDE4_KIO_LIBRARY         - the kio library
# KDE4_KPARTS_LIBRARY      - the kparts library
# KDE4_KUTILS_LIBRARY      - the kutils library
# KDE4_KDE3SUPPORT_LIBRARY - the kde3support library
# KDE4_KFILE_LIBRARY       - the kfile library
# KDE4_KHTML_LIBRARY       - the khtml library
# KDE4_KJS_LIBRARY         - the kjs library
# KDE4_KNEWSTUFF2_LIBRARY  - the knewstuff2 library
# KDE4_KDNSSD_LIBRARY      - the kdnssd library
# KDE4_PHONON_LIBRARY      - the phonon library
# KDE4_KAUDIODEVICELIST_LIBRARY - the kaudiodevicelist library
# KDE4_THREADWEAVER_LIBRARY- the threadweaver library
# KDE4_SOLID_LIBRARY       - the solid library
# KDE4_KNOTIFYCONFIG_LIBRARY- the knotifyconfig library
# KDE4_KROSSCORE_LIBRARY   - the krosscore library
# KDE4_KTEXTEDITOR_LIBRARY - the ktexteditor library
# KDE4_KNEPOMUK_LIBRARY    - the knepomuk library
# KDE4_KMETADATA_LIBRARY   - the kmetadata library
#
# Compared to the variables above, the following variables
# also contain all of the depending libraries, so the variables below
# should be used instead of the ones above:
#
# KDE4_KDECORE_LIBS          - the kdecore library and all depending libraries
# KDE4_KDEUI_LIBS            - the kdeui library and all depending libraries
# KDE4_KIO_LIBS              - the kio library and all depending libraries
# KDE4_KPARTS_LIBS           - the kparts library and all depending libraries
# KDE4_KUTILS_LIBS           - the kutils library and all depending libraries
# KDE4_KDE3SUPPORT_LIBS      - the kde3support library and all depending libraries
# KDE4_KFILE_LIBS            - the kfile library and all depending libraries
# KDE4_KHTML_LIBS            - the khtml library and all depending libraries
# KDE4_KJS_LIBS              - the kjs library and all depending libraries
# KDE4_KNEWSTUFF2_LIBS       - the knewstuff2 library and all depending libraries
# KDE4_KDNSSD_LIBS           - the kdnssd library and all depending libraries
# KDE4_KDESU_LIBS            - the kdesu library and all depending libraries
# KDE4_KPTY_LIBS             - the kpty library and all depending libraries
# KDE4_PHONON_LIBS           - the phonon library and all depending librairies
# KDE4_THREADWEAVER_LIBRARIES- the threadweaver library and all depending libraries
# KDE4_SOLID_LIBS            - the solid library and all depending libraries
# KDE4_KNOTIFYCONFIG_LIBS    - the knotify config library and all depending libraries
# KDE4_KROSSCORE_LIBS        - the kross core library and all depending libraries
# KDE4_KROSSUI_LIBS          - the kross ui library which includes core and all depending libraries
# KDE4_KTEXTEDITOR_LIBS      - the ktexteditor library and all depending libraries
# KDE4_KNEPOMUK_LIBS         - the knepomuk library and all depending libraries
# KDE4_KMETADATA_LIBS        - the kmetadata library and all depending libraries
#
# This module defines a bunch of variables used as locations
# for install directories. They are all interpreted relative
# to CMAKE_INSTALL_PREFIX
#
# BIN_INSTALL_DIR          - the directory where executables be installed (default is prefix/bin)
# SBIN_INSTALL_DIR         - the directory where system executables will be installed (default is prefix/sbin)
# LIB_INSTALL_DIR          - the directory where libraries will be installed (default is prefix/lib)
# CONFIG_INSTALL_DIR       - the config file install dir
# DATA_INSTALL_DIR         - the parent directory where applications can install their data
# HTML_INSTALL_DIR         - the HTML install dir for documentation
# ICON_INSTALL_DIR         - the icon install dir (default prefix/share/icons/)
# INFO_INSTALL_DIR         - the kde info install dir (default prefix/info)
# KCFG_INSTALL_DIR         - the install dir for kconfig files
# LOCALE_INSTALL_DIR       - the install dir for translations
# MAN_INSTALL_DIR          - the kde man page install dir (default prefix/man/)
# MIME_INSTALL_DIR         - the install dir for the mimetype desktop files
# PLUGIN_INSTALL_DIR       - the subdirectory relative to the install prefix where plugins will be installed (default is ${KDE4_LIB_INSTALL_DIR}/kde4)
# SERVICES_INSTALL_DIR     - the install dir for service (desktop, protocol, ...) files
# SERVICETYPES_INSTALL_DIR - the install dir for servicestypes desktop files
# SOUND_INSTALL_DIR        - the install dir for sound files
# TEMPLATES_INSTALL_DIR    - the install dir for templates (Create new file...)
# WALLPAPER_INSTALL_DIR    - the install dir for wallpapers
# KCONF_UPDATE_INSTALL_DIR - the kconf_update install dir
# XDG_APPS_INSTALL_DIR     - the XDG apps dir
# XDG_DIRECTORY_INSTALL_DIR- the XDG directory
# XDG_MIME_INSTALL_DIR     - the XDG mimetypes install dir
# DBUS_INTERFACES_INSTALL_DIR - the directory where dbus interfaces be installed (default is prefix/share/dbus-1/interfaces)
# DBUS_SERVICES_INSTALL_DIR        - the directory where dbus services be installed (default is prefix/share/dbus-1/services )
#
# The following variable is provided, but seem to be unused:
# LIBS_HTML_INSTALL_DIR    /share/doc/HTML            CACHE STRING "Is this still used ?")
#
# The following user adjustable options are provided:
#
# KDE4_ENABLE_FINAL - enable KDE-style enable-final all-in-one-compilation
# KDE4_BUILD_TESTS  - enable this to build the testcases
# KDE4_ENABLE_FPIE  - enable it to use gcc Position Independent Executables feature
#
# It also adds the following macros (from KDE4Macros.cmake)
# KDE4_ADD_UI_FILES (SRCS_VAR file1.ui ... fileN.ui)
#    Use this to add Qt designer ui files to your application/library.
#
# KDE4_ADD_UI3_FILES (SRCS_VAR file1.ui ... fileN.ui)
#    Use this to add Qt designer ui files from Qt version 3 to your application/library.
#
# KDE4_ADD_KCFG_FILES (SRCS_VAR [GENERATE_MOC] file1.kcfgc ... fileN.kcfgc)
#    Use this to add KDE config compiler files to your application/library.
#    Use optional GENERATE_MOC to generate moc if you use signals in your kcfg files.
#
# KDE4_ADD_WIDGET_FILES (SRCS_VAR file1.widgets ... fileN.widgets)
#    Use this to add widget description files for the makekdewidgets code generator
#    for Qt Designer plugins.
#
# KDE4_CREATE_FINAL_FILES (filename_CXX filename_C file1 ... fileN)
#    This macro is intended mainly for internal uses.
#    It is used for enable-final. It will generate two source files,
#    one for the C files and one for the C++ files.
#    These files will have the names given in filename_CXX and filename_C.
#
# KDE4_ADD_PLUGIN ( name [WITH_PREFIX] file1 ... fileN )
#    Create a KDE plugin (KPart, kioslave, etc.) from the given source files.
#    It supports KDE4_ENABLE_FINAL.
#    If WITH_PREFIX is given, the resulting plugin will have the prefix "lib", otherwise it won't.
#    It creates and installs an appropriate libtool la-file.
#
# KDE4_ADD_KDEINIT_EXECUTABLE (name [NOGUI] [RUN_UNINSTALLED] file1 ... fileN)
#    Create a KDE application in the form of a module loadable via kdeinit.
#    A library named kdeinit_<name> will be created and a small executable which links to it.
#    It supports KDE4_ENABLE_FINAL
#    If the executable has to be run from the buildtree (e.g. unit tests and code generators
#    used later on when compiling), set the option RUN_UNINSTALLED.
#    If the executable doesn't have a GUI, use the option NOGUI. By default on OS X
#    application bundles are created, with the NOGUI option no bundles but simple executables
#    are created. Currently it doesn't have any effect on other platforms.
#
# KDE4_ADD_EXECUTABLE (name [NOGUI] [RUN_UNINSTALLED] file1 ... fileN)
#    Equivalent to ADD_EXECUTABLE(), but additionally adds some more features:
#    -support for KDE4_ENABLE_FINAL
#    -support for automoc
#    -automatic RPATH handling
#    If the executable has to be run from the buildtree (e.g. unit tests and code generators
#    used later on when compiling), set the option RUN_UNINSTALLED.
#    If the executable doesn't have a GUI, use the option NOGUI. By default on OS X
#    application bundles are created, with the NOGUI option no bundles but simple executables
#    are created. Currently it doesn't have any effect on other platforms.
#
# KDE4_ADD_LIBRARY (name [STATIC | SHARED | MODULE ] file1 ... fileN)
#    Equivalent to ADD_LIBRARY(), but additionally it supports KDE4_ENABLE_FINAL
#    and under Windows it adds a -DMAKE_<name>_LIB definition to the compilation.
#
# KDE4_ADD_UNIT_TEST (testname [TESTNAME targetname] file1 ... fileN)
#    add a unit test, which is executed when running make test
#    it will be built with RPATH poiting to the build dir
#    The targets are always created, but only built for the "all"
#    target if the option KDE4_BUILD_TESTS is enabled. Otherwise the rules for the target
#    are created but not built by default. You can build them by manually building the target.
#    The name of the target can be specified using TESTNAME <targetname>, if it is not given
#    the macro will default to the <testname>
#
# KDE4_ADD_TEST_EXECUTABLE (name file1 ... fileN)
#    add an test executable
#    it will be built with RPATH poiting to the build dir
#    The targets are always created, but only built for the "all"
#    target if the option KDE4_BUILD_TESTS is enabled. Otherwise the rules for the target
#    are created but not built by default. You can build them by manually building the target.
#    KDESRCDIR is set to the source directory of the test, this can be used with
#    KGlobal::dirs()->addResourceDir( "data", KDESRCDIR ); to be able to use xmlgui
#    and other things in the test, that normally require installation
#
# KDE4_UPDATE_ICONCACHE()
#    Notifies the icon cache that new icons have been installed by updating
#    mtime of ${ICON_INSTALL_DIR}/hicolor directory.
#
# KDE4_INSTALL_ICONS( path theme)
#    Installs all png and svgz files in the current directory to the icon
#    directoy given in path, in the subdirectory for the given icon theme.
#
# KDE4_CREATE_HANDBOOK( docbookfile [INSTALL_DESTINATION installdest] [SUBDIR subdir])
#   Create the handbook from the docbookfile (using meinproc4)
#   The resulting handbook will be installed to <installdest> when using
#   INSTALL_DESTINATION <installdest>, or to <installdest>/<subdir> if
#   SUBDIR <subdir> is specified.
#
# KDE4_CREATE_MANPAGE( docbookfile section )
#   Create the manpage for the specified section from the docbookfile (using meinproc4)
#   The resulting manpage will be installed to <installdest> when using
#   INSTALL_DESTINATION <installdest>, or to <installdest>/<subdir> if
#   SUBDIR <subdir> is specified.
#
# _KDE4_PLATFORM_INCLUDE_DIRS is used only internally
# _KDE4_PLATFORM_DEFINITIONS is used only internally
#
#
# Copyright (c) 2006, Alexander Neundorf <neundorf@kde.org>
# Copyright (c) 2006, Laurent Montel, <montel@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(KDE4_FOUND)
  # Already found in this cmake run, nothing more to do
else(KDE4_FOUND)

include (MacroEnsureVersion)

cmake_minimum_required(VERSION 2.4.5 FATAL_ERROR)

set(QT_MIN_VERSION "4.3.0")
#this line includes FindQt4.cmake, which searches the Qt library and headers
find_package(Qt4 REQUIRED)

if (NOT QT_DBUSXML2CPP_EXECUTABLE)
    message(FATAL_ERROR "Qt4 qdbusxml2cpp was not found. Make sure it has been built and installed by Qt")
endif (NOT QT_DBUSXML2CPP_EXECUTABLE)


# Perl is required for building KDE software,
find_package(Perl REQUIRED)

include (MacroLibrary)
include (CheckCXXCompilerFlag)
include (CheckCXXSourceCompiles)


# get the directory of the current file, used later on in the file
get_filename_component( kde_cmake_module_dir  ${CMAKE_CURRENT_LIST_FILE} PATH)

# are we trying to compile kdelibs ? kdelibs_SOURCE_DIR comes from "project(kdelibs)" in kdelibs/CMakeLists.txt
# then enter bootstrap mode

if(kdelibs_SOURCE_DIR)
   set(_kdeBootStrapping TRUE)
   message(STATUS "Building kdelibs...")
else(kdelibs_SOURCE_DIR)
   set(_kdeBootStrapping FALSE)
endif(kdelibs_SOURCE_DIR)

#######################  #now try to find some kde stuff  ################################

if (_kdeBootStrapping)
   set(KDE4_INCLUDE_DIR ${kdelibs_SOURCE_DIR})
   set(KDE4_KDECORE_LIBS ${QT_QTCORE_LIBRARY} kdecore)
   set(KDE4_KDEUI_LIBS ${KDE4_KDECORE_LIBS} kdeui)
   set(KDE4_KIO_LIBS ${KDE4_KDEUI_LIBS} kio)
   set(KDE4_KPARTS_LIBS ${KDE4_KIO_LIBS} kparts)
   if (UNIX)
      set(KDE4_KPTY_LIBS ${KDE4_KDECORE_LIBS} kpty)
   endif (UNIX)
   set(KDE4_KUTILS_LIBS ${KDE4_KIO_LIBS} kutils)
   set(KDE4_KDE3SUPPORT_LIBS ${KDE4_KIO_LIBS} kde3support)
   set(KDE4_SOLID_LIBS ${KDE4_KDECORE_LIBS} solid)
   set(KDE4_PHONON_LIBS ${QT_QTDBUS_LIBRARY} ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} phonon)
   set(KDE4_KAUDIODEVICELIST_LIBS ${KDE4_SOLID_LIBS} kaudiodevicelist)
   set(KDE4_KFILE_LIBS ${KDE4_KDE3SUPPORT_LIBS} kfile)
   set(KDE4_KHTML_LIBS ${KDE4_KPARTS_LIBS} khtml)

   set(EXECUTABLE_OUTPUT_PATH ${kdelibs_BINARY_DIR}/bin )

   if (WIN32)
      set(LIBRARY_OUTPUT_PATH            ${EXECUTABLE_OUTPUT_PATH} )
      # CMAKE_CFG_INTDIR is the output subdirectory created e.g. by XCode and MSVC
      set(KDE4_KCFGC_EXECUTABLE          ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}/kconfig_compiler )
      set(KDE4_AUTOMOC_EXECUTABLE        ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}/kde4automoc )
      set(KDE4_MEINPROC_EXECUTABLE       ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}/meinproc4 )
      set(KDE4_MAKEKDEWIDGETS_EXECUTABLE ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}/makekdewidgets )
   else (WIN32)
      set(LIBRARY_OUTPUT_PATH            ${CMAKE_BINARY_DIR}/lib )
      set(KDE4_KCFGC_EXECUTABLE          ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}/kconfig_compiler.shell )
      set(KDE4_AUTOMOC_EXECUTABLE        ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}/kde4automoc.shell )
      set(KDE4_MEINPROC_EXECUTABLE       ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}/meinproc4.shell )
      set(KDE4_MAKEKDEWIDGETS_EXECUTABLE ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}/makekdewidgets.shell )
   endif (WIN32)

   set(KDE4_LIB_DIR ${LIBRARY_OUTPUT_PATH}/${CMAKE_CFG_INTDIR})

   # when building kdelibs, make the kcfg rules depend on the binaries...
   set( _KDE4_KCONFIG_COMPILER_DEP kconfig_compiler)
   set( _KDE4_AUTOMOC_EXECUTABLE_DEP kde4automoc)
   set( _KDE4_MAKEKDEWIDGETS_DEP makekdewidgets)
   set( _KDE4_MEINPROC_EXECUTABLE_DEP meinproc4)

   set(KDE4_INSTALLED_VERSION_OK TRUE)

else (_kdeBootStrapping)

  # ... but NOT otherwise
   set( _KDE4_KCONFIG_COMPILER_DEP)
   set( _KDE4_AUTOMOC_EXECUTABLE_DEP)
   set( _KDE4_MAKEKDEWIDGETS_DEP)
   set( _KDE4_MEINPROC_EXECUTABLE_DEP)

   # Check the version of kde. KDE4_KDECONFIG_EXECUTABLE was set by FindKDE4
   exec_program(${KDE4_KDECONFIG_EXECUTABLE} ARGS "--version" OUTPUT_VARIABLE kdeconfig_output )

   string(REGEX MATCH "KDE: [0-9]+\\.[0-9]+\\.[0-9]+" KDEVERSION "${kdeconfig_output}")
   if (KDEVERSION)

      string(REGEX REPLACE "^KDE: " "" KDEVERSION "${KDEVERSION}")

      # we need at least this version:
      if (NOT KDE_MIN_VERSION)
         set(KDE_MIN_VERSION "3.9.0")
      endif (NOT KDE_MIN_VERSION)

      #message(STATUS "KDE_MIN_VERSION=${KDE_MIN_VERSION}  found ${KDEVERSION}")
      macro_ensure_version( ${KDE_MIN_VERSION} ${KDEVERSION} KDE4_INSTALLED_VERSION_OK )

   else (KDEVERSION)
      message(FATAL_ERROR "Couldn't parse KDE version string from the kde4-config output:\n${kdeconfig_output}")
   endif (KDEVERSION)

   set(LIBRARY_OUTPUT_PATH  ${CMAKE_BINARY_DIR}/lib )

   if (WIN32)
      # we don't want to be forced to set two paths into the build tree
      set(LIBRARY_OUTPUT_PATH  ${CMAKE_BINARY_DIR}/bin )

      # on win32 the install dir is determined on runtime not install time
      # KDELIBS_INSTALL_DIR and QT_INSTALL_DIR are used in KDELibsDependencies.cmake to setup
      # kde install paths and library dependencies
      get_filename_component(_DIR ${KDE4_KDECONFIG_EXECUTABLE} PATH )
      get_filename_component(KDE4_INSTALL_DIR ${_DIR} PATH )
      get_filename_component(_DIR ${QT_QMAKE_EXECUTABLE} PATH )
      get_filename_component(QT_INSTALL_DIR ${_DIR} PATH )
   endif (WIN32)

   # this file contains all dependencies of all libraries of kdelibs, Alex
   include(${kde_cmake_module_dir}/KDELibsDependencies.cmake)

   find_library(KDE4_KDEFAKES_LIBRARY NAMES kdefakes PATHS ${KDE4_LIB_INSTALL_DIR} NO_DEFAULT_PATH )
   set(KDE4_KDEFAKES_LIBS ${kdefakes_LIB_DEPENDS} ${KDE4_KDEFAKES_LIBRARY} )

   find_library(KDE4_KDECORE_LIBRARY NAMES kdecore PATHS ${KDE4_LIB_INSTALL_DIR} NO_DEFAULT_PATH )
   set(KDE4_KDECORE_LIBS ${kdecore_LIB_DEPENDS} ${KDE4_KDECORE_LIBRARY} )

   find_library(KDE4_KDEUI_LIBRARY NAMES kdeui PATHS ${KDE4_LIB_INSTALL_DIR} NO_DEFAULT_PATH )
   set(KDE4_KDEUI_LIBS ${kdeui_LIB_DEPENDS} ${KDE4_KDEUI_LIBRARY} )

   find_library(KDE4_KIO_LIBRARY NAMES kio PATHS ${KDE4_LIB_INSTALL_DIR} NO_DEFAULT_PATH )
   set(KDE4_KIO_LIBS ${kio_LIB_DEPENDS} ${KDE4_KIO_LIBRARY} )

   find_library(KDE4_KPARTS_LIBRARY NAMES kparts PATHS ${KDE4_LIB_INSTALL_DIR} NO_DEFAULT_PATH )
   set(KDE4_KPARTS_LIBS ${kparts_LIB_DEPENDS} ${KDE4_KPARTS_LIBRARY} )

   find_library(KDE4_KUTILS_LIBRARY NAMES kutils PATHS ${KDE4_LIB_INSTALL_DIR} NO_DEFAULT_PATH )
   set(KDE4_KUTILS_LIBS ${kutils_LIB_DEPENDS} ${KDE4_KUTILS_LIBRARY} )

   find_library(KDE4_KDE3SUPPORT_LIBRARY NAMES kde3support PATHS ${KDE4_LIB_INSTALL_DIR} NO_DEFAULT_PATH )
   set(KDE4_KDE3SUPPORT_LIBS ${kde3support_LIB_DEPENDS} ${KDE4_KDE3SUPPORT_LIBRARY} )

   find_library(KDE4_KFILE_LIBRARY NAMES kfile PATHS ${KDE4_LIB_INSTALL_DIR} NO_DEFAULT_PATH )
   set(KDE4_KFILE_LIBS ${kfile_LIB_DEPENDS} ${KDE4_KFILE_LIBRARY} )

   find_library(KDE4_KHTML_LIBRARY NAMES khtml PATHS ${KDE4_LIB_INSTALL_DIR} NO_DEFAULT_PATH )
   set(KDE4_KHTML_LIBS ${khtml_LIB_DEPENDS} ${KDE4_KHTML_LIBRARY} )

   find_library(KDE4_KJS_LIBRARY NAMES kjs PATHS ${KDE4_LIB_INSTALL_DIR} NO_DEFAULT_PATH )
   set(KDE4_KJS_LIBS ${kjs_LIB_DEPENDS} ${KDE4_KJS_LIBRARY} )

   find_library(KDE4_KNEWSTUFF2_LIBRARY NAMES knewstuff2 PATHS ${KDE4_LIB_INSTALL_DIR} NO_DEFAULT_PATH )
   set(KDE4_KNEWSTUFF2_LIBS ${knewstuff2_LIB_DEPENDS} ${KDE4_KNEWSTUFF2_LIBRARY} )

   if (UNIX)
      find_library(KDE4_KPTY_LIBRARY NAMES kpty PATHS ${KDE4_LIB_INSTALL_DIR} NO_DEFAULT_PATH )
      set(KDE4_KPTY_LIBS ${kpty_LIB_DEPENDS} ${KDE4_KPTY_LIBRARY} )

      find_library(KDE4_KDESU_LIBRARY NAMES kdesu PATHS ${KDE4_LIB_INSTALL_DIR} NO_DEFAULT_PATH )
      set(KDE4_KDESU_LIBS ${kdesu_LIB_DEPENDS} ${KDE4_KDESU_LIBRARY} )
   endif (UNIX)

   find_library(KDE4_KDNSSD_LIBRARY NAMES kdnssd PATHS ${KDE4_LIB_INSTALL_DIR} NO_DEFAULT_PATH )
   set(KDE4_KDNSSD_LIBS ${kdnssd_LIB_DEPENDS} ${KDE4_KDNSSD_LIBRARY} )

   find_library(KDE4_PHONON_LIBRARY NAMES phonon PATHS ${KDE4_LIB_INSTALL_DIR} NO_DEFAULT_PATH )
   set(KDE4_PHONON_LIBS ${phonon_LIB_DEPENDS} ${KDE4_PHONON_LIBRARY} )

   find_library(KDE4_KAUDIODEVICELIST_LIBRARY NAMES kaudiodevicelist PATHS ${KDE4_LIB_INSTALL_DIR} NO_DEFAULT_PATH )
   set(KDE4_KAUDIODEVICELIST_LIBS ${kaudiodevicelist_LIB_DEPENDS} ${KDE4_KAUDIODEVICELIST_LIBRARY} )

   find_library(KDE4_SOLID_LIBRARY NAMES solid PATHS ${KDE4_LIB_INSTALL_DIR} NO_DEFAULT_PATH )
   set(KDE4_SOLID_LIBS ${solid_LIB_DEPENDS} ${KDE4_SOLID_LIBRARY} )

   find_library(KDE4_THREADWEAVER_LIBRARY NAMES threadweaver PATHS ${KDE4_LIB_INSTALL_DIR} NO_DEFAULT_PATH )
   set(KDE4_THREADWEAVER_LIBRARIES ${threadweaver_LIB_DEPENDS} ${KDE4_THREADWEAVER_LIBRARY} )

   find_library(KDE4_KNOTIFYCONFIG_LIBRARY NAMES knotifyconfig PATHS ${KDE4_LIB_INSTALL_DIR} NO_DEFAULT_PATH )
   set(KDE4_KNOTIFYCONFIG_LIBS ${knotifyconfig_LIB_DEPENDS} ${KDE4_KNOTIFYCONFIG_LIBRARY} )

   find_library(KDE4_KROSSCORE_LIBRARY NAMES krosscore PATHS ${KDE4_LIB_INSTALL_DIR} NO_DEFAULT_PATH )
   set(KDE4_KROSSCORE_LIBS ${krosscore_LIB_DEPENDS} ${KDE4_KROSSCORE_LIBRARY} )

   find_library(KDE4_KROSSUI_LIBRARY NAMES krossui PATHS ${KDE4_LIB_INSTALL_DIR} NO_DEFAULT_PATH )
   set(KDE4_KROSSUI_LIBS ${krossui_LIB_DEPENDS} ${KDE4_KROSSCORE_LIBS} ${KDE4_KROSSUI_LIBRARY} )

   find_library(KDE4_KTEXTEDITOR_LIBRARY NAMES ktexteditor PATHS ${KDE4_LIB_INSTALL_DIR} NO_DEFAULT_PATH )
   set(KDE4_KTEXTEDITOR_LIBS ${ktexteditor_LIB_DEPENDS} ${KDE4_KTEXTEDITOR_LIBRARY} )

   # Can't do that, it's not always compiled.
   #find_library(KDE4_KNEPOMUK_LIBRARY NAMES knepomuk PATHS ${KDE4_LIB_INSTALL_DIR} NO_DEFAULT_PATH )
   #set(KDE4_KNEPOMUK_LIBS ${knepomuk_LIB_DEPENDS} ${KDE4_KNEPOMUK_LIBRARY} )

   # Can't do that, it's not always compiled. See FindKMetaData.cmake
   #find_library(KDE4_KMETADATA_LIBRARY NAMES kmetadata PATHS ${KDE4_LIB_INSTALL_DIR} NO_DEFAULT_PATH )
   #set(KDE4_KMETADATA_LIBS ${kmetadata_LIB_DEPENDS} ${KDE4_KMETADATA_LIBRARY} )

   get_filename_component(KDE4_LIB_DIR ${KDE4_KDECORE_LIBRARY} PATH )

   # kpassworddialog.h is new with KDE4
   # KDE4_INCLUDE_INSTALL_DIR is defined by KDELibsDependencies.cmake
   find_path(KDE4_INCLUDE_DIR kpassworddialog.h ${KDE4_INCLUDE_INSTALL_DIR} NO_DEFAULT_PATH )

   # at first look in LIBEXEC_INSTALL_DIR and no default paths,
   # if this didn't succeed, the second call makes cmake search again, but in the standard paths
   find_program(KDE4_KCFGC_EXECUTABLE NAME kconfig_compiler PATHS ${KDE4_BIN_INSTALL_DIR} NO_DEFAULT_PATH )
   find_program(KDE4_KCFGC_EXECUTABLE NAME kconfig_compiler )

   find_program(KDE4_AUTOMOC_EXECUTABLE NAME kde4automoc PATHS ${KDE4_BIN_INSTALL_DIR} NO_DEFAULT_PATH )
   find_program(KDE4_AUTOMOC_EXECUTABLE NAME kde4automoc )

   find_program(KDE4_MEINPROC_EXECUTABLE NAME meinproc4 PATHS ${KDE4_BIN_INSTALL_DIR} NO_DEFAULT_PATH )
   find_program(KDE4_MEINPROC_EXECUTABLE NAME meinproc4 )

   find_program(KDE4_MAKEKDEWIDGETS_EXECUTABLE NAME makekdewidgets PATHS ${KDE4_BIN_INSTALL_DIR} NO_DEFAULT_PATH )
   find_program(KDE4_MAKEKDEWIDGETS_EXECUTABLE NAME makekdewidgets )

   # allow searching cmake modules in all given kde install locations (KDEDIRS based)
   execute_process(COMMAND "${KDE4_KDECONFIG_EXECUTABLE}" --path data OUTPUT_VARIABLE _data_DIR ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
   file(TO_CMAKE_PATH "${_data_DIR}" _data_DIR)
   foreach(dir ${_data_DIR})
      set (apath "${dir}/cmake/modules")
      if (EXISTS "${apath}")
         set (included 0)
         string(TOLOWER "${apath}" _apath)
         # ignore already added pathes, case insensitive
         foreach(adir ${CMAKE_MODULE_PATH})
            string(TOLOWER "${adir}" _adir)
            if ("${_adir}" STREQUAL "${_apath}")
               set (included 1)
            endif ("${_adir}" STREQUAL "${_apath}")
         endforeach(adir)
         if (NOT included)
            message(STATUS "Adding ${apath} to CMAKE_MODULE_PATH")
            set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${apath}")
         endif (NOT included)
      endif (EXISTS "${apath}")
   endforeach(dir)

endif (_kdeBootStrapping)


#####################  provide some options   ##########################################

option(KDE4_ENABLE_FINAL "Enable final all-in-one compilation")
option(KDE4_BUILD_TESTS  "Build the tests")
option(KDE4_ENABLE_HTMLHANDBOOK  "Create targets htmlhandbook for creating the html versions of the docbook docs")

if( KDE4_ENABLE_FINAL)
   add_definitions(-DKDE_USE_FINAL)
endif(KDE4_ENABLE_FINAL)

#Position-Independent-Executable is a feature of Binutils, Libc, and GCC that creates an executable
#which is something between a shared library and a normal executable.
#Programs compiled with these features appear as ?shared object? with the file command.
#info from "http://www.linuxfromscratch.org/hlfs/view/unstable/glibc/chapter02/pie.html"
option(KDE4_ENABLE_FPIE  "Enable platform supports PIE linking")

set(LIB_SUFFIX "" CACHE STRING "Define suffix of directory name (32/64)" )

########## the following are directories where stuff will be installed to  ###########
#
# this has to be after find_xxx() block above, since there KDELibsDependencies.cmake is included
# which contains the install dirs from kdelibs, which are reused below

# this macro implements some very special logic how to deal with the cache
# by default the various install locations inherit their value from theit "parent" variable
# so if you set CMAKE_INSTALL_PREFIX, then EXEC_INSTALL_PREFIX, PLUGIN_INSTALL_DIR will
# calculate their value by appending subdirs to CMAKE_INSTALL_PREFIX
# this would work completely without using the cache.
# but if somebody wants e.g. a different EXEC_INSTALL_PREFIX this value has to go into
# the cache, otherwise it will be forgotten on the next cmake run.
# Once a variable is in the cache, it doesn't depend on its "parent" variables
# anymore and you can only change it by editing it directly.
# this macro helps in this regard, because as long as you don't set one of the
# variables explicitely to some location, it will always calculate its value from its
# parents. So modifying CMAKE_INSTALL_PREFIX later on will have the desired effect.
# But once you decide to set e.g. EXEC_INSTALL_PREFIX to some special location
# this will go into the cache and it will no longer depend on CMAKE_INSTALL_PREFIX.
#
# additionally if installing to the same location as kdelibs, the other install
# directories are reused from the installed kdelibs
macro(_SET_FANCY _var _value _comment)
   set(predefinedvalue "${_value}")
   if ("${CMAKE_INSTALL_PREFIX}" STREQUAL "${KDE4_INSTALL_DIR}" AND DEFINED KDE4_${_var})
      set(predefinedvalue "${KDE4_${_var}}")
   endif ("${CMAKE_INSTALL_PREFIX}" STREQUAL "${KDE4_INSTALL_DIR}" AND DEFINED KDE4_${_var})

   if (NOT DEFINED ${_var})
      set(${_var} ${predefinedvalue})
   else (NOT DEFINED ${_var})
      set(${_var} "${${_var}}" CACHE PATH "${_comment}")
   endif (NOT DEFINED ${_var})
endmacro(_SET_FANCY)


if (WIN32)
# use relative install prefix to avoid hardcoded install pathes in cmake_install.cmake files
   _set_fancy(EXEC_INSTALL_PREFIX  ""                                        "Base directory for executables and libraries")
   _set_fancy(SHARE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}/share"           "Base directory for files which go to share/")
   _set_fancy(BIN_INSTALL_DIR      "bin"                                     "The install dir for executables (default ${EXEC_INSTALL_PREFIX}/bin)")
   _set_fancy(SBIN_INSTALL_DIR     "sbin"                                    "The install dir for system executables (default ${EXEC_INSTALL_PREFIX}/sbin)")
   _set_fancy(LIB_INSTALL_DIR      "lib${LIB_SUFFIX}"                        "The subdirectory relative to the install prefix where libraries will be installed (default is ${EXEC_INSTALL_PREFIX}/lib${LIB_SUFFIX})")
   _set_fancy(LIBEXEC_INSTALL_DIR  "${BIN_INSTALL_DIR}"                      "The subdirectory relative to the install prefix where libraries will be installed (default is ${BIN_INSTALL_DIR})")
   _set_fancy(INCLUDE_INSTALL_DIR  "include"                                 "The subdirectory to the header prefix")
else (WIN32)
   _set_fancy(EXEC_INSTALL_PREFIX  "${CMAKE_INSTALL_PREFIX}"                 "Base directory for executables and libraries")
   _set_fancy(SHARE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}/share"           "Base directory for files which go to share/")
   _set_fancy(BIN_INSTALL_DIR      "${EXEC_INSTALL_PREFIX}/bin"              "The install dir for executables (default ${EXEC_INSTALL_PREFIX}/bin)")
   _set_fancy(SBIN_INSTALL_DIR     "${EXEC_INSTALL_PREFIX}/sbin"             "The install dir for system executables (default ${EXEC_INSTALL_PREFIX}/sbin)")
   _set_fancy(LIB_INSTALL_DIR      "${EXEC_INSTALL_PREFIX}/lib${LIB_SUFFIX}" "The subdirectory relative to the install prefix where libraries will be installed (default is ${EXEC_INSTALL_PREFIX}/lib${LIB_SUFFIX})")
   _set_fancy(LIBEXEC_INSTALL_DIR  "${LIB_INSTALL_DIR}/kde4/libexec"         "The subdirectory relative to the install prefix where libraries will be installed (default is ${LIB_INSTALL_DIR}/kde4/libexec)")
   _set_fancy(INCLUDE_INSTALL_DIR  "${CMAKE_INSTALL_PREFIX}/include"         "The subdirectory to the header prefix")
endif (WIN32)

_set_fancy(PLUGIN_INSTALL_DIR       "${LIB_INSTALL_DIR}/kde4"                "The subdirectory relative to the install prefix where plugins will be installed (default is ${LIB_INSTALL_DIR}/kde4)")
_set_fancy(CONFIG_INSTALL_DIR       "${SHARE_INSTALL_PREFIX}/config"         "The config file install dir")
_set_fancy(DATA_INSTALL_DIR         "${SHARE_INSTALL_PREFIX}/apps"           "The parent directory where applications can install their data")
_set_fancy(HTML_INSTALL_DIR         "${SHARE_INSTALL_PREFIX}/doc/HTML"       "The HTML install dir for documentation")
_set_fancy(ICON_INSTALL_DIR         "${SHARE_INSTALL_PREFIX}/icons"          "The icon install dir (default ${SHARE_INSTALL_PREFIX}/share/icons/)")
_set_fancy(KCFG_INSTALL_DIR         "${SHARE_INSTALL_PREFIX}/config.kcfg"    "The install dir for kconfig files")
_set_fancy(LOCALE_INSTALL_DIR       "${SHARE_INSTALL_PREFIX}/locale"         "The install dir for translations")
_set_fancy(MIME_INSTALL_DIR         "${SHARE_INSTALL_PREFIX}/mimelnk"        "The install dir for the mimetype desktop files")
_set_fancy(SERVICES_INSTALL_DIR     "${SHARE_INSTALL_PREFIX}/kde4/services"  "The install dir for service (desktop, protocol, ...) files")
_set_fancy(SERVICETYPES_INSTALL_DIR "${SHARE_INSTALL_PREFIX}/kde4/servicetypes" "The install dir for servicestypes desktop files")
_set_fancy(SOUND_INSTALL_DIR        "${SHARE_INSTALL_PREFIX}/sounds"         "The install dir for sound files")
_set_fancy(TEMPLATES_INSTALL_DIR    "${SHARE_INSTALL_PREFIX}/templates"      "The install dir for templates (Create new file...)")
_set_fancy(WALLPAPER_INSTALL_DIR    "${SHARE_INSTALL_PREFIX}/wallpapers"     "The install dir for wallpapers")
_set_fancy(KCONF_UPDATE_INSTALL_DIR "${DATA_INSTALL_DIR}/kconf_update"       "The kconf_update install dir")
_set_fancy(AUTOSTART_INSTALL_DIR    "${SHARE_INSTALL_PREFIX}/autostart"      "The install dir for autostart files")

_set_fancy(XDG_APPS_INSTALL_DIR     "${SHARE_INSTALL_PREFIX}/applications/kde4"         "The XDG apps dir")
_set_fancy(XDG_DIRECTORY_INSTALL_DIR "${SHARE_INSTALL_PREFIX}/desktop-directories"      "The XDG directory")
_set_fancy(XDG_MIME_INSTALL_DIR     "${SHARE_INSTALL_PREFIX}/mime/packages"  "The install dir for the xdg mimetypes")

_set_fancy(SYSCONF_INSTALL_DIR      "${CMAKE_INSTALL_PREFIX}/etc"            "The kde sysconfig install dir (default /etc)")
_set_fancy(MAN_INSTALL_DIR          "${SHARE_INSTALL_PREFIX}/man"            "The kde man install dir (default ${SHARE_INSTALL_PREFIX}/man/)")
_set_fancy(INFO_INSTALL_DIR         "${SHARE_INSTALL_PREFIX}/info"           "The kde info install dir (default ${SHARE_INSTALL_PREFIX}/info)")
_set_fancy(DBUS_INTERFACES_INSTALL_DIR      "${SHARE_INSTALL_PREFIX}/dbus-1/interfaces" "The kde dbus interfaces install dir (default  ${SHARE_INSTALL_PREFIX}/dbus-1/interfaces)")
_set_fancy(DBUS_SERVICES_INSTALL_DIR      "${SHARE_INSTALL_PREFIX}/dbus-1/services"     "The kde dbus services install dir (default  ${SHARE_INSTALL_PREFIX}/dbus-1/services)")


##############  add some more default search paths  ###############
# always search in the directory where cmake is installed
# and in the current installation prefix
# the KDE4_xxx_INSTALL_DIR variables are empty when building kdelibs itself
# and otherwise point to the kde4 install dirs
# they will be set by default starting with cmake 2.6.0, maybe already 2.4.8

# also add the install directory of the running cmake to the search directories
# CMAKE_ROOT is CMAKE_INSTALL_PREFIX/share/cmake, so we need to go two levels up
get_filename_component(_CMAKE_INSTALL_DIR "${CMAKE_ROOT}" PATH)
get_filename_component(_CMAKE_INSTALL_DIR "${_CMAKE_INSTALL_DIR}" PATH)

set(CMAKE_SYSTEM_INCLUDE_PATH ${CMAKE_SYSTEM_INCLUDE_PATH}
                              "${KDE4_INCLUDE_INSTALL_DIR}"
                              "${_CMAKE_INSTALL_DIR}/include"
                              "${CMAKE_INSTALL_PREFIX}/include" )

set(CMAKE_SYSTEM_PROGRAM_PATH ${CMAKE_SYSTEM_PROGRAM_PATH}
                              "${KDE4_BIN_INSTALL_DIR}"
                              "${_CMAKE_INSTALL_DIR}/bin"
                              "${CMAKE_INSTALL_PREFIX}/bin" )

set(CMAKE_SYSTEM_LIBRARY_PATH ${CMAKE_SYSTEM_LIBRARY_PATH}
                              "${KDE4_LIB_INSTALL_DIR}"
                              "${_CMAKE_INSTALL_DIR}/lib"
                              "${CMAKE_INSTALL_PREFIX}/lib" )

# under Windows dlls may be also installed in bin/
if(WIN32)
  set(CMAKE_SYSTEM_LIBRARY_PATH ${CMAKE_SYSTEM_LIBRARY_PATH}
                                "${_CMAKE_INSTALL_DIR}/bin"
                                "${CMAKE_INSTALL_PREFIX}/bin" )
endif(WIN32)


#####################  and now the platform specific stuff  ############################

# Set a default build type for single-configuration
# CMake generators if no build type is set.
if (NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif (NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)


if (WIN32)

   if(CYGWIN)
      message(FATAL_ERROR "Cygwin is NOT supported, use mingw or MSVC to build KDE4.")
   endif(CYGWIN)

   find_package(KDEWIN32 REQUIRED)

   # limit win32 packaging to kdelibs at now
   # don't know if package name, version and notes are always available
   if(_kdeBootStrapping)
      find_package(KDEWIN_Packager)
      if (KDEWIN_PACKAGER_FOUND)
         kdewin_packager("kdelibs" "${KDE_VERSION}" "KDE base library" "")
      endif (KDEWIN_PACKAGER_FOUND)

      include(Win32Macros)
      addExplorerWrapper("kdelibs")
   endif(_kdeBootStrapping)

   set( _KDE4_PLATFORM_INCLUDE_DIRS ${KDEWIN32_INCLUDES})

   # if we are compiling kdelibs, add KDEWIN32_LIBRARIES explicitely,
   # otherwise they come from KDELibsDependencies.cmake, Alex
   if (_kdeBootStrapping)
      set( KDE4_KDECORE_LIBS ${KDE4_KDECORE_LIBS} ${KDEWIN32_LIBRARIES} )
   endif (_kdeBootStrapping)

   # windows, mingw
   if(MINGW)
      #hmmm, something special to do here ?
   endif(MINGW)

   # windows, microsoft compiler
   if(MSVC)
      set( _KDE4_PLATFORM_DEFINITIONS -DKDE_FULL_TEMPLATE_EXPORT_INSTANTIATION -DWIN32_LEAN_AND_MEAN -DUNICODE )
      # C4250: 'class1' : inherits 'class2::member' via dominance
      set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -wd4250" )
      # C4251: 'identifier' : class 'type' needs to have dll-interface to be used by clients of class 'type2'
      set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -wd4251" )
      # C4396: 'identifier' : 'function' the inline specifier cannot be used when a friend declaration refers to a specialization of a function template
      set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -wd4396" )
      if(CMAKE_COMPILER_2005)
         # to avoid a lot of deprecated warnings
         add_definitions( -D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE -D_SCL_SECURE_NO_WARNINGS )
         # 'identifier' : no suitable definition provided for explicit template instantiation request
         set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -wd4661" )
      endif(CMAKE_COMPILER_2005)
   endif(MSVC)


   # for visual studio IDE set the path correctly for custom commands
   # maybe under windows bat-files should be generated for running apps during the build
   if(MSVC_IDE)
     get_filename_component(PERL_LOCATION "${PERL_EXECUTABLE}" PATH)
     file(TO_NATIVE_PATH "${PERL_LOCATION}" PERL_PATH_WINDOWS)
     file(TO_NATIVE_PATH "${QT_BINARY_DIR}" QT_BIN_DIR_WINDOWS)
     set(CMAKE_MSVCIDE_RUN_PATH "${PERL_PATH_WINDOWS}\;${QT_BIN_DIR_WINDOWS}"
       CACHE STATIC "MSVC IDE Run path" FORCE)
   endif(MSVC_IDE)

   # christian:
   # don't know where to add this -> FindQt4.cmake is only executed once and
   # because somehow those values are not saved in the cache, I have to set them
   # new on every cmake run
   # http://www.cmake.org/pipermail/cmake/2007-November/017541.html
   # set(CMAKE_CXX_FLAGS_DEBUG          "${CMAKE_CXX_FLAGS_DEBUG} -DQT_DEBUG")
   set(CMAKE_CXX_FLAGS_MINSIZEREL     "${CMAKE_CXX_FLAGS_MINSIZEREL} -DQT_NO_DEBUG")
   set(CMAKE_CXX_FLAGS_RELEASE        "${CMAKE_CXX_FLAGS_RELEASE} -DQT_NO_DEBUG")
   set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -DQT_NO_DEBUG")

endif (WIN32)


# also use /usr/local by default under UNIX, including Mac OS X
if (UNIX)
   option(KDE4_USE_ALWAYS_FULL_RPATH "If set to TRUE, also libs and plugins will be linked with the full RPATH, which will usually make them work better, but make install will take longer." OFF)

   set( _KDE4_PLATFORM_INCLUDE_DIRS)

   # the rest is RPATH handling
   # here the defaults are set
   # which are partly overwritten in kde4_handle_rpath_for_library()
   # and kde4_handle_rpath_for_executable(), both located in KDE4Macros.cmake, Alex
   if (APPLE)
      set(CMAKE_INSTALL_NAME_DIR ${LIB_INSTALL_DIR})
   else (APPLE)
      # add our LIB_INSTALL_DIR to the RPATH and use the RPATH figured out by cmake when compiling
      set(CMAKE_INSTALL_RPATH ${LIB_INSTALL_DIR} )
      set(CMAKE_SKIP_BUILD_RPATH TRUE)
      set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
      set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
   endif (APPLE)
endif (UNIX)


if (Q_WS_X11)
   # Done by FindQt4.cmake already
   #find_package(X11 REQUIRED)
   # UNIX has already set _KDE4_PLATFORM_INCLUDE_DIRS, so append
   set(_KDE4_PLATFORM_INCLUDE_DIRS ${_KDE4_PLATFORM_INCLUDE_DIRS} ${X11_INCLUDE_DIR} )
endif (Q_WS_X11)


# This will need to be modified later to support either Qt/X11 or Qt/Mac builds
if (APPLE)

  set ( _KDE4_PLATFORM_DEFINITIONS -D__APPLE_KDE__ )

  # we need to set MACOSX_DEPLOYMENT_TARGET to (I believe) at least 10.2 or maybe 10.3 to allow
  # -undefined dynamic_lookup; in the future we should do this programmatically
  # hmm... why doesn't this work?
  set (ENV{MACOSX_DEPLOYMENT_TARGET} 10.3)

  # "-undefined dynamic_lookup" means we don't care about missing symbols at link-time by default
  # this is bad, but unavoidable until there is the equivalent of libtool -no-undefined implemented
  # or perhaps it already is, and I just don't know where to look  ;)

  set (CMAKE_SHARED_LINKER_FLAGS "-single_module -multiply_defined suppress ${CMAKE_SHARED_LINKER_FLAGS}")
  set (CMAKE_MODULE_LINKER_FLAGS "-multiply_defined suppress ${CMAKE_MODULE_LINKER_FLAGS}")
  #set(CMAKE_SHARED_LINKER_FLAGS "-single_module -undefined dynamic_lookup -multiply_defined suppress")
  #set(CMAKE_MODULE_LINKER_FLAGS "-undefined dynamic_lookup -multiply_defined suppress")

  # we profile...
  if(CMAKE_BUILD_TYPE_TOLOWER MATCHES profile)
    set (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fprofile-arcs -ftest-coverage")
    set (CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage")
  endif(CMAKE_BUILD_TYPE_TOLOWER MATCHES profile)

  # removed -Os, was there a special reason for using -Os instead of -O2 ?, Alex
  # optimization flags are set below for the various build types
  set (CMAKE_C_FLAGS     "${CMAKE_C_FLAGS} -fno-common")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-common")
endif (APPLE)


if (CMAKE_SYSTEM_NAME MATCHES Linux)
   if (CMAKE_COMPILER_IS_GNUCXX)
      set ( _KDE4_PLATFORM_DEFINITIONS -D_XOPEN_SOURCE=500 -D_BSD_SOURCE -D_GNU_SOURCE)
# #       set ( CMAKE_SHARED_LINKER_FLAGS "-Wl,--fatal-warnings -Wl,--no-undefined -lc ${CMAKE_SHARED_LINKER_FLAGS}")
# #       set ( CMAKE_MODULE_LINKER_FLAGS "-Wl,--fatal-warnings -Wl,--no-undefined -lc ${CMAKE_MODULE_LINKER_FLAGS}")
      # we profile...
      if(CMAKE_BUILD_TYPE_TOLOWER MATCHES profile)
        set (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fprofile-arcs -ftest-coverage")
        set (CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage")
      endif(CMAKE_BUILD_TYPE_TOLOWER MATCHES profile)
   endif (CMAKE_COMPILER_IS_GNUCXX)
   if (CMAKE_C_COMPILER MATCHES "icc")
      set ( _KDE4_PLATFORM_DEFINITIONS -D_XOPEN_SOURCE=500 -D_BSD_SOURCE -D_GNU_SOURCE)
# #       set ( CMAKE_SHARED_LINKER_FLAGS "-Wl,--fatal-warnings -Wl,--no-undefined -lc ${CMAKE_SHARED_LINKER_FLAGS}")
# #       set ( CMAKE_MODULE_LINKER_FLAGS "-Wl,--fatal-warnings -Wl,--no-undefined -lc ${CMAKE_MODULE_LINKER_FLAGS}")
   endif (CMAKE_C_COMPILER MATCHES "icc")
endif (CMAKE_SYSTEM_NAME MATCHES Linux)


if (CMAKE_SYSTEM_NAME MATCHES BSD)
   set ( _KDE4_PLATFORM_DEFINITIONS -D_GNU_SOURCE )
   set ( CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -lc")
   set ( CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -lc")
endif (CMAKE_SYSTEM_NAME MATCHES BSD)

# compiler specific stuff, maybe this should be done differently, Alex

if (MSVC)
   set (KDE4_ENABLE_EXCEPTIONS -EHsc)
endif(MSVC)


if (MINGW)
   set (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--export-all-symbols -Wl,--disable-auto-import")
   set (CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,--export-all-symbols -Wl,--disable-auto-import")
endif (MINGW)


if (CMAKE_COMPILER_IS_GNUCXX)
   set (KDE4_ENABLE_EXCEPTIONS -fexceptions)
   # Select flags.
   set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")
   set(CMAKE_CXX_FLAGS_RELEASE        "-O2")
   set(CMAKE_CXX_FLAGS_DEBUG          "-g -O2 -fno-reorder-blocks -fno-schedule-insns -fno-inline")
   set(CMAKE_CXX_FLAGS_DEBUGFULL      "-g3 -fno-inline")
   set(CMAKE_CXX_FLAGS_PROFILE        "-g3 -fno-inline -ftest-coverage -fprofile-arcs")
   set(CMAKE_C_FLAGS_RELWITHDEBINFO   "-O2 -g")
   set(CMAKE_C_FLAGS_RELEASE          "-O2")
   set(CMAKE_C_FLAGS_DEBUG            "-g -O2 -fno-reorder-blocks -fno-schedule-insns -fno-inline")
   set(CMAKE_C_FLAGS_DEBUGFULL        "-g3 -fno-inline")
   set(CMAKE_C_FLAGS_PROFILE          "-g3 -fno-inline -ftest-coverage -fprofile-arcs")

   if (CMAKE_SYSTEM_NAME MATCHES Linux)
     set ( CMAKE_C_FLAGS     "${CMAKE_C_FLAGS} -Wno-long-long -std=iso9899:1990 -Wundef -Wcast-align -Werror-implicit-function-declaration -Wchar-subscripts -Wall -W -Wpointer-arith -Wwrite-strings -Wformat-security -Wmissing-format-attribute -fno-common")
     set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wnon-virtual-dtor -Wno-long-long -ansi -Wundef -Wcast-align -Wchar-subscripts -Wall -W -Wpointer-arith -Wformat-security -fno-exceptions -fno-check-new -fno-common")
     add_definitions (-D_BSD_SOURCE)
   endif (CMAKE_SYSTEM_NAME MATCHES Linux)


   check_cxx_compiler_flag(-fPIE HAVE_FPIE_SUPPORT)
   if(KDE4_ENABLE_FPIE)
       if(HAVE_FPIE_SUPPORT)
        set (KDE4_CXX_FPIE_FLAGS "-fPIE")
        set (KDE4_PIE_LDFLAGS "-pie")
       else(HAVE_FPIE_SUPPORT)
        message(STATUS "Your compiler doesn't support PIE flag")
       endif(HAVE_FPIE_SUPPORT)
   endif(KDE4_ENABLE_FPIE)

   # save a little by making local statics not threadsafe
   check_cxx_compiler_flag(-fno-threadsafe-statics __KDE_HAVE_NO_THREADSAFE_STATICS)
   if (__KDE_HAVE_NO_THREADSAFE_STATICS)
       # currently disabled for Alpha1 due to what appears
       # to be a compiler bug
       # see http://gcc.gnu.org/bugzilla/show_bug.cgi?id=31806
       #set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-threadsafe-statics")
   endif (__KDE_HAVE_NO_THREADSAFE_STATICS)

   # visibility support
   check_cxx_compiler_flag(-fvisibility=hidden __KDE_HAVE_GCC_VISIBILITY)
   set( __KDE_HAVE_GCC_VISIBILITY ${__KDE_HAVE_GCC_VISIBILITY} CACHE BOOL "GCC support for hidden visibility")

   # get the gcc version
   exec_program(${CMAKE_C_COMPILER} ARGS --version OUTPUT_VARIABLE _gcc_version_info)

   string (REGEX MATCH "[345]\\.[0-9]\\.[0-9]" _gcc_version "${_gcc_version_info}")
   # gcc on mac just reports: "gcc (GCC) 3.3 20030304 ..." without the patch level, handle this here:
   if (NOT _gcc_version)
      string (REGEX REPLACE ".*\\(GCC\\).* ([34]\\.[0-9]) .*" "\\1.0" _gcc_version "${_gcc_version_info}")
   endif (NOT _gcc_version)

   macro_ensure_version("4.1.0" "${_gcc_version}" GCC_IS_NEWER_THAN_4_1)
   macro_ensure_version("4.2.0" "${_gcc_version}" GCC_IS_NEWER_THAN_4_2)

   set(_GCC_COMPILED_WITH_BAD_ALLOCATOR FALSE)
   if (GCC_IS_NEWER_THAN_4_1)
      exec_program(${CMAKE_C_COMPILER} ARGS -v OUTPUT_VARIABLE _gcc_alloc_info)
      string(REGEX MATCH "(--enable-libstdcxx-allocator=mt)" _GCC_COMPILED_WITH_BAD_ALLOCATOR "${_gcc_alloc_info}")
   endif (GCC_IS_NEWER_THAN_4_1)

   if (__KDE_HAVE_GCC_VISIBILITY AND GCC_IS_NEWER_THAN_4_1 AND NOT _GCC_COMPILED_WITH_BAD_ALLOCATOR)
# #       set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
# #       set (KDE4_C_FLAGS "-fvisibility=hidden")
      # check that Qt defines Q_DECL_EXPORT as __attribute__ ((visibility("default")))
      # if it doesn't and KDE compiles with hidden default visibiltiy plugins will break
      set(_source "#include <QtCore/QtGlobal>\n int main()\n {\n #ifdef QT_VISIBILITY_AVAILABLE \n return 0;\n #else \n return 1; \n #endif \n }\n")
      set(_source_file ${CMAKE_BINARY_DIR}/CMakeTmp/check_qt_visibility.cpp)
      file(WRITE "${_source_file}" "${_source}")
      set(_include_dirs "-DINCLUDE_DIRECTORIES:STRING=${QT_INCLUDES}")

      try_run(_run_result _compile_result ${CMAKE_BINARY_DIR} ${_source_file} CMAKE_FLAGS "${_include_dirs}")

      if(NOT _compile_result)
         message(FATAL_ERROR "Could not compile simple test program:\n ${_source}")
      endif(NOT _compile_result)
      if(_run_result)
         message(FATAL_ERROR "Qt compiled without support for -fvisibility=hidden. This will break plugins and linking of some applications. Please fix your Qt installation.")
      endif(_run_result)

      if (GCC_IS_NEWER_THAN_4_2)
          set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility-inlines-hidden")
      endif (GCC_IS_NEWER_THAN_4_2)
   else (__KDE_HAVE_GCC_VISIBILITY AND GCC_IS_NEWER_THAN_4_1 AND NOT _GCC_COMPILED_WITH_BAD_ALLOCATOR)
      set (__KDE_HAVE_GCC_VISIBILITY 0)
   endif (__KDE_HAVE_GCC_VISIBILITY AND GCC_IS_NEWER_THAN_4_1 AND NOT _GCC_COMPILED_WITH_BAD_ALLOCATOR)

endif (CMAKE_COMPILER_IS_GNUCXX)


if (CMAKE_C_COMPILER MATCHES "icc")
   set (KDE4_ENABLE_EXCEPTIONS -fexceptions)
   # Select flags.
   set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")
   set(CMAKE_CXX_FLAGS_RELEASE        "-O2")
   set(CMAKE_CXX_FLAGS_DEBUG          "-O2 -g -0b0 -noalign")
   set(CMAKE_CXX_FLAGS_DEBUGFULL      "-g -Ob0 -noalign")
   set(CMAKE_C_FLAGS_RELWITHDEBINFO   "-O2 -g")
   set(CMAKE_C_FLAGS_RELEASE          "-O2")
   set(CMAKE_C_FLAGS_DEBUG            "-O2 -g -Ob0 -noalign")
   set(CMAKE_C_FLAGS_DEBUGFULL        "-g -Ob0 -noalign")

   set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -ansi -Wpointer-arith -fno-common")
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ansi -Wpointer-arith -fno-exceptions -fno-common")

   # visibility support
   set(__KDE_HAVE_ICC_VISIBILITY)
#   check_cxx_compiler_flag(-fvisibility=hidden __KDE_HAVE_ICC_VISIBILITY)
#   if (__KDE_HAVE_ICC_VISIBILITY)
#      set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
#   endif (__KDE_HAVE_ICC_VISIBILITY)

endif (CMAKE_C_COMPILER MATCHES "icc")

# we prefer to use a different postfix for debug libs only on Windows
# does not work atm
if (WIN32)
   SET(CMAKE_DEBUG_POSTFIX "")
endif (WIN32)

###########    end of platform specific stuff  ##########################


# KDE4Macros.cmake contains all the KDE specific macros
include(${kde_cmake_module_dir}/KDE4Macros.cmake)


# decide whether KDE4 has been found
set(KDE4_FOUND FALSE)
if (KDE4_INCLUDE_DIR AND KDE4_LIB_DIR AND KDE4_KCFGC_EXECUTABLE AND KDE4_INSTALLED_VERSION_OK)
   set(KDE4_FOUND TRUE)
endif (KDE4_INCLUDE_DIR AND KDE4_LIB_DIR AND KDE4_KCFGC_EXECUTABLE AND KDE4_INSTALLED_VERSION_OK)


macro (KDE4_PRINT_RESULTS)

   # inside kdelibs the include dir and lib dir are internal, not "found"
   if (NOT _kdeBootStrapping)
       if(KDE4_INCLUDE_DIR)
          message(STATUS "Found KDE 4.0 include dir: ${KDE4_INCLUDE_DIR}")
       else(KDE4_INCLUDE_DIR)
          message(STATUS "ERROR: unable to find KDE 4 headers")
       endif(KDE4_INCLUDE_DIR)

       if(KDE4_LIB_DIR)
          message(STATUS "Found KDE 4 library dir: ${KDE4_LIB_DIR}")
       else(KDE4_LIB_DIR)
          message(STATUS "ERROR: unable to find KDE 4 core library")
       endif(KDE4_LIB_DIR)
   endif (NOT _kdeBootStrapping)

   if(KDE4_KCFGC_EXECUTABLE)
      message(STATUS "Found KDE4 kconfig_compiler preprocessor: ${KDE4_KCFGC_EXECUTABLE}")
   else(KDE4_KCFGC_EXECUTABLE)
      message(STATUS "Didn't find the KDE4 kconfig_compiler preprocessor")
   endif(KDE4_KCFGC_EXECUTABLE)

   if(KDE4_AUTOMOC_EXECUTABLE)
      message(STATUS "Found KDE4 automoc: ${KDE4_AUTOMOC_EXECUTABLE}")
   else(KDE4_AUTOMOC_EXECUTABLE)
      message(STATUS "Didn't find the KDE4 automoc")
   endif(KDE4_AUTOMOC_EXECUTABLE)
endmacro (KDE4_PRINT_RESULTS)


if (KDE4Internal_FIND_REQUIRED AND NOT KDE4_FOUND)
   #bail out if something wasn't found
   kde4_print_results()
   if (NOT KDE4_INSTALLED_VERSION_OK)
     message(FATAL_ERROR "ERROR: the installed kdelibs version ${KDEVERSION} is too old, at least version ${KDE_MIN_VERSION} is required")
   endif (NOT KDE4_INSTALLED_VERSION_OK)

   if (NOT KDE4_KCFGC_EXECUTABLE)
     message(FATAL_ERROR "ERROR: could not detect a usable kconfig_compiler")
   endif (NOT KDE4_KCFGC_EXECUTABLE)

   message(FATAL_ERROR "ERROR: could NOT find everything required for compiling KDE 4 programs")
endif (KDE4Internal_FIND_REQUIRED AND NOT KDE4_FOUND)


if (NOT KDE4Internal_FIND_QUIETLY)
   kde4_print_results()
endif (NOT KDE4Internal_FIND_QUIETLY)

#add the found Qt and KDE include directories to the current include path
#the ${KDE4_INCLUDE_DIR}/KDE directory is for forwarding includes, eg. #include <KMainWindow>
set(KDE4_INCLUDES ${QT_INCLUDES} ${KDE4_INCLUDE_DIR} ${KDE4_INCLUDE_DIR}/KDE ${_KDE4_PLATFORM_INCLUDE_DIRS} )

set(KDE4_DEFINITIONS ${_KDE4_PLATFORM_DEFINITIONS} -DQT_NO_STL -DQT_NO_CAST_TO_ASCII -D_REENTRANT -DKDE_DEPRECATED_WARNINGS )

if (NOT _kde4_uninstall_rule_created)
   set(_kde4_uninstall_rule_created TRUE)

   configure_file("${kde_cmake_module_dir}/kde4_cmake_uninstall.cmake.in" "${CMAKE_BINARY_DIR}/cmake_uninstall.cmake" @ONLY)

   add_custom_target(uninstall "${CMAKE_COMMAND}" -P "${CMAKE_BINARY_DIR}/cmake_uninstall.cmake")

endif (NOT _kde4_uninstall_rule_created)

endif(KDE4_FOUND)

