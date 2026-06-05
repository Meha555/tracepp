#include "event.hpp"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace trace;

/**
 * @brief 生成符合Chrome Trace Viewer格式的示例数据
 * 参考: https://blog.csdn.net/u011331731/article/details/108354605
 */
void generate_simple_trace_example()
{
    std::vector<Event> events;
    uint64_t base_ts = 0;
    uint32_t pid = 1234; // Main process

    // 1. 设置进程名称元数据
    events.push_back(Event::CreateProcessName(base_ts, pid, "MyApplication"));

    // 2. 创建异步事件 "DailyTask"
    events.push_back(Event::CreateAsyncStart("DailyTask", base_ts, pid, "daily-task-1", "daily"));

    // 3. 工作线程事件
    uint32_t work_tid = 1001;
    events.push_back(Event::CreateThreadName(base_ts, pid, work_tid, "WorkerThread"));

    // Planning - 完整事件 (28.8秒)
    auto planning = Event::CreateComplete("Planning", base_ts, 28800000000, pid, work_tid);
    planning.cname = "good"; // 绿色
    events.push_back(planning);

    // Coding - 完整事件 (3.6秒)
    auto coding = Event::CreateComplete("Coding", base_ts + 32400000000, 3600000000, pid, work_tid);
    events.push_back(coding);

    // 4. 休闲线程事件
    uint32_t leisure_tid = 1002;
    events.push_back(Event::CreateThreadName(base_ts, pid, leisure_tid, "LeisureThread"));

    // Watching movie - 开始和结束事件
    events.push_back(Event::Create("WatchMovie", Event::Phase::DurationBegin, base_ts + 28800000000, pid, leisure_tid));
    events.push_back(Event::Create("WatchMovie", Event::Phase::DurationEnd, base_ts + 32400000000, pid, leisure_tid));

    // Walking dog - 完整事件 (1.8秒)
    auto walking = Event::CreateComplete("WalkDog", base_ts + 36000000000, 1800000000, pid, leisure_tid);
    events.push_back(walking);

    // 5. 即时事件 - Got an idea
    auto idea = Event::CreateInstant("GotIdea", base_ts + 18800000000, pid, work_tid, Event::Scope::Global);
    events.push_back(idea);

    // 6. 流事件 - connect
    events.push_back(Event::CreateFlowStart("Connect", base_ts + 28800000000, pid, work_tid, "connection-1"));
    events.push_back(Event::CreateFlowEnd("Connect", base_ts + 32400000000, pid, work_tid, "connection-1", "e"));

    // 7. 对象快照事件 - Reflection
    auto reflection = Event::Create("Reflection", Event::Phase::ObjectSnapshot, base_ts + 37800000000, pid, 1003);
    reflection.id = "ref-1";
    reflection.args.push_back({"snapshot", "Productive day"});
    events.push_back(reflection);

    // 8. 结束异步事件 "DailyTask"
    events.push_back(Event::CreateAsyncEnd("DailyTask", base_ts + 37800000000, pid, "daily-task-1", "daily"));

    // 9. 计数器事件 - Memory usage
    events.push_back(Event::CreateCounter("MemoryUsage", base_ts + 10000000, pid, work_tid, "heap_size", 256.5));
    events.push_back(Event::CreateCounter("MemoryUsage", base_ts + 20000000, pid, work_tid, "heap_size", 512.0));
    events.push_back(Event::CreateCounter("MemoryUsage", base_ts + 30000000, pid, work_tid, "heap_size", 768.25));

    // 输出到文件
    std::ofstream out("simple_trace.json");
    out << "[" << std::endl;
    for (size_t i = 0; i < events.size(); ++i) {
        out << "  " << events[i].toJSON();
        if (i < events.size() - 1)
            out << ",";
        out << std::endl;
    }
    out << "]" << std::endl;
    out.close();

    std::cout << "Simple trace example generated: simple_trace.json" << std::endl;
    std::cout << "You can open it in chrome://tracing to view the visualization" << std::endl;
}

/**
 * @brief 生成性能分析示例
 */
