# for documentation look at FindKDE4Internal.cmake
#
# this file contains the following macros:
# KDE4_ADD_UI_FILES
# KDE4_ADD_UI3_FILES
# KDE4_ADD_KCFG_FILES
# KDE4_SET_CUSTOM_TARGET_PROPERTY
# KDE4_GET_CUSTOM_TARGET_PROPERTY
# KDE4_MOC_HEADERS
# KDE4_HANDLE_AUTOMOC
# KDE4_CREATE_FINAL_FILES
# KDE4_ADD_PLUGIN
# KDE4_ADD_KDEINIT_EXECUTABLE
# KDE4_ADD_UNIT_TEST
# KDE4_ADD_EXECUTABLE
# KDE4_ADD_WIDGET_FILES
# KDE4_UPDATE_ICONCACHE
# KDE4_INSTALL_ICONS
# KDE4_REMOVE_OBSOLETE_CMAKE_FILES
# KDE4_NO_ENABLE_FINAL
# KDE4_CREATE_HANDBOOK
# KDE4_ADD_WIN32_APP_ICON (Use on Win32)
# KDE4_CREATE_MANPAGE

# Copyright (c) 2006, 2007, Alexander Neundorf, <neundorf@kde.org>
# Copyright (c) 2006, 2007, Laurent Montel, <montel@kde.org>
# Copyright (c) 2007 Matthias Kretz <kretz@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

macro (KDE4_ADD_KCFG_FILES _sources )
   if( ${ARGV1} STREQUAL "GENERATE_MOC" )
      set(_kcfg_generatemoc TRUE)
   endif( ${ARGV1} STREQUAL "GENERATE_MOC" )

   foreach (_current_FILE ${ARGN})

     if(NOT ${_current_FILE} STREQUAL "GENERATE_MOC")
       get_filename_component(_tmp_FILE ${_current_FILE} ABSOLUTE)
       get_filename_component(_abs_PATH ${_tmp_FILE} PATH)
       get_filename_component(_basename ${_tmp_FILE} NAME_WE)

       file(READ ${_tmp_FILE} _contents)
       string(REGEX REPLACE "^(.*\n)?File=([^\n]+kcfg).*\n.*$" "\\2"  _kcfg_FILE "${_contents}")
       set(_src_FILE    ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.cpp)
       set(_header_FILE ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.h)
       set(_moc_FILE    ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.moc)

       # the command for creating the source file from the kcfg file
       add_custom_command(OUTPUT ${_header_FILE} ${_src_FILE}
          COMMAND ${KDE4_KCFGC_EXECUTABLE}
          ARGS ${_abs_PATH}/${_kcfg_FILE} ${_tmp_FILE} -d ${CMAKE_CURRENT_BINARY_DIR}
          MAIN_DEPENDENCY ${_tmp_FILE}
          DEPENDS ${_abs_PATH}/${_kcfg_FILE} ${_KDE4_KCONFIG_COMPILER_DEP} )

       if(_kcfg_generatemoc)
         qt4_generate_moc(${_header_FILE} ${_moc_FILE} )
         set_source_files_properties(${_src_FILE} PROPERTIES SKIP_AUTOMOC TRUE)  # don't run automoc on this file
         list(APPEND ${_sources} ${_moc_FILE})
       endif(_kcfg_generatemoc)

       list(APPEND ${_sources} ${_src_FILE} ${_header_FILE})
     endif(NOT ${_current_FILE} STREQUAL "GENERATE_MOC")
   endforeach (_current_FILE)

endmacro (KDE4_ADD_KCFG_FILES)


GET_FILENAME_COMPONENT(KDE4_MODULE_DIR  ${CMAKE_CURRENT_LIST_FILE} PATH)

#create the implementation files from the ui files and add them to the list of sources
#usage: KDE4_ADD_UI_FILES(foo_SRCS ${ui_files})
macro (KDE4_ADD_UI_FILES _sources )
   foreach (_current_FILE ${ARGN})

      get_filename_component(_tmp_FILE ${_current_FILE} ABSOLUTE)
      get_filename_component(_basename ${_tmp_FILE} NAME_WE)
      set(_header ${CMAKE_CURRENT_BINARY_DIR}/ui_${_basename}.h)

      # we need to run uic and replace some things in the generated file
      # this is done by executing the cmake script kde4uic.cmake
      add_custom_command(OUTPUT ${_header}
         COMMAND ${CMAKE_COMMAND}
         ARGS
         -DKDE4_HEADER:BOOL=ON
         -DKDE_UIC_EXECUTABLE:FILEPATH=${QT_UIC_EXECUTABLE}
         -DKDE_UIC_FILE:FILEPATH=${_tmp_FILE}
         -DKDE_UIC_H_FILE:FILEPATH=${_header}
         -DKDE_UIC_BASENAME:STRING=${_basename}
         -P ${KDE4_MODULE_DIR}/kde4uic.cmake
         MAIN_DEPENDENCY ${_tmp_FILE}
      )
      list(APPEND ${_sources} ${_header})
   endforeach (_current_FILE)
endmacro (KDE4_ADD_UI_FILES)


#create the implementation files from the ui files and add them to the list of sources
#usage: KDE4_ADD_UI3_FILES(foo_SRCS ${ui_files})
macro (KDE4_ADD_UI3_FILES _sources )

   qt4_get_moc_inc_dirs(_moc_INCS)

   foreach (_current_FILE ${ARGN})

      get_filename_component(_tmp_FILE ${_current_FILE} ABSOLUTE)
      get_filename_component(_basename ${_tmp_FILE} NAME_WE)
      set(_header ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.h)
      set(_src ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.cpp)
      set(_moc ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.moc.cpp)

      add_custom_command(OUTPUT ${_header}
         COMMAND ${CMAKE_COMMAND}
         -DKDE3_HEADER:BOOL=ON
         -DKDE_UIC_EXECUTABLE:FILEPATH=${QT_UIC3_EXECUTABLE}
         -DKDE_UIC_FILE:FILEPATH=${_tmp_FILE}
         -DKDE_UIC_H_FILE:FILEPATH=${_header}
         -DKDE_UIC_BASENAME:STRING=${_basename}
         -DKDE_UIC_PLUGIN_DIR:FILEPATH="."
         -P ${KDE4_MODULE_DIR}/kde4uic.cmake
         MAIN_DEPENDENCY ${_tmp_FILE}
      )

