cmake_minimum_required(VERSION 4.0)
project(unicode_norm C)

set(CMAKE_C_STANDARD 11)

add_executable(unicode_norm main.c)
target_link_libraries(unicode_norm ${CMAKE_SOURCE_DIR}/libutf8proc.a)
