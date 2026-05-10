cmake_minimum_required(VERSION 3.25)

if(DEFINED ENV{VCPKG_ROOT})
    file(TO_CMAKE_PATH $ENV{VCPKG_ROOT} VCPKG_ROOT)
    if(EXISTS "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
        set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
        set(USING_VCPKG ON)
    endif()
endif()
if(WIN32)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()
if(POLICY CMP0141)
    cmake_policy(SET CMP0141 NEW)
    set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_program(CLANG_FORMAT clang-format)
find_program(CLANG_TIDY clang-tidy)

project("libdesktop" LANGUAGES C CXX VERSION 2025.5.0 DESCRIPTION "A cross-platform desktop library.")

option(BUILD_SHARED_LIBS "Build libdesktop as a shared library" OFF)
option(BUILD_TESTING "Build tests" ON)

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

add_library(sqlcipher STATIC "third-party/sqlcipher/sqlite3.c")
target_compile_definitions(sqlcipher PUBLIC
    SQLITE_HAS_CODEC
    SQLITE_TEMP_STORE=3
    SQLITE_EXTRA_INIT=sqlcipher_extra_init
    SQLITE_EXTRA_SHUTDOWN=sqlcipher_extra_shutdown
    SQLITE_THREADSAFE=1)

find_package(OpenSSL REQUIRED)
target_link_libraries(sqlcipher PUBLIC OpenSSL::SSL OpenSSL::Crypto)
target_include_directories(sqlcipher PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/third-party/sqlcipher>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}>")

add_library(${PROJECT_NAME}
    "include/app.h"
    "include/database.h"
    "include/events.h"
    "include/filesystem.h"
    "include/helpers.h"
    "include/hosting.h"
    "include/libdesktop.h"
    "include/network.h"
    "include/notifications.h"
    "include/secrets.h"
    "include/services.h"
    "include/system.h"
    "include/updates.h"
    "include/app/app_info.h"
    "include/app/arguments_service.h"
    "include/app/configuration_saved_event_args.h"
    "include/app/configuration_service.h"
    "include/app/keyring_service.h"
    "include/app/log_type.h"
    "include/app/logger.h"
    "include/app/translation_service.h"
    "include/app/window_geometry.h"
    "include/database/database_service.h"
    "include/database/database_value.h"
    "include/events/event.h"
    "include/events/event_args.h"
    "include/events/param_event_args.h"
    "include/filesystem/user_directories.h"
    "include/helpers/string_manip.h"
    "include/helpers/uuid.h"
    "include/hosting/host.h"
    "include/hosting/host_options.h"
    "include/hosting/lifetime_service.h"
    "include/network/download_progress.h"
    "include/network/http_response.h"
    "include/network/http_service.h"
    "include/network/network_monitor.h"
    "include/notifications/notification.h"
    "include/notifications/notification_sent_event_args.h"
    "include/notifications/notification_service.h"
    "include/notifications/notification_severity.h"
    "include/notifications/shell_notification.h"
    "include/secrets/credential.h"
    "include/secrets/password_content.h"
    "include/secrets/password_generator.h"
    "include/secrets/secret.h"
    "include/secrets/secret_service.h"
    "include/services/service_collection.h"
    "include/services/service_provider.h"
    "include/services/service_scope.h"
    "include/system/dependency_search_option.h"
    "include/system/deployment_mode.h"
    "include/system/environment.h"
    "include/system/power_service.h"
    "include/system/process.h"
    "include/system/process_status.h"
    "include/updates/release.h"
    "include/updates/release_asset.h"
    "include/updates/version.h"
    "src/app/app_info.cpp"
    "src/app/arguments_service.cpp"
    "src/app/configuration_saved_event_args.cpp"
    "src/app/configuration_service.cpp"
    "src/app/keyring_service.cpp"
    "src/app/logger.cpp"
    "src/app/translation_service.cpp"
    "src/app/window_geometry.cpp"
    "src/database/database_service.cpp"
    "src/database/database_value.cpp"
    "src/helpers/string_manip.cpp"
    "src/helpers/uuid.cpp"
    "src/hosting/host.cpp"
    "src/hosting/host_options.cpp"
    "src/hosting/lifetime_service.cpp"
    "src/network/download_progress.cpp"
    "src/network/http_response.cpp"
    "src/network/http_service.cpp"
    "src/network/network_monitor.cpp"
    "src/notifications/notification.cpp"
    "src/notifications/notification_sent_event_args.cpp"
    "src/notifications/notification_service.cpp"
    "src/notifications/shell_notification.cpp"
    "src/secrets/credential.cpp"
    "src/secrets/password_generator.cpp"
    "src/secrets/secret.cpp"
    "src/services/service_collection.cpp"
    "src/updates/release.cpp"
    "src/updates/release_asset.cpp"
    "src/updates/version.cpp")

if(WIN32)
    target_sources(${PROJECT_NAME} PRIVATE
        "src/filesystem/user_directories_windows.cpp"
        "src/secrets/secret_service_windows.cpp"
        "src/system/environment_windows.cpp"
        "src/system/power_service_windows.cpp"
        "src/system/process_windows.cpp")
endif()
if(UNIX)
    target_sources(${PROJECT_NAME} PRIVATE
        "src/system/process_unix.cpp")
endif()
if(APPLE)
    target_sources(${PROJECT_NAME} PRIVATE
        "src/filesystem/user_directories_macos.cpp"
        "src/secrets/secret_service_macos.cpp"
        "src/system/environment_macos.cpp"
        "src/system/power_service_macos.cpp")
endif()
if(LINUX)
    target_sources(${PROJECT_NAME} PRIVATE
        "src/filesystem/user_directories_linux.cpp"
        "src/secrets/secret_service_linux.cpp"
        "src/system/environment_linux.cpp"
        "src/system/power_service_linux.cpp")
endif()

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

if(CLANG_TIDY)
    if(MSVC)
        set_target_properties(${PROJECT_NAME} PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY};--extra-arg=/EHsc")
    else()
        set_target_properties(${PROJECT_NAME} PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY};--extra-arg=-fexceptions")
    endif()
endif()

if(CLANG_FORMAT)
    file(GLOB_RECURSE FORMAT_SOURCE_FILES CONFIGURE_DEPENDS
        "${CMAKE_SOURCE_DIR}/include/*.h"
        "${CMAKE_SOURCE_DIR}/src/*.cpp"
        "${CMAKE_SOURCE_DIR}/tests/*.cpp"
        "${CMAKE_SOURCE_DIR}/tests/*.h")
    add_custom_target(format
        COMMAND ${CLANG_FORMAT} -i ${FORMAT_SOURCE_FILES}
        COMMENT "Running clang-format")
    add_dependencies(${PROJECT_NAME} format)
endif()

target_include_directories(${PROJECT_NAME} PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}>")

set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION "${PROJECT_VERSION}")