# we need to run uic3 and replace some things in the generated file
      # this is done by executing the cmake script kde4uic.cmake
      add_custom_command(OUTPUT ${_src}
         COMMAND ${CMAKE_COMMAND}
         ARGS
         -DKDE3_IMPL:BOOL=ON
         -DKDE_UIC_EXECUTABLE:FILEPATH=${QT_UIC3_EXECUTABLE}
         -DKDE_UIC_FILE:FILEPATH=${_tmp_FILE}
         -DKDE_UIC_CPP_FILE:FILEPATH=${_src}
         -DKDE_UIC_H_FILE:FILEPATH=${_header}
         -DKDE_UIC_BASENAME:STRING=${_basename}
         -DKDE_UIC_PLUGIN_DIR:FILEPATH="."
         -P ${KDE4_MODULE_DIR}/kde4uic.cmake
         MAIN_DEPENDENCY ${_header}
      )

      add_custom_command(OUTPUT ${_moc}
         COMMAND ${QT_MOC_EXECUTABLE}
         ARGS ${_moc_INCS} ${_header} -o ${_moc}
         MAIN_DEPENDENCY ${_header}
      )
      list(APPEND ${_sources} ${_src} ${_moc} )

   endforeach (_current_FILE)
endmacro (KDE4_ADD_UI3_FILES)

macro (KDE4_SET_CUSTOM_TARGET_PROPERTY _target_name _property_name _property)
   string(REGEX REPLACE "[/ ]" "_" _dir "${CMAKE_CURRENT_SOURCE_DIR}")
   set(_kde4_${_dir}_${_target_name}_${_property_name} "${_property}")
endmacro (KDE4_SET_CUSTOM_TARGET_PROPERTY)


macro (KDE4_GET_CUSTOM_TARGET_PROPERTY _var _target_name _property_name)
   string(REGEX REPLACE "[/ ]" "_" _dir "${CMAKE_CURRENT_SOURCE_DIR}")
   set(${_var} "${_kde4_${_dir}_${_target_name}_${_property_name}}")
endmacro (KDE4_GET_CUSTOM_TARGET_PROPERTY)


macro (KDE4_MOC_HEADERS _target_NAME)
   set (_headers_to_moc)
   foreach (_current_FILE ${ARGN})
      get_filename_component(_suffix "${_current_FILE}" EXT)
      if (".h" STREQUAL "${_suffix}" OR ".hpp" STREQUAL "${_suffix}" OR ".hxx" STREQUAL "${_suffix}" OR ".H" STREQUAL "${_suffix}")
         list(APPEND _headers_to_moc ${_current_FILE})
      else (".h" STREQUAL "${_suffix}" OR ".hpp" STREQUAL "${_suffix}" OR ".hxx" STREQUAL "${_suffix}" OR ".H" STREQUAL "${_suffix}")
         message(STATUS "KDE4_MOC_HEADERS: ignoring non-header file ${_current_FILE}")
      endif (".h" STREQUAL "${_suffix}" OR ".hpp" STREQUAL "${_suffix}" OR ".hxx" STREQUAL "${_suffix}" OR ".H" STREQUAL "${_suffix}")
   endforeach (_current_FILE)
   # need to create moc_<filename>.cpp file using kde4automoc.cmake
   # and add it to the target
   if(_headers_to_moc)
      KDE4_SET_CUSTOM_TARGET_PROPERTY(${_target_NAME} AUTOMOC_HEADERS "${_headers_to_moc}")
   endif(_headers_to_moc)
endmacro (KDE4_MOC_HEADERS)

macro(KDE4_HANDLE_AUTOMOC _target_NAME _SRCS)
   set(_moc_files)
   set(_moc_headers)

   # first list all explicitly set headers
   KDE4_GET_CUSTOM_TARGET_PROPERTY(_headers_to_moc ${_target_NAME} AUTOMOC_HEADERS)
   if(NOT _headers_to_moc STREQUAL "NOTFOUND")
      foreach(_header_to_moc ${_headers_to_moc})
         get_filename_component(_abs_header ${_header_to_moc} ABSOLUTE)
         list(APPEND _moc_files ${_abs_header})
         list(APPEND _moc_headers ${_abs_header})
      endforeach(_header_to_moc)
   endif(NOT _headers_to_moc STREQUAL "NOTFOUND")

   # now add all the sources for the automoc
   foreach (_current_FILE ${${_SRCS}})
      get_filename_component(_abs_current_FILE "${_current_FILE}" ABSOLUTE)
      get_source_file_property(_skip "${_abs_current_FILE}" SKIP_AUTOMOC)
      get_source_file_property(_generated "${_abs_current_FILE}" GENERATED)

      if(NOT _generated AND NOT _skip)
         get_filename_component(_suffix "${_current_FILE}" EXT)
         # skip every source file that's not C++
         if(_suffix STREQUAL ".cpp" OR _suffix STREQUAL ".cc" OR _suffix STREQUAL ".cxx" OR _suffix STREQUAL ".C")
            get_filename_component(_basename "${_current_FILE}" NAME_WE)
            get_filename_component(_abs_path "${_abs_current_FILE}" PATH)
            set(_header "${_abs_path}/${_basename}.h")
            if(EXISTS "${_header}")
               list(APPEND _moc_headers ${_header})
            endif(EXISTS "${_header}")
            set(_pheader "${_abs_path}/${_basename}_p.h")
            if(EXISTS "${_pheader}")
               list(APPEND _moc_headers ${_pheader})
            endif(EXISTS "${_pheader}")
            list(APPEND _moc_files ${_abs_current_FILE})
         endif(_suffix STREQUAL ".cpp" OR _suffix STREQUAL ".cc" OR _suffix STREQUAL ".cxx" OR _suffix STREQUAL ".C")
      endif(NOT _generated AND NOT _skip)
   endforeach (_current_FILE)

   set(_automoc_source "${CMAKE_CURRENT_BINARY_DIR}/${_target_NAME}_automoc.cpp")
   GET_DIRECTORY_PROPERTY(_moc_incs INCLUDE_DIRECTORIES)
   CONFIGURE_FILE(${KDE4_MODULE_DIR}/kde4automoc.files.in ${_automoc_source}.files)
   add_custom_command(OUTPUT ${_automoc_source}
      COMMAND ${KDE4_AUTOMOC_EXECUTABLE}
      ${_automoc_source}
      ${CMAKE_CURRENT_SOURCE_DIR}
      ${CMAKE_CURRENT_BINARY_DIR}
      ${QT_MOC_EXECUTABLE}
      DEPENDS ${${_SRCS}} ${_moc_headers} ${_automoc_source}.files ${_KDE4_AUTOMOC_EXECUTABLE_DEP}
      )
   # the OBJECT_DEPENDS is only necessary when a new moc file has to be generated that is included in a source file
   # problem: the whole target is recompiled when the automoc.cpp file is touched
   # set_source_files_properties(${${_SRCS}} PROPERTIES OBJECT_DEPENDS ${_automoc_source})
   set(${_SRCS} ${_automoc_source} ${${_SRCS}})
endmacro(KDE4_HANDLE_AUTOMOC)

