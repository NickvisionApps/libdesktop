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

project("libdesktop" LANGUAGES CXX VERSION 2025.5.0 DESCRIPTION "A cross-platform desktop library.")

option(BUILD_SHARED_LIBS "Build libdesktop as a shared library" OFF)
option(BUILD_TESTING "Build tests" ON)

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

add_library(${PROJECT_NAME}
    "include/app.h"
    "include/events.h"
    "include/filesystem.h"
    "include/helpers.h"
    "include/hosting.h"
    "include/libdesktop.h"
    "include/notifications.h"
    "include/secrets.h"
    "include/services.h"
    "include/system.h"
    "include/updates.h"
    "include/app/app_info.h"
    "include/app/arguments_service.h"
    "include/app/logger.h"
    "include/app/log_type.h"
    "include/events/event.h"
    "include/events/event_args.h"
    "include/events/param_event_args.h"
    "include/filesystem/user_directories.h"
    "include/helpers/string_manip.h"
    "include/helpers/uuid.h"
    "include/hosting/host.h"
    "include/hosting/host_options.h"
    "include/hosting/lifetime_service.h"
    "include/notifications/notification.h"
    "include/notifications/notification_sent_event_args.h"
    "include/notifications/notification_service.h"
    "include/notifications/notification_severity.h"
    "include/notifications/shell_notification.h"
    "include/secrets/password_content.h"
    "include/secrets/password_generator.h"
    "include/secrets/secret.h"
    "include/secrets/secret_service.h"
    "include/services/service.h"
    "include/services/service_collection.h"
    "include/services/service_provider.h"
    "include/services/service_scope.h"
    "include/system/power_service.h"
    "include/system/process.h"
    "include/system/process_status.h"
    "include/updates/release.h"
    "include/updates/release_asset.h"
    "include/updates/version.h"
    "src/app/app_info.cpp"
    "src/app/arguments_service.cpp"
    "src/app/logger.cpp"
    "src/helpers/string_manip.cpp"
    "src/helpers/uuid.cpp"
    "src/hosting/host.cpp"
    "src/hosting/host_options.cpp"
    "src/hosting/lifetime_service.cpp"
    "src/notifications/notification.cpp"
    "src/notifications/notification_sent_event_args.cpp"
    "src/notifications/notification_service.cpp"
    "src/notifications/shell_notification.cpp"
    "src/secrets/password_generator.cpp"
    "src/secrets/secret.cpp"
    "src/updates/release.cpp"
    "src/updates/release_asset.cpp"
    "src/updates/version.cpp")

if(WIN32)
    target_sources(${PROJECT_NAME} PRIVATE
        "src/filesystem/user_directories_windows.cpp"
        "src/secrets/secret_service_windows.cpp"
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
        "src/system/power_service_macos.cpp")
endif()
if(LINUX)
    target_sources(${PROJECT_NAME} PRIVATE
        "src/filesystem/user_directories_linux.cpp"
        "src/secrets/secret_service_linux.cpp"
        "src/system/power_service_linux.cpp")
endif()

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

target_include_directories(${PROJECT_NAME} PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}>")

set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION "${PROJECT_VERSION}")

set_property(TARGET ${PROJECT_NAME} APPEND PROPERTY COMPATIBLE_INTERFACE_STRING "${PROJECT_VERSION}")

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

find_package(CURL REQUIRED)
find_package(Intl REQUIRED)
find_package(maddy CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC CURL::libcurl Intl::Intl maddy::maddy nlohmann_json::nlohmann_json)
if(WIN32)
    target_link_libraries(${PROJECT_NAME} PUBLIC Kernel32 Shell32)
else()
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
    target_link_libraries(${PROJECT_NAME} PUBLIC Threads::Threads)
endif()
if(LINUX)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(glib REQUIRED IMPORTED_TARGET glib-2.0)
    pkg_check_modules(gio REQUIRED IMPORTED_TARGET gio-2.0)
    pkg_check_modules(gmodule REQUIRED IMPORTED_TARGET gmodule-2.0)
    pkg_check_modules(gobject REQUIRED IMPORTED_TARGET gobject-2.0)
    pkg_check_modules(gthread REQUIRED IMPORTED_TARGET gthread-2.0)
    pkg_check_modules(libsecret REQUIRED IMPORTED_TARGET libsecret-1)
    target_link_libraries(${PROJECT_NAME} PUBLIC PkgConfig::glib PkgConfig::gio PkgConfig::gmodule PkgConfig::gobject PkgConfig::gthread PkgConfig::libsecret)
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
install(TARGETS ${PROJECT_NAME} EXPORT "${PROJECT_NAME}Targets"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
install(DIRECTORY "${CMAKE_SOURCE_DIR}/include/" DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}")
install(FILES "${CMAKE_BINARY_DIR}/cmake/${PROJECT_NAME}.pc" DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")
install(EXPORT "${PROJECT_NAME}Targets" FILE "${PROJECT_NAME}Targets.cmake" NAMESPACE ${PROJECT_NAME}:: DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")
install(FILES "${CMAKE_BINARY_DIR}/cmake/${PROJECT_NAME}Config.cmake" "${CMAKE_BINARY_DIR}/cmake/${PROJECT_NAME}ConfigVersion.cmake" DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")
export(EXPORT "${PROJECT_NAME}Targets" FILE "${CMAKE_BINARY_DIR}/cmake/${PROJECT_NAME}Targets.cmake" NAMESPACE ${PROJECT_NAME}::)

if (BUILD_TESTING)
    enable_testing()
    find_package(GTest CONFIG REQUIRED)
    include(GoogleTest)

    add_executable(${PROJECT_NAME}_tests
        "tests/main.cpp"
        "tests/test_app.cpp"
        "tests/test_events.cpp"
        "tests/test_filesystem.cpp"
        "tests/test_helpers.cpp"
        "tests/test_hosting.cpp"
        "tests/test_notifications.cpp"
        "tests/test_secrets.cpp"
        "tests/test_services.cpp"
        "tests/test_system.cpp"
        "tests/test_updates.cpp")

    target_link_libraries(${PROJECT_NAME}_tests PRIVATE ${PROJECT_NAME} GTest::gtest GTest::gmock)

    gtest_discover_tests(${PROJECT_NAME}_tests)
endif()