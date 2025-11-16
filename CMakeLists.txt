cmake_minimum_required(VERSION 4.0)
project(unicode_norm
        LANGUAGES C
        VERSION 0.0.1)
configure_file(version.h.in version.h @ONLY)

set(CMAKE_C_STANDARD 11)

add_executable(unicode_norm unicode_norm.c)

add_subdirectory(libs/utf8proc)
target_include_directories(unicode_norm PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(unicode_norm PRIVATE utf8proc)