macro(KDE4_INSTALL_TS_FILES _lang _sdir)
   file(GLOB_RECURSE _ts_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${_sdir}/*)
   foreach(_current_TS_FILES ${_ts_files})
      string(REGEX MATCH "\\.svn/" _in_svn ${_current_TS_FILES})
      if(NOT _in_svn)
         get_filename_component(_subpath ${_current_TS_FILES} PATH)
         install(FILES ${_current_TS_FILES} DESTINATION ${LOCALE_INSTALL_DIR}/${_lang}/LC_SCRIPTS/${_subpath})
      endif(NOT _in_svn)
   endforeach(_current_TS_FILES)
endmacro(KDE4_INSTALL_TS_FILES)

macro (KDE4_INSTALL_HANDBOOK _lang)
   message(STATUS "KDE4_INSTALL_HANDBOOK() is deprecated. Remove it please. Now all is done in KDE4_CREATE_HANDBOOK.")
endmacro (KDE4_INSTALL_HANDBOOK )


macro (KDE4_CREATE_HANDBOOK _docbook)
   get_filename_component(_input ${_docbook} ABSOLUTE)
   set(_doc ${CMAKE_CURRENT_BINARY_DIR}/index.cache.bz2)

   #Bootstrap
   if (_kdeBootStrapping)
      set(_ssheet ${CMAKE_SOURCE_DIR}/kdoctools/customization/kde-chunk.xsl)
      set(_bootstrapOption "--srcdir=${CMAKE_SOURCE_DIR}/kdoctools/")
   else (_kdeBootStrapping)
      set(_ssheet ${KDE4_DATA_INSTALL_DIR}/ksgmltools2/customization/kde-chunk.xsl)
      set(_bootstrapOption)
   endif (_kdeBootStrapping)

   file(GLOB _docs *.docbook)
   add_custom_command(OUTPUT ${_doc}
      COMMAND ${KDE4_MEINPROC_EXECUTABLE} --check ${_bootstrapOption} --cache ${_doc} ${_input}
      DEPENDS ${_docs} ${_KDE4_MEINPROC_EXECUTABLE_DEP} ${_ssheet}
   )
   add_custom_target(handbook ALL DEPENDS ${_doc})

   if(KDE4_ENABLE_HTMLHANDBOOK)
      set(_htmlDoc ${CMAKE_CURRENT_SOURCE_DIR}/index.html)
      add_custom_command(OUTPUT ${_htmlDoc}
         COMMAND ${KDE4_MEINPROC_EXECUTABLE} --check ${_bootstrapOption} -o ${_htmlDoc} ${_input}
         DEPENDS ${_input} ${_KDE4_MEINPROC_EXECUTABLE_DEP} ${_ssheet}
         )
      add_custom_target(htmlhandbook DEPENDS ${_htmlDoc})
   endif(KDE4_ENABLE_HTMLHANDBOOK)

   set(_args ${ARGN})

   set(_installDest)
   if(_args)
      list(GET _args 0 _tmp)
      if("${_tmp}" STREQUAL "INSTALL_DESTINATION")
         list(GET _args 1 _installDest )
         list(REMOVE_AT _args 0 1)
      endif("${_tmp}" STREQUAL "INSTALL_DESTINATION")
   endif(_args)

   get_filename_component(dirname ${CMAKE_CURRENT_SOURCE_DIR} NAME_WE)
   if(_args)
      list(GET _args 0 _tmp)
      if("${_tmp}" STREQUAL "SUBDIR")
         list(GET _args 1 dirname )
         list(REMOVE_AT _args 0 1)
      endif("${_tmp}" STREQUAL "SUBDIR")
   endif(_args)

   if(_installDest)
      file(GLOB _images *.png)
      install(FILES ${_doc} ${_docs} ${_images} DESTINATION ${_installDest}/${dirname})
      # TODO symlinks on non-unix platforms
      if (UNIX)
         # execute some cmake code on make install which creates the symlink
         install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink \"${_installDest}/common\"  \"${E_TEMPDIR}/${_installDest}/${dirname}/common\" )" )
      endif (UNIX)
   endif(_installDest)

endmacro (KDE4_CREATE_HANDBOOK)


macro (KDE4_CREATE_MANPAGE _docbook _section)
   get_filename_component(_input ${_docbook} ABSOLUTE)
   get_filename_component(_base ${_input} NAME_WE)

   set(_doc ${CMAKE_CURRENT_BINARY_DIR}/${_base}.${_section})
   # sometimes we have "man-" prepended
   string(REGEX REPLACE "/man-" "/" _outdoc ${_doc})

   #Bootstrap
   if (_kdeBootStrapping)
      set(_ssheet ${CMAKE_SOURCE_DIR}/kdoctools/docbook/xsl/manpages/docbook.xsl)
      set(_bootstrapOption "--srcdir=${CMAKE_SOURCE_DIR}/kdoctools/")
   else (_kdeBootStrapping)
      set(_ssheet ${KDE4_DATA_INSTALL_DIR}/ksgmltools2/docbook/xsl/manpages/docbook.xsl)
      set(_bootstrapOption)
   endif (_kdeBootStrapping)

   add_custom_command(OUTPUT ${_outdoc}
      COMMAND ${KDE4_MEINPROC_EXECUTABLE} --stylesheet ${_ssheet} --check ${_bootstrapOption} ${_input}
      DEPENDS ${_input} ${_KDE4_MEINPROC_EXECUTABLE_DEP} ${_ssheet}
   )
   add_custom_target(manpage ALL DEPENDS ${_outdoc})

   set(_args ${ARGN})

   set(_installDest)
   if(_args)
      list(GET _args 0 _tmp)
      if("${_tmp}" STREQUAL "INSTALL_DESTINATION")
         list(GET _args 1 _installDest )
         list(REMOVE_AT _args 0 1)
      endif("${_tmp}" STREQUAL "INSTALL_DESTINATION")
   endif(_args)

   get_filename_component(dirname ${CMAKE_CURRENT_SOURCE_DIR} NAME_WE)
   if(_args)
      list(GET _args 0 _tmp)
      if("${_tmp}" STREQUAL "SUBDIR")
         list(GET _args 1 dirname )
         list(REMOVE_AT _args 0 1)
      endif("${_tmp}" STREQUAL "SUBDIR")
   endif(_args)

   if(_installDest)
      install(FILES ${_outdoc} DESTINATION ${_installDest}/man${_section})
   endif(_installDest)
endmacro (KDE4_CREATE_MANPAGE)


macro (KDE4_UPDATE_ICONCACHE)
    # Update mtime of hicolor icon theme dir.
    # We don't always have touch command (e.g. on Windows), so instead create
    #  and delete a temporary file in the theme dir.
   install(CODE "
    set(DESTDIR_VALUE \"\$ENV{DESTDIR}\")
    if (NOT DESTDIR_VALUE)
        file(WRITE \"${ICON_INSTALL_DIR}/hicolor/temp.txt\" \"update\")
        file(REMOVE \"${ICON_INSTALL_DIR}/hicolor/temp.txt\")
    endif (NOT DESTDIR_VALUE)
    ")
endmacro (KDE4_UPDATE_ICONCACHE)

# a "map" of short type names to the directories
# unknown names should give empty results
# KDE 3 compatibility
set(_KDE4_ICON_GROUP_mime       "mimetypes")
set(_KDE4_ICON_GROUP_filesys    "places")
set(_KDE4_ICON_GROUP_device     "devices")
set(_KDE4_ICON_GROUP_app        "apps")
set(_KDE4_ICON_GROUP_action     "actions")
# KDE 4 compatibility
set(_KDE4_ICON_GROUP_mimetypes  "mimetypes")
set(_KDE4_ICON_GROUP_places     "places")
set(_KDE4_ICON_GROUP_devices    "devices")
set(_KDE4_ICON_GROUP_apps       "apps")
set(_KDE4_ICON_GROUP_actions    "actions")
set(_KDE4_ICON_GROUP_categories "categories")
set(_KDE4_ICON_GROUP_emblems    "emblems")

# a "map" of short theme names to the theme directory
set(_KDE4_ICON_THEME_ox "oxygen")
set(_KDE4_ICON_THEME_cr "crystalsvg")
set(_KDE4_ICON_THEME_lo "locolor")
set(_KDE4_ICON_THEME_hi "hicolor")


# only used internally by KDE4_INSTALL_ICONS
macro (_KDE4_ADD_ICON_INSTALL_RULE _install_SCRIPT _install_PATH _group _orig_NAME _install_NAME)

   # if the string doesn't match the pattern, the result is the full string, so all three have the same content
   if (NOT ${_group} STREQUAL ${_install_NAME} )
      set(_icon_GROUP  ${_KDE4_ICON_GROUP_${_group}})
      if(NOT _icon_GROUP)
         set(_icon_GROUP "actions")
      endif(NOT _icon_GROUP)
#      message(STATUS "icon: ${_current_ICON} size: ${_size} group: ${_group} name: ${_name}" )
      install(FILES ${_orig_NAME} DESTINATION ${_install_PATH}/${_icon_GROUP}/ RENAME ${_install_NAME} )
   endif (NOT ${_group} STREQUAL ${_install_NAME} )

endmacro (_KDE4_ADD_ICON_INSTALL_RULE)


macro (KDE4_INSTALL_ICONS _defaultpath )

   # first the png icons
   file(GLOB _icons *.png)
   foreach (_current_ICON ${_icons} )
      string(REGEX REPLACE "^.*/([a-zA-Z]+)([0-9]+)\\-([a-z]+)\\-(.+\\.png)$" "\\1" _type  "${_current_ICON}")
      string(REGEX REPLACE "^.*/([a-zA-Z]+)([0-9]+)\\-([a-z]+)\\-(.+\\.png)$" "\\2" _size  "${_current_ICON}")
      string(REGEX REPLACE "^.*/([a-zA-Z]+)([0-9]+)\\-([a-z]+)\\-(.+\\.png)$" "\\3" _group "${_current_ICON}")
      string(REGEX REPLACE "^.*/([a-zA-Z]+)([0-9]+)\\-([a-z]+)\\-(.+\\.png)$" "\\4" _name  "${_current_ICON}")

      set(_theme_GROUP ${_KDE4_ICON_THEME_${_type}})
      if( _theme_GROUP)
         _KDE4_ADD_ICON_INSTALL_RULE(${CMAKE_CURRENT_BINARY_DIR}/install_icons.cmake
                    ${_defaultpath}/${_theme_GROUP}/${_size}x${_size}
                    ${_group} ${_current_ICON} ${_name})
      endif( _theme_GROUP)
   endforeach (_current_ICON)

   # mng icons
   file(GLOB _icons *.mng)
   foreach (_current_ICON ${_icons} )
      STRING(REGEX REPLACE "^.*/([a-zA-Z]+)([0-9]+)\\-([a-z]+)\\-(.+\\.mng)$" "\\1" _type  "${_current_ICON}")
      STRING(REGEX REPLACE "^.*/([a-zA-Z]+)([0-9]+)\\-([a-z]+)\\-(.+\\.mng)$" "\\2" _size  "${_current_ICON}")
      STRING(REGEX REPLACE "^.*/([a-zA-Z]+)([0-9]+)\\-([a-z]+)\\-(.+\\.mng)$" "\\3" _group "${_current_ICON}")
      STRING(REGEX REPLACE "^.*/([a-zA-Z]+)([0-9]+)\\-([a-z]+)\\-(.+\\.mng)$" "\\4" _name  "${_current_ICON}")

      set(_theme_GROUP ${_KDE4_ICON_THEME_${_type}})
      if( _theme_GROUP)
         _KDE4_ADD_ICON_INSTALL_RULE(${CMAKE_CURRENT_BINARY_DIR}/install_icons.cmake
                ${_defaultpath}/${_theme_GROUP}/${_size}x${_size}
                ${_group} ${_current_ICON} ${_name})
      endif( _theme_GROUP)
   endforeach (_current_ICON)

   # and now the svg icons
   file(GLOB _icons *.svgz)
   foreach (_current_ICON ${_icons} )
      STRING(REGEX REPLACE "^.*/([a-zA-Z]+)sc\\-([a-z]+)\\-(.+\\.svgz)$" "\\1" _type "${_current_ICON}")
      STRING(REGEX REPLACE "^.*/([a-zA-Z]+)sc\\-([a-z]+)\\-(.+\\.svgz)$" "\\2" _group "${_current_ICON}")
      STRING(REGEX REPLACE "^.*/([a-zA-Z]+)sc\\-([a-z]+)\\-(.+\\.svgz)$" "\\3" _name "${_current_ICON}")

      set(_theme_GROUP ${_KDE4_ICON_THEME_${_type}})
      if( _theme_GROUP)
          _KDE4_ADD_ICON_INSTALL_RULE(${CMAKE_CURRENT_BINARY_DIR}/install_icons.cmake
                            ${_defaultpath}/${_theme_GROUP}/scalable
                            ${_group} ${_current_ICON} ${_name})
      endif( _theme_GROUP)
   endforeach (_current_ICON)

   KDE4_UPDATE_ICONCACHE()

endmacro (KDE4_INSTALL_ICONS)


# For all C++ sources a big source file which includes all the files
# is created.
# This is not done for the C sources, they are just gathered in a separate list
# because they are usually not written by KDE and as such not intended to be
# compiled all-in-one.
macro (KDE4_CREATE_FINAL_FILES _filenameCPP _filesExcludedFromFinalFile )
   set(${_filesExcludedFromFinalFile})
   file(WRITE "${_filenameCPP}" "//autogenerated file\n")
   foreach (_current_FILE ${ARGN})
      get_filename_component(_abs_FILE "${_current_FILE}" ABSOLUTE)
      # don't include any generated files in the final-file
      # because then cmake will not know the dependencies
      get_source_file_property(_isGenerated "${_abs_FILE}" GENERATED)
      if (_isGenerated)
         list(APPEND ${_filesExcludedFromFinalFile} "${_abs_FILE}")
      else (_isGenerated)
         # only put C++ files in the final-file
         if("${_abs_FILE}" MATCHES ".+\\.(cpp|cc|cxx|C)$")
            file(APPEND "${_filenameCPP}" "#include \"${_abs_FILE}\"\n")
         else("${_abs_FILE}" MATCHES ".+\\.(cpp|cc|cxx|C)$")
            list(APPEND ${_filesExcludedFromFinalFile} "${_abs_FILE}")
         endif("${_abs_FILE}" MATCHES ".+\\.(cpp|cc|cxx|C)$")
      endif (_isGenerated)
   endforeach (_current_FILE)

endmacro (KDE4_CREATE_FINAL_FILES)

# This macro sets the RPATH related options for libraries, plugins and kdeinit executables.
# It overrides the defaults set in FindKDE4Internal.cmake.
# If RPATH is not explicitly disabled, libraries and plugins are built without RPATH, in
# the hope that the RPATH which is compiled into the executable is good enough.
macro (KDE4_HANDLE_RPATH_FOR_LIBRARY _target_NAME)
   if (NOT CMAKE_SKIP_RPATH AND NOT KDE4_USE_ALWAYS_FULL_RPATH)
       set_target_properties(${_target_NAME} PROPERTIES INSTALL_RPATH_USE_LINK_PATH FALSE SKIP_BUILD_RPATH TRUE BUILD_WITH_INSTALL_RPATH TRUE INSTALL_RPATH "")
   endif (NOT CMAKE_SKIP_RPATH AND NOT KDE4_USE_ALWAYS_FULL_RPATH)
endmacro (KDE4_HANDLE_RPATH_FOR_LIBRARY)

# This macro sets the RPATH related options for executables
# and creates wrapper shell scripts for the executables.
# It overrides the defaults set in FindKDE4Internal.cmake.
# For every executable a wrapper script is created, which sets the appropriate
# environment variable for the platform (LD_LIBRARY_PATH on most UNIX systems,
# DYLD_LIBRARY_PATH on OS X and PATH in Windows) so  that it points to the built
# but not yet installed versions of the libraries. So if RPATH is disabled, the executables
# can be run via these scripts from the build tree and will find the correct libraries.
# If RPATH is not disabled, these scripts are also used but only for consistency, because
# they don't really influence anything then, because the compiled-in RPATH overrides
# the LD_LIBRARY_PATH env. variable.
# Executables with the RUN_UNINSTALLED option will be built with the RPATH pointing to the
# build dir, so that they can be run safely without being installed, e.g. as code generators
# for other stuff during the build. These executables will be relinked during "make install".
# All other executables are built with the RPATH with which they will be installed.
macro (KDE4_HANDLE_RPATH_FOR_EXECUTABLE _target_NAME _type)
   if (UNIX)

      # set the RPATH related properties
      if (NOT CMAKE_SKIP_RPATH)
         if (${_type} STREQUAL "GUI")
            set_target_properties(${_target_NAME} PROPERTIES SKIP_BUILD_RPATH TRUE BUILD_WITH_INSTALL_RPATH TRUE)
         endif (${_type} STREQUAL "GUI")

         if (${_type} STREQUAL "NOGUI")
            set_target_properties(${_target_NAME} PROPERTIES SKIP_BUILD_RPATH TRUE BUILD_WITH_INSTALL_RPATH TRUE)
         endif (${_type} STREQUAL "NOGUI")

         if (${_type} STREQUAL "RUN_UNINSTALLED")
            set_target_properties(${_target_NAME} PROPERTIES SKIP_BUILD_RPATH FALSE BUILD_WITH_INSTALL_RPATH FALSE)
         endif (${_type} STREQUAL "RUN_UNINSTALLED")
      endif (NOT CMAKE_SKIP_RPATH)

      if (APPLE)
         set(_library_path_variable "DYLD_LIBRARY_PATH")
      else (APPLE)
         set(_library_path_variable "LD_LIBRARY_PATH")
      endif (APPLE)

      if (APPLE)
         # DYLD_LIBRARY_PATH does not work like LD_LIBRARY_PATH
         # OSX already has the RPATH in libraries and executables, putting runtime directories in
         # DYLD_LIBRARY_PATH actually breaks things
         set(_ld_library_path "${LIBRARY_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}/:${KDE4_LIB_DIR}")
      else (APPLE)
         set(_ld_library_path "${LIBRARY_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}/:${LIB_INSTALL_DIR}:${KDE4_LIB_DIR}:${QT_LIBRARY_DIR}")
      endif (APPLE)
      get_target_property(_executable ${_target_NAME} LOCATION )

      # use add_custom_target() to have the sh-wrapper generated during build time instead of cmake time
      add_custom_command(TARGET ${_target_NAME} POST_BUILD
         COMMAND ${CMAKE_COMMAND}
         -D_filename=${_executable}.shell -D_library_path_variable=${_library_path_variable}
         -D_ld_library_path="${_ld_library_path}" -D_executable=${_executable}
         -P ${KDE4_MODULE_DIR}/kde4_exec_via_sh.cmake
         )

      macro_additional_clean_files(${_executable}.shell)

      # under UNIX, set the property WRAPPER_SCRIPT to the name of the generated shell script
      # so it can be queried and used later on easily
      set_target_properties(${_target_NAME} PROPERTIES WRAPPER_SCRIPT ${_executable}.shell)

   else (UNIX)
      # under windows, set the property WRAPPER_SCRIPT just to the name of the executable
      # maybe later this will change to a generated batch file (for setting the PATH so that the Qt libs are found)
      get_target_property(_executable ${_target_NAME} LOCATION )
      set_target_properties(${_target_NAME} PROPERTIES WRAPPER_SCRIPT ${_executable})

      set(_ld_library_path "${LIBRARY_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}\;${LIB_INSTALL_DIR}\;${KDE4_LIB_DIR}\;${QT_LIBRARY_DIR}")
      get_target_property(_executable ${_target_NAME} LOCATION )

      # use add_custom_target() to have the batch-file-wrapper generated during build time instead of cmake time
      add_custom_command(TARGET ${_target_NAME} POST_BUILD
         COMMAND ${CMAKE_COMMAND}
         -D_filename="${_executable}.bat"
         -D_ld_library_path="${_ld_library_path}" -D_executable="${_executable}"
         -P ${KDE4_MODULE_DIR}/kde4_exec_via_sh.cmake
         )

   endif (UNIX)
endmacro (KDE4_HANDLE_RPATH_FOR_EXECUTABLE)


macro (KDE4_ADD_PLUGIN _target_NAME _with_PREFIX)
#is the first argument is "WITH_PREFIX" then keep the standard "lib" prefix, otherwise set the prefix empty
   if (${_with_PREFIX} STREQUAL "WITH_PREFIX")
      set(_first_SRC)
   else (${_with_PREFIX} STREQUAL "WITH_PREFIX")
      set(_first_SRC ${_with_PREFIX})
   endif (${_with_PREFIX} STREQUAL "WITH_PREFIX")

   set(_SRCS ${_first_SRC} ${ARGN})
   kde4_handle_automoc(${_target_NAME} _SRCS)
   if (KDE4_ENABLE_FINAL)
      kde4_create_final_files(${CMAKE_CURRENT_BINARY_DIR}/${_target_NAME}_final_cpp.cpp _separate_files ${_SRCS})
      add_library(${_target_NAME} MODULE  ${CMAKE_CURRENT_BINARY_DIR}/${_target_NAME}_final_cpp.cpp ${_separate_files})
   else (KDE4_ENABLE_FINAL)
      add_library(${_target_NAME} MODULE ${_SRCS})
   endif (KDE4_ENABLE_FINAL)

   if (_first_SRC)
      set_target_properties(${_target_NAME} PROPERTIES PREFIX "")
   endif (_first_SRC)

   kde4_handle_rpath_for_library(${_target_NAME})

   if (WIN32)
      # for shared libraries/plugins a -DMAKE_target_LIB is required
      string(TOUPPER ${_target_NAME} _symbol)
      string(REGEX REPLACE "[^_A-Za-z0-9]" "_" _symbol ${_symbol})
      set(_symbol "MAKE_${_symbol}_LIB")
      set_target_properties(${_target_NAME} PROPERTIES DEFINE_SYMBOL ${_symbol})
   endif (WIN32)

endmacro (KDE4_ADD_PLUGIN _target_NAME _with_PREFIX)


# this macro is intended to check whether a list of source
# files has the "NOGUI" or "RUN_UNINSTALLED" keywords at the beginning
# in _output_LIST the list of source files is returned with the "NOGUI"
# and "RUN_UNINSTALLED" keywords removed
# if "NOGUI" is in the list of files, the _nogui argument is set to
# "NOGUI" (which evaluates to TRUE in cmake), otherwise it is set empty
# (which evaluates to FALSE in cmake)
# if "RUN_UNINSTALLED" is in the list of files, the _uninst argument is set to
# "RUN_UNINSTALLED" (which evaluates to TRUE in cmake), otherwise it is set empty
# (which evaluates to FALSE in cmake)
# if "TEST" is in the list of files, the _test argument is set to
# "TEST" (which evaluates to TRUE in cmake), otherwise it is set empty
# (which evaluates to FALSE in cmake)
macro(KDE4_CHECK_EXECUTABLE_PARAMS _output_LIST _nogui _uninst _test)
   set(${_nogui})
   set(${_uninst})
   set(${_test})
   set(${_output_LIST} ${ARGN})
   list(LENGTH ${_output_LIST} count)

   list(GET ${_output_LIST} 0 first_PARAM)

   set(second_PARAM "NOTFOUND")
   if (${count} GREATER 1)
      list(GET ${_output_LIST} 1 second_PARAM)
   endif (${count} GREATER 1)

   set(remove "NOTFOUND")

   if (${first_PARAM} STREQUAL "NOGUI")
      set(${_nogui} "NOGUI")
      set(remove 0)
   endif (${first_PARAM} STREQUAL "NOGUI")

   if (${second_PARAM} STREQUAL "NOGUI")
      set(${_nogui} "NOGUI")
      set(remove 0;1)
   endif (${second_PARAM} STREQUAL "NOGUI")

   if (${first_PARAM} STREQUAL "RUN_UNINSTALLED")
      set(${_uninst} "RUN_UNINSTALLED")
      set(remove 0)
   endif (${first_PARAM} STREQUAL "RUN_UNINSTALLED")

   if (${second_PARAM} STREQUAL "RUN_UNINSTALLED")
      set(${_uninst} "RUN_UNINSTALLED")
      set(remove 0;1)
   endif (${second_PARAM} STREQUAL "RUN_UNINSTALLED")

   if (${first_PARAM} STREQUAL "TEST")
      set(${_test} "TEST")
      set(remove 0)
   endif (${first_PARAM} STREQUAL "TEST")

   if (${second_PARAM} STREQUAL "TEST")
      set(${_test} "TEST")
      set(remove 0;1)
   endif (${second_PARAM} STREQUAL "TEST")



   if (NOT "${remove}" STREQUAL "NOTFOUND")
      list(REMOVE_AT ${_output_LIST} ${remove})
   endif (NOT "${remove}" STREQUAL "NOTFOUND")

endmacro(KDE4_CHECK_EXECUTABLE_PARAMS)


macro (KDE4_ADD_KDEINIT_EXECUTABLE _target_NAME )

   kde4_check_executable_params(_SRCS _nogui _uninst _test ${ARGN})

#   if (WIN32)
#      # under windows, just build a normal executable
#      KDE4_ADD_EXECUTABLE(${_target_NAME} ${CMAKE_CURRENT_BINARY_DIR}/${_target_NAME}_dummy.cpp ${ARGN} )
#   else (WIN32)
      # under UNIX, create a shared library and a small executable, which links to this library

   kde4_handle_automoc(kdeinit_${_target_NAME} _SRCS)
   if (KDE4_ENABLE_FINAL)
      kde4_create_final_files(${CMAKE_CURRENT_BINARY_DIR}/kdeinit_${_target_NAME}_final_cpp.cpp _separate_files ${_SRCS})
      add_library(kdeinit_${_target_NAME} SHARED  ${CMAKE_CURRENT_BINARY_DIR}/kdeinit_${_target_NAME}_final_cpp.cpp ${_separate_files})

   else (KDE4_ENABLE_FINAL)
      add_library(kdeinit_${_target_NAME} SHARED ${_SRCS})
   endif (KDE4_ENABLE_FINAL)

   kde4_handle_rpath_for_library(kdeinit_${_target_NAME})
   set_target_properties(kdeinit_${_target_NAME} PROPERTIES OUTPUT_NAME kdeinit4_${_target_NAME})

   configure_file(${KDE4_MODULE_DIR}/kde4init_dummy.cpp.in ${CMAKE_CURRENT_BINARY_DIR}/${_target_NAME}_dummy.cpp)
   kde4_add_executable(${_target_NAME} "${_nogui}" "${_uninst}" ${CMAKE_CURRENT_BINARY_DIR}/${_target_NAME}_dummy.cpp)
   target_link_libraries(${_target_NAME} kdeinit_${_target_NAME})
#   endif (WIN32)

   if (WIN32)
      target_link_libraries(${_target_NAME} ${QT_QTMAIN_LIBRARY})
   endif (WIN32)

endmacro (KDE4_ADD_KDEINIT_EXECUTABLE)

# add a unit test, which is executed when running make test
# it will be built with RPATH pointing to the build dir
# The targets are always created, but only built for the "all"
# target if the option KDE4_BUILD_TESTS is enabled. Otherwise the rules for the target
# are created but not built by default. You can build them by manually building the target.
# The name of the target can be specified using TESTNAME <testname>, if it is not given
# the macro will default to the <name>
macro (KDE4_ADD_UNIT_TEST _test_NAME)
    set(_srcList ${ARGN})
    set(_targetName ${_test_NAME})
    if( ${ARGV1} STREQUAL "TESTNAME" )
        set(_targetName ${ARGV2})
        list(REMOVE_AT _srcList 0 1)
    endif( ${ARGV1} STREQUAL "TESTNAME" )
    kde4_add_executable( ${_test_NAME} TEST ${_srcList} )

    if(NOT KDE4_TEST_OUTPUT)
        set(KDE4_TEST_OUTPUT plaintext)
    endif(NOT KDE4_TEST_OUTPUT)
    set(KDE4_TEST_OUTPUT ${KDE4_TEST_OUTPUT} CACHE STRING "The output to generate when running the QTest unit tests")

    set(using_qtest "")
    foreach(_filename ${_srcList})
        if(NOT using_qtest)
            if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${_filename}")
                file(READ ${_filename} file_CONTENT)
                string(REGEX MATCH "QTEST_(KDE)?MAIN" using_qtest "${file_CONTENT}")
            endif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${_filename}")
        endif(NOT using_qtest)
    endforeach(_filename)

    if (using_qtest AND KDE4_TEST_OUTPUT STREQUAL "xml")
        #MESSAGE(STATUS "${_targetName} : Using QTestLib, can produce XML report.")
        add_test( ${_targetName} ${EXECUTABLE_OUTPUT_PATH}/${_test_NAME} -xml -o ${_targetName}.tml)
    else (using_qtest AND KDE4_TEST_OUTPUT STREQUAL "xml")
        #MESSAGE(STATUS "${_targetName} : NOT using QTestLib, can't produce XML report, please use QTestLib to write your unit tests.")
        add_test( ${_targetName} ${EXECUTABLE_OUTPUT_PATH}/${_test_NAME} )
    endif (using_qtest AND KDE4_TEST_OUTPUT STREQUAL "xml")

#    add_test( ${_targetName} ${EXECUTABLE_OUTPUT_PATH}/${_test_NAME} -xml -o ${_test_NAME}.tml )

    if (NOT MSVC_IDE)   #not needed for the ide
        # if the tests are EXCLUDE_FROM_ALL, add a target "buildtests" to build all tests
        if (NOT KDE4_BUILD_TESTS)
           get_directory_property(_buildtestsAdded BUILDTESTS_ADDED)
           if(NOT _buildtestsAdded)
              add_custom_target(buildtests)
              set_directory_properties(PROPERTIES BUILDTESTS_ADDED TRUE)
           endif(NOT _buildtestsAdded)
           add_dependencies(buildtests ${_test_NAME})
        endif (NOT KDE4_BUILD_TESTS)
    endif (NOT MSVC_IDE)

endmacro (KDE4_ADD_UNIT_TEST)

macro (KDE4_ADD_TEST_EXECUTABLE _target_NAME)
   MESSAGE(SEND_ERROR "KDE4_ADD_TEST_EXECUTABLE is deprecated use KDE4_ADD_EXECUTABLE(<target> TEST <files>) instead")
endmacro (KDE4_ADD_TEST_EXECUTABLE)


macro (KDE4_ADD_EXECUTABLE _target_NAME)

   kde4_check_executable_params( _SRCS _nogui _uninst _test ${ARGN})

   set(_add_executable_param)
   set(_type "GUI")

   # determine additional parameters for add_executable()
   # for GUI apps, create a bundle on OSX
   if (Q_WS_MAC)
      set(_add_executable_param MACOSX_BUNDLE)
   endif (Q_WS_MAC)

   # for GUI apps, this disables the additional console under Windows
   if (WIN32)
      set(_add_executable_param WIN32)
   endif (WIN32)

   if (_nogui)
      set(_type "NOGUI")
      set(_add_executable_param)
   endif (_nogui)

   if (_uninst OR _test)
      set(_type "RUN_UNINSTALLED")
   endif (_uninst OR _test)

   if (_test AND NOT KDE4_BUILD_TESTS)
      set(_add_executable_param ${_add_executable_param} EXCLUDE_FROM_ALL)
   endif (_test AND NOT KDE4_BUILD_TESTS)

   kde4_handle_automoc(${_target_NAME} _SRCS)
   if (KDE4_ENABLE_FINAL)
      kde4_create_final_files(${CMAKE_CURRENT_BINARY_DIR}/${_target_NAME}_final_cpp.cpp _separate_files ${_SRCS})
      add_executable(${_target_NAME} ${_add_executable_param} ${CMAKE_CURRENT_BINARY_DIR}/${_target_NAME}_final_cpp.cpp ${_separate_files})
   else (KDE4_ENABLE_FINAL)
      add_executable(${_target_NAME} ${_add_executable_param} ${_SRCS})
   endif (KDE4_ENABLE_FINAL)

   if (_test)
      set_target_properties(${_target_NAME} PROPERTIES COMPILE_FLAGS -DKDESRCDIR="\\"${CMAKE_CURRENT_SOURCE_DIR}\\"")
   endif (_test)

   kde4_handle_rpath_for_executable(${_target_NAME} ${_type})

   if (WIN32)
      target_link_libraries(${_target_NAME} ${QT_QTMAIN_LIBRARY})
   endif (WIN32)

endmacro (KDE4_ADD_EXECUTABLE)


macro (KDE4_ADD_LIBRARY _target_NAME _lib_TYPE)
#is the first argument is "WITH_PREFIX" then keep the standard "lib" prefix, otherwise set the prefix empty

   set(_first_SRC ${_lib_TYPE})
   set(_add_lib_param)

   if (${_lib_TYPE} STREQUAL "STATIC")
      set(_first_SRC)
      set(_add_lib_param STATIC)
   endif (${_lib_TYPE} STREQUAL "STATIC")
   if (${_lib_TYPE} STREQUAL "SHARED")
      set(_first_SRC)
      set(_add_lib_param SHARED)
   endif (${_lib_TYPE} STREQUAL "SHARED")
   if (${_lib_TYPE} STREQUAL "MODULE")
      set(_first_SRC)
      set(_add_lib_param MODULE)
   endif (${_lib_TYPE} STREQUAL "MODULE")

   set(_SRCS ${_first_SRC} ${ARGN})
   kde4_handle_automoc(${_target_NAME} _SRCS)
   if (KDE4_ENABLE_FINAL)
      kde4_create_final_files(${CMAKE_CURRENT_BINARY_DIR}/${_target_NAME}_final_cpp.cpp _separate_files ${_SRCS})
      add_library(${_target_NAME} ${_add_lib_param}  ${CMAKE_CURRENT_BINARY_DIR}/${_target_NAME}_final_cpp.cpp ${_separate_files})
   else (KDE4_ENABLE_FINAL)
      add_library(${_target_NAME} ${_add_lib_param} ${_SRCS})
   endif (KDE4_ENABLE_FINAL)

   kde4_handle_rpath_for_library(${_target_NAME})

   # for shared libraries a -DMAKE_target_LIB is required
   string(TOUPPER ${_target_NAME} _symbol)
   string(REGEX REPLACE "[^_A-Za-z0-9]" "_" _symbol ${_symbol})
   set(_symbol "MAKE_${_symbol}_LIB")
   set_target_properties(${_target_NAME} PROPERTIES DEFINE_SYMBOL ${_symbol})

endmacro (KDE4_ADD_LIBRARY _target_NAME _lib_TYPE)


macro (KDE4_ADD_WIDGET_FILES _sources)
   foreach (_current_FILE ${ARGN})

      get_filename_component(_input ${_current_FILE} ABSOLUTE)
      get_filename_component(_basename ${_input} NAME_WE)
      set(_source ${CMAKE_CURRENT_BINARY_DIR}/${_basename}widgets.cpp)
      set(_moc ${CMAKE_CURRENT_BINARY_DIR}/${_basename}widgets.moc)

      # create source file from the .widgets file
      add_custom_command(OUTPUT ${_source}
        COMMAND ${KDE4_MAKEKDEWIDGETS_EXECUTABLE}
        ARGS -o ${_source} ${_input}
        MAIN_DEPENDENCY ${_input} DEPENDS ${_KDE4_MAKEKDEWIDGETS_DEP})

      # create moc file
      qt4_generate_moc(${_source} ${_moc} )

      list(APPEND ${_sources} ${_source} ${_moc})

   endforeach (_current_FILE)

endmacro (KDE4_ADD_WIDGET_FILES)


MACRO(KDE4_REMOVE_OBSOLETE_CMAKE_FILES)
# the files listed here will be removed by remove_obsoleted_cmake_files.cmake, Alex
   install(SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/remove_files.cmake )
   set(module_install_dir ${DATA_INSTALL_DIR}/cmake/modules )

   file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/remove_files.cmake "#generated by cmake, dont edit\n\n")
   foreach ( _current_FILE ${ARGN})
      file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/remove_files.cmake "message(STATUS \"Removing ${module_install_dir}/${_current_FILE}\" )\n" )
      file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/remove_files.cmake "exec_program( ${CMAKE_COMMAND} ARGS -E remove ${module_install_dir}/${_current_FILE} OUTPUT_VARIABLE _dummy)\n" )
   endforeach ( _current_FILE)

