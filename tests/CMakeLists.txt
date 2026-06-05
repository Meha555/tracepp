add_executable(format_test format_test.cpp)
target_link_libraries(format_test PRIVATE tracepp)
add_test(NAME format_test COMMAND $<TARGET_FILE:format_test>)