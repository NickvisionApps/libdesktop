# DesktopHelpers
# Reusable CMake helper functions for desktop application projects.
#
# Functions provided:
#   desktop_generate_translations()    – xgettext / msgmerge / msgfmt
#   desktop_compile_glib_resources()   – glib-compile-resources (GNOME only)
#   desktop_compile_blueprints()       – blueprint-compiler    (GNOME only)

# ------------------------------------------------------------------------------
# desktop_generate_translations
#
# Runs xgettext, msgmerge, and msgfmt as POST_BUILD steps on a given target.
#
# Usage:
#   desktop_generate_translations(
#       TARGET          <cmake-target>
#       SHORT_NAME      <name>
#       [LANGUAGE       <xgettext-language>]  # default: C++
#       [ROOT_DIRECTORY <dir>]                # default: CMAKE_SOURCE_DIR
#       [OUTPUT_DIRECTORY <dir>]              # default: CMAKE_BINARY_DIR
#   )
#
# Expected files under ROOT_DIRECTORY:
#   resources/po/POTFILES          – source files to scan (one per line)
#   resources/po/LINGUAS           – language codes to compile (one per line)
#   resources/po/<SHORT_NAME>.pot  – template file (created/updated by xgettext)
function(desktop_generate_translations)
    cmake_parse_arguments(ARG "" "TARGET;SHORT_NAME;LANGUAGE;ROOT_DIRECTORY;OUTPUT_DIRECTORY" "" ${ARGN})

    if(NOT ARG_TARGET)
        message(FATAL_ERROR "desktop_generate_translations: TARGET is required")
    endif()
    if(NOT ARG_SHORT_NAME)
        message(FATAL_ERROR "desktop_generate_translations: SHORT_NAME is required")
    endif()
    if(NOT ARG_LANGUAGE)
        set(ARG_LANGUAGE "C++")
    endif()
    if(NOT ARG_ROOT_DIRECTORY)
        set(ARG_ROOT_DIRECTORY "${CMAKE_SOURCE_DIR}")
    endif()
    if(NOT ARG_OUTPUT_DIRECTORY)
        set(ARG_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
    endif()

    find_program(XGETTEXT xgettext REQUIRED)
    find_program(MSGMERGE msgmerge REQUIRED)
    find_program(MSGFMT   msgfmt   REQUIRED)

    set(_po_dir   "${ARG_ROOT_DIRECTORY}/resources/po")
    set(_template "${_po_dir}/${ARG_SHORT_NAME}.pot")

    if(NOT EXISTS "${_po_dir}/POTFILES")
        message(FATAL_ERROR "desktop_generate_translations: ${_po_dir}/POTFILES not found")
    endif()
    if(NOT EXISTS "${_po_dir}/LINGUAS")
        message(FATAL_ERROR "desktop_generate_translations: ${_po_dir}/LINGUAS not found")
    endif()

    file(STRINGS "${_po_dir}/POTFILES" _potfiles)
    file(STRINGS "${_po_dir}/LINGUAS"  _linguas)

    add_custom_command(TARGET ${ARG_TARGET} POST_BUILD
        COMMENT "Generating translations..."
        WORKING_DIRECTORY "${ARG_ROOT_DIRECTORY}"
        COMMAND ${XGETTEXT}
            --from-code=utf-8
            --language=${ARG_LANGUAGE}
            --force-po
            --output=${_template}
            --keyword=_
            --keyword=_n:1,2
            --keyword=_p:1c,2
            --keyword=_pn:1c,2,3
            --keyword=C_:1c,2
            --width=80
            ${_potfiles}
        VERBATIM)

    foreach(_lang IN LISTS _linguas)
        add_custom_command(TARGET ${ARG_TARGET} POST_BUILD
            WORKING_DIRECTORY "${ARG_ROOT_DIRECTORY}"
            COMMAND ${CMAKE_COMMAND} -E make_directory "${ARG_OUTPUT_DIRECTORY}/${_lang}"
            COMMAND ${MSGMERGE} --backup=off --update
                "${_po_dir}/${_lang}.po" "${_template}"
            COMMAND ${MSGFMT}
                "${_po_dir}/${_lang}.po"
                --output-file="${ARG_OUTPUT_DIRECTORY}/${_lang}/${ARG_SHORT_NAME}.mo"
            VERBATIM)
    endforeach()
endfunction()

# ------------------------------------------------------------------------------
# desktop_compile_glib_resources
#
# Runs glib-compile-resources as a POST_BUILD step. GNOME/Linux projects only.
#
# Usage:
#   desktop_compile_glib_resources(
#       TARGET          <cmake-target>
#       RESOURCE_XML    <path-to-.gresource.xml>
#       OUTPUT          <output-file-path>
#       [SOURCE_DIR     <resource-source-dir>]  # default: directory of RESOURCE_XML
#   )
function(desktop_compile_glib_resources)
    cmake_parse_arguments(ARG "" "TARGET;RESOURCE_XML;OUTPUT;SOURCE_DIR" "" ${ARGN})

    if(NOT ARG_TARGET)
        message(FATAL_ERROR "desktop_compile_glib_resources: TARGET is required")
    endif()
    if(NOT ARG_RESOURCE_XML)
        message(FATAL_ERROR "desktop_compile_glib_resources: RESOURCE_XML is required")
    endif()
    if(NOT ARG_OUTPUT)
        message(FATAL_ERROR "desktop_compile_glib_resources: OUTPUT is required")
    endif()
    if(NOT ARG_SOURCE_DIR)
        get_filename_component(ARG_SOURCE_DIR "${ARG_RESOURCE_XML}" DIRECTORY)
    endif()

    find_program(GLIB_COMPILE_RESOURCES glib-compile-resources REQUIRED)

    add_custom_command(TARGET ${ARG_TARGET} POST_BUILD
        COMMENT "Compiling glib resources..."
        COMMAND ${GLIB_COMPILE_RESOURCES}
            --sourcedir "${ARG_SOURCE_DIR}"
            "${ARG_RESOURCE_XML}"
            --target="${ARG_OUTPUT}"
        VERBATIM)
endfunction()

# ------------------------------------------------------------------------------
# desktop_compile_blueprints
#
# Runs blueprint-compiler batch-compile as a POST_BUILD step. GNOME projects only.
#
# On Windows the blueprint-compiler Python script is invoked via the python
# interpreter found in the MSYS2 prefix appropriate for the target architecture
# (clangarm64 for ARM64, mingw64 otherwise). On all other platforms the
# blueprint-compiler executable is located via PATH.
#
# Usage:
#   desktop_compile_blueprints(
#       TARGET          <cmake-target>
#       BLUEPRINT_DIR   <directory-containing-.blp-files>
#       OUTPUT_DIR      <directory-for-compiled-ui-files>
#   )
function(desktop_compile_blueprints)
    cmake_parse_arguments(ARG "" "TARGET;BLUEPRINT_DIR;OUTPUT_DIR" "" ${ARGN})

    if(NOT ARG_TARGET)
        message(FATAL_ERROR "desktop_compile_blueprints: TARGET is required")
    endif()
    if(NOT ARG_BLUEPRINT_DIR)
        message(FATAL_ERROR "desktop_compile_blueprints: BLUEPRINT_DIR is required")
    endif()
    if(NOT ARG_OUTPUT_DIR)
        message(FATAL_ERROR "desktop_compile_blueprints: OUTPUT_DIR is required")
    endif()

    if(WIN32)
        find_program(PYTHON_EXECUTABLE python REQUIRED)
        if(CMAKE_GENERATOR_PLATFORM MATCHES "ARM64" OR CMAKE_SYSTEM_PROCESSOR MATCHES "ARM64")
            set(_bp_command "${PYTHON_EXECUTABLE}" "C:/msys64/clangarm64/bin/blueprint-compiler")
        else()
            set(_bp_command "${PYTHON_EXECUTABLE}" "C:/msys64/mingw64/bin/blueprint-compiler")
        endif()
    else()
        find_program(BLUEPRINT_COMPILER blueprint-compiler REQUIRED)
        set(_bp_command "${BLUEPRINT_COMPILER}")
    endif()

    file(GLOB_RECURSE _blp_files CONFIGURE_DEPENDS "${ARG_BLUEPRINT_DIR}/*.blp")

    add_custom_command(TARGET ${ARG_TARGET} POST_BUILD
        COMMENT "Compiling blueprints..."
        COMMAND ${_bp_command}
            batch-compile
            "${ARG_OUTPUT_DIR}"
            "${ARG_BLUEPRINT_DIR}"
            ${_blp_files}
        VERBATIM)
endfunction()