ENDMACRO(KDE4_REMOVE_OBSOLETE_CMAKE_FILES)


MACRO(KDE4_NO_ENABLE_FINAL _project_name)
   if(KDE4_ENABLE_FINAL)
      set(KDE4_ENABLE_FINAL OFF)
      remove_definitions(-DKDE_USE_FINAL)
      message(STATUS "You used enable-final argument but \"${_project_name}\" doesn't support it. Try to fix compile it and remove KDE4_NO_ENABLE_FINAL macro. Thanks")

   endif(KDE4_ENABLE_FINAL)
ENDMACRO(KDE4_NO_ENABLE_FINAL _project_name)


macro(KDE4_CREATE_EXPORTS_HEADER _outputFile _libName)
   string(TOUPPER ${_libName} _libNameUpperCase)
   string(REGEX REPLACE "[^_A-Za-z0-9]" "_" _libNameUpperCase ${_libNameUpperCase})
   # the next line is is required, because in CMake arguments to macros are not real
   # variables, but handled differently. The next line create a real CMake variable,
   # so configure_file() will replace it correctly.
   set(_libName ${_libName})
   # compared to write(FILE) configure_file() only really writes the file if the
   # contents have changed. Otherwise we would have a lot of recompiles.
   configure_file(${KDE4_MODULE_DIR}/kde4exportsheader.h.in ${_outputFile})
endmacro(KDE4_CREATE_EXPORTS_HEADER _outputFile _libName)


macro (KDE4_CREATE_HTML_HANDBOOK _docbook)
   message(STATUS "KDE4_CREATE_HTML_HANDBOOK() is deprecated. Enable the option KDE4_ENABLE_HTMLHANDBOOK instead, this will give you targets htmlhandbook for creating the html help.")
endmacro (KDE4_CREATE_HTML_HANDBOOK)


# adds application icon to target source list
# 'appsources' - the sources of the application
# 'pngfiles' - specifies the list of icon files
# example: KDE4_ADD_WIN32_APP_ICON(myapp_SRCS "pics/cr16-myapp.png;pics/cr32-myapp.png")

macro (KDE4_ADD_WIN32_APP_ICON appsources)
    if (WIN32)
        find_program(PNG2ICO_EXECUTABLE NAMES png2ico)
        find_program(WINDRES_EXECUTABLE NAMES windres)
        if(MSVC)
            set(WINDRES_EXECUTABLE TRUE)
        endif(MSVC)
        STRING(REPLACE _SRCS "" appname ${appsources})
        if (PNG2ICO_EXECUTABLE AND WINDRES_EXECUTABLE)
            set (_outfilename ${CMAKE_CURRENT_BINARY_DIR}/${appname})

            # png2ico is found by the above find_program
#            message("png2ico ${_outfilename}.ico ${ARGN}")
            exec_program(png2ico ARGS ${_outfilename}.ico ${ARGN})

            # now make rc file for adding it to the sources
            file(WRITE ${_outfilename}.rc "IDI_ICON1        ICON        DISCARDABLE    \"${_outfilename}.ico\"\n")
            if (MINGW)
                exec_program(windres
                    ARGS "-i ${_outfilename}.rc -o ${_outfilename}_res.o --include-dir=${CMAKE_CURRENT_SOURCE_DIR}")
                list(APPEND ${appsources} ${CMAKE_CURRENT_BINARY_DIR}/${appname}_res.o)
            else(MINGW)
                list(APPEND ${appsources} ${CMAKE_CURRENT_BINARY_DIR}/${appname}.rc)
            endif(MINGW)
        endif(PNG2ICO_EXECUTABLE AND WINDRES_EXECUTABLE)
    endif(WIN32)
endmacro (KDE4_ADD_WIN32_APP_ICON)

