find_package(CURL REQUIRED)

add_executable(exp_tracer exp_tracer.cpp)
target_link_libraries(exp_tracer PRIVATE tracepp CURL::libcurl)

add_executable(exp_event exp_event.cpp)
target_link_libraries(exp_event PRIVATE tracepp)
add_executable(simple_integrated simple_integrated.cpp)
target_link_libraries(simple_integrated PRIVATE tracepp)

add_custom_target(phony-tracer-exps
    DEPENDS exp_tracer exp_event simple_integrated
)