void generate_performance_analysis()
{
    std::vector<Event> events;
    uint32_t pid = 5678;
    uint64_t base_ts = 1000000;

    // 设置进程和线程信息
    events.push_back(Event::CreateProcessName(base_ts, pid, "PerformanceApp"));
    events.push_back(Event::CreateThreadName(base_ts, pid, 2001, "MainThread"));
    events.push_back(Event::CreateThreadName(base_ts, pid, 2002, "WorkerThread"));
    events.push_back(Event::CreateThreadName(base_ts, pid, 2003, "DatabaseThread"));

    // Main thread events
    auto main_scope = Event::CreateComplete("MainFunction", base_ts, 10000000, pid, 2001);
    main_scope.cname = "good";
    events.push_back(main_scope);

    // Database query - async event
    events.push_back(Event::CreateAsyncStart("DatabaseQuery", base_ts + 1000000, pid, "db-query-1", "database"));

    // Worker thread processing
    auto worker_task = Event::CreateComplete("ProcessData", base_ts + 1500000, 5000000, pid, 2002);
    worker_task.args.push_back({"data_size", "1024KB"});
    worker_task.args.push_back({"algorithm", "quick_sort"});
    events.push_back(worker_task);

    // Database thread events
    auto db_task = Event::CreateComplete("ExecuteQuery", base_ts + 2000000, 3000000, pid, 2003);
    db_task.args.push_back({"query_type", "SELECT"});
    db_task.args.push_back({"table", "users"});
    events.push_back(db_task);

    // Memory usage counters
    events.push_back(Event::CreateCounter("MemoryUsage", base_ts + 2000000, pid, 2002, "heap_size", 256.5));
    events.push_back(Event::CreateCounter("MemoryUsage", base_ts + 4000000, pid, 2002, "heap_size", 512.0));
    events.push_back(Event::CreateCounter("MemoryUsage", base_ts + 6000000, pid, 2002, "heap_size", 768.25));
    events.push_back(Event::CreateCounter("MemoryUsage", base_ts + 8000000, pid, 2002, "heap_size", 1024.0));

    // CPU usage counters
    events.push_back(Event::CreateCounter("CPUUsage", base_ts + 2000000, pid, 2002, "cpu_percent", 45.2));
    events.push_back(Event::CreateCounter("CPUUsage", base_ts + 4000000, pid, 2002, "cpu_percent", 78.5));
    events.push_back(Event::CreateCounter("CPUUsage", base_ts + 6000000, pid, 2002, "cpu_percent", 92.1));
    events.push_back(Event::CreateCounter("CPUUsage", base_ts + 8000000, pid, 2002, "cpu_percent", 23.7));

    // Database query end
    events.push_back(Event::CreateAsyncEnd("DatabaseQuery", base_ts + 8500000, pid, "db-query-1", "database"));

    // Flow events - Data pipeline
    events.push_back(Event::CreateFlowStart("DataPipeline", base_ts + 1500000, pid, 2002, "pipeline-1"));
    events.push_back(Event::CreateFlowEnd("DataPipeline", base_ts + 9500000, pid, 2001, "pipeline-1"));

    // Instant events - Important moments
    events.push_back(Event::CreateInstant("CacheHit", base_ts + 3000000, pid, 2002, Event::Scope::Process));
    events.push_back(Event::CreateInstant("CacheMiss", base_ts + 7000000, pid, 2002, Event::Scope::Thread));

    // Output to file
    std::ofstream out("performance_analysis.json");
    out << "[" << std::endl;
    for (size_t i = 0; i < events.size(); ++i) {
        out << "  " << events[i].toJSON();
        if (i < events.size() - 1)
            out << ",";
        out << std::endl;
    }
    out << "]" << std::endl;
    out.close();

    std::cout << "Performance analysis example generated: performance_analysis.json" << std::endl;
}

int main()
{
    std::cout << "=== Chrome Trace Viewer Event System Demo ===" << std::endl;
    std::cout << "Based on: https://blog.csdn.net/u011331731/article/details/108354605" << std::endl;
    std::cout << std::endl;

    // Generate simple trace example
    generate_simple_trace_example();
    std::cout << std::endl;

    // Generate performance analysis
    generate_performance_analysis();
    std::cout << std::endl;

    std::cout << "Demo completed successfully!" << std::endl;
    std::cout << "Usage instructions:" << std::endl;
    std::cout << "1. Open Chrome browser" << std::endl;
    std::cout << "2. Type chrome://tracing in address bar" << std::endl;
    std::cout << "3. Click Load button and select the generated JSON files" << std::endl;
    std::cout << "4. View the visualization results" << std::endl;
    std::cout << std::endl;
    std::cout << "Generated files:" << std::endl;
    std::cout << "- simple_trace.json (Basic timeline example)" << std::endl;
    std::cout << "- performance_analysis.json (Performance monitoring example)" << std::endl;

    return 0;
}