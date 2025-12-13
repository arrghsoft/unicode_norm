cmake_minimum_required(VERSION 4.0)
project(unicode_norm
        LANGUAGES C
        VERSION 1.0.1)

option(UNICODE_NORM_LIBRARY_STATIC_LINK "Build unicode_norm with statically linked libraries" ON)

configure_file(version.h.in version.h @ONLY)

set(CMAKE_C_STANDARD 11)

if (WIN32)
    add_executable(unicode_norm unicode_norm.c windows/app.rc)
else ()
    add_executable(unicode_norm unicode_norm.c)
endif ()

if (UNICODE_NORM_LIBRARY_STATIC_LINK)
    message(STATUS "Building utf8proc with STATIC libraries")
    set(BUILD_SHARED_LIBS OFF)
    add_subdirectory(libs/utf8proc)
else()
    message(STATUS "Building utf8proc with DYNAMIC libraries")
    set(BUILD_SHARED_LIBS ON)
endif()


target_include_directories(unicode_norm PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(unicode_norm PRIVATE utf8proc)

install(TARGETS unicode_norm
    RUNTIME DESTINATION bin)
