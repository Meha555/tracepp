find_package(CURL REQUIRED)

add_executable(test_tracer test_tracer.cpp)
target_link_libraries(test_tracer PRIVATE tracepp CURL::libcurl)

add_test(NAME test_tracer COMMAND $<TARGET_FILE:test_tracer>)

add_executable(test_event test_event.cpp)
target_link_libraries(test_event PRIVATE tracepp)
add_executable(simple_integrated_test simple_integrated_test.cpp)
target_link_libraries(simple_integrated_test PRIVATE tracepp)

add_custom_target(phony-tracer-test
    DEPENDS test_tracer test_event simple_integrated_test
)