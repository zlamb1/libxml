cmake_minimum_required(VERSION 3.28.3)

project(xml VERSION 1.0.0 LANGUAGES C)

set(LIB_NAME "xml")
set(EXE_NAME "main")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(${LIB_NAME}
    src/libxml.c
    src/xmlalloc.c
    src/xmlencoding.c
    src/xmlparse.c
    src/xmlparsecmd.c
    src/xmlparser.c
)

set_target_properties(${LIB_NAME} PROPERTIES C_STANDARD 11)
target_include_directories(${LIB_NAME} PUBLIC include)

target_compile_options(${LIB_NAME} PRIVATE $ENV{LIB_COMPILE_OPTIONS})

add_subdirectory(tests)