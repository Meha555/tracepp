#include "trace.hpp"
#include <chrono>
#include <iostream>
#include <thread>

using namespace trace;

int main()
{
    try {
        std::cout << "=== Simple Integrated Test ===" << std::endl;

        // 测试基本功能
        {
            TRACE_SCOPE("Scope");
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        TRACE_BEGIN("MainFunction");
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        TRACE_END("MainFunction");

        // 测试即时事件
        TRACE_INSTANT("StartEvent");

        // 测试异步事件
        TRACE_ASYNC_BEGIN("BackgroundTask", "task-1", "background");
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        TRACE_ASYNC_END("BackgroundTask", "task-1", "background");

        // 测试计数器
        for (int i = 0; i < 3; ++i) {
            TRACE_COUNTER("MemoryUsage", "heap_size", 1024.0 + 1000 * i);
            TRACE_COUNTER("CPUUsage", "cpu_percent", 45.5 + 10.0 * i);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        // 测试流事件
        TRACE_FLOW_START("DataPipeline", "pipeline-1");
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        TRACE_FLOW_END("DataPipeline", "pipeline-1");

        // 测试对象快照
        TRACE_OBJECT_SNAPSHOT("SystemState", "state-1", "System initialized");

        // 导出数据
        TRACE_DUMP("simple_integrated.json");

        std::cout << "Test completed successfully!" << std::endl;
        std::cout << "Output: simple_integrated.json" << std::endl;

    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}