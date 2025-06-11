add_executable(std_allocator std_allocator.c)
add_executable(std_debug_allocator std_debug_allocator.c)
target_link_libraries(std_allocator PRIVATE ${LIB_NAME})
target_link_libraries(std_debug_allocator PRIVATE ${LIB_NAME})