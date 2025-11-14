cmake_minimum_required(VERSION 4.0)
project(unicode_norm C)

set(CMAKE_C_STANDARD 11)

add_executable(unicode_norm main.c)

add_subdirectory(libs/utf8proc)
target_link_libraries(unicode_norm PRIVATE utf8proc)
