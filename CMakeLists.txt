cmake_minimum_required(VERSION 3.28.3)

project(pix VERSION 1.0.0 LANGUAGES C CXX)

set(LIB_NAME "xmlparser")
set(EXE_NAME "main")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(${LIB_NAME}
    src/xml_parse.c
    src/xml_parser.c 
    src/xml_string.c
    src/xml_unicode.c
)

target_compile_features(${LIB_NAME} PRIVATE c_std_11)
target_include_directories(${LIB_NAME} PUBLIC include)

target_compile_options(${LIB_NAME} PRIVATE $ENV{LIB_COMPILE_OPTIONS})

add_executable(${EXE_NAME} examples/main.c)
target_link_libraries(${EXE_NAME} PRIVATE ${LIB_NAME})

add_executable(utf8_test examples/utf8_test.c)
target_link_libraries(utf8_test PRIVATE ${LIB_NAME})