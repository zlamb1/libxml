add_subdirectory(alloc)

add_executable(example01 main.c)
target_link_libraries(example01 PRIVATE ${LIB_NAME})