set_property(TARGET ${PROJECT_NAME} APPEND PROPERTY COMPATIBLE_INTERFACE_STRING "${PROJECT_VERSION}")

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
    target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

find_package(CURL REQUIRED)
find_package(Intl REQUIRED)
find_package(maddy CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC CURL::libcurl Intl::Intl maddy::maddy nlohmann_json::nlohmann_json sqlcipher)
if(WIN32)
    target_link_libraries(${PROJECT_NAME} PUBLIC Kernel32 Shell32)
else()
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(glib REQUIRED IMPORTED_TARGET glib-2.0)
    pkg_check_modules(gio REQUIRED IMPORTED_TARGET gio-2.0)
    pkg_check_modules(gmodule REQUIRED IMPORTED_TARGET gmodule-2.0)
    pkg_check_modules(gobject REQUIRED IMPORTED_TARGET gobject-2.0)
    pkg_check_modules(gthread REQUIRED IMPORTED_TARGET gthread-2.0)
    target_link_libraries(${PROJECT_NAME} PUBLIC Threads::Threads PkgConfig::glib PkgConfig::gio PkgConfig::gmodule PkgConfig::gobject PkgConfig::gthread)
endif()
if(LINUX)
    pkg_check_modules(libsecret REQUIRED IMPORTED_TARGET libsecret-1)
    target_link_libraries(${PROJECT_NAME} PUBLIC  PkgConfig::libsecret)
endif()
if(APPLE)
    find_library(CF_LIBRARY CoreFoundation)
    find_library(CS_LIBRARY CoreServices)
    find_library(IOKIT_LIBRARY IOKit)
    find_library(SECURITY_LIBRARY Security)
    target_link_libraries(${PROJECT_NAME} PUBLIC ${CF_LIBRARY} ${CS_LIBRARY} ${IOKIT_LIBRARY} ${SECURITY_LIBRARY})
endif()

configure_file("${CMAKE_SOURCE_DIR}/cmake/${PROJECT_NAME}.pc.in" "${CMAKE_BINARY_DIR}/cmake/${PROJECT_NAME}.pc" @ONLY)
configure_package_config_file("${CMAKE_SOURCE_DIR}/cmake/config.cmake.in" "${CMAKE_BINARY_DIR}/cmake/${PROJECT_NAME}Config.cmake" INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")
write_basic_package_version_file("${CMAKE_BINARY_DIR}/cmake/${PROJECT_NAME}ConfigVersion.cmake" VERSION "${PROJECT_VERSION}" COMPATIBILITY AnyNewerVersion)
install(TARGETS ${PROJECT_NAME} sqlcipher EXPORT "${PROJECT_NAME}Targets"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
install(DIRECTORY "${CMAKE_SOURCE_DIR}/include/" DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}")
install(FILES "third-party/sqlcipher/sqlite3.h" DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}")
install(FILES "${CMAKE_BINARY_DIR}/cmake/${PROJECT_NAME}.pc" DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")
install(EXPORT "${PROJECT_NAME}Targets" FILE "${PROJECT_NAME}Targets.cmake" NAMESPACE ${PROJECT_NAME}:: DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")
install(FILES "${CMAKE_BINARY_DIR}/cmake/${PROJECT_NAME}Config.cmake" "${CMAKE_BINARY_DIR}/cmake/${PROJECT_NAME}ConfigVersion.cmake" DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")
install(FILES
    "${CMAKE_SOURCE_DIR}/cmake/DesktopHelpers.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")
export(EXPORT "${PROJECT_NAME}Targets" FILE "${CMAKE_BINARY_DIR}/cmake/${PROJECT_NAME}Targets.cmake" NAMESPACE ${PROJECT_NAME}::)

if (BUILD_TESTING)
    enable_testing()
    find_package(GTest CONFIG REQUIRED)
    include(GoogleTest)

    add_executable(${PROJECT_NAME}_tests
        "tests/main.cpp")

    target_link_libraries(${PROJECT_NAME}_tests PRIVATE ${PROJECT_NAME} GTest::gtest GTest::gmock)

    gtest_discover_tests(${PROJECT_NAME}_tests)
endif()