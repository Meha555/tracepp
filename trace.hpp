#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <list>
#include <stack>
#include <string>

// clang-format off
#ifdef _WIN32
#include <windows.h>
#include <processthreadsapi.h>
#else
#include <pthread.h>
#include <unistd.h>
#endif
// clang-format on

#define TRACE_CONCAT(a, b) a##b
#define TRACE_UNIQUE(a, b) TRACE_CONCAT(a, b)
#ifdef TRACE_DISABLED
#define TRACE_SCOPE(name)
#define TRACE_BEGIN(name, ...)
#define TRACE_END(name, ...)
#define TRACE_INSTANT(name, ...)
#define TRACE_ASYNC_BEGIN(name, id, ...)
#define TRACE_ASYNC_END(name, id, ...)
#define TRACE_FLOW_START(name, id)
#define TRACE_FLOW_END(name, id, ...)
#define TRACE_COUNTER(name, counter_name, value)
#define TRACE_OBJECT_SNAPSHOT(name, id, snapshot)
#define TRACE_DATA(p_str)
#define TRACE_DUMP(filename)
#else
#define TRACE_SCOPE(name) trace::TraceScope TRACE_UNIQUE(__trace_scope_, __LINE__)(name);
#define TRACE_BEGIN(name, ...) trace::Tracer::Instance().begin(name, ##__VA_ARGS__);
#define TRACE_END(name, ...) trace::Tracer::Instance().end(name, ##__VA_ARGS__);
#define TRACE_INSTANT(name, ...) trace::Tracer::Instance().instant(name, ##__VA_ARGS__);
#define TRACE_ASYNC_BEGIN(name, id, ...) trace::Tracer::Instance().asyncBegin(name, id, ##__VA_ARGS__);
#define TRACE_ASYNC_END(name, id, ...) trace::Tracer::Instance().asyncEnd(name, id, ##__VA_ARGS__);
#define TRACE_FLOW_START(name, id) trace::Tracer::Instance().flowStart(name, id);
#define TRACE_FLOW_END(name, id, ...) trace::Tracer::Instance().flowEnd(name, id, ##__VA_ARGS__);
#define TRACE_COUNTER(name, counter_name, value) trace::Tracer::Instance().counter(name, counter_name, value);
#define TRACE_OBJECT_SNAPSHOT(name, id, snapshot) trace::Tracer::Instance().objectSnapshot(name, id, snapshot);
#define TRACE_DATA(p_str) trace::Tracer::Instance().data(p_str);
#define TRACE_DUMP(filename) trace::Tracer::Instance().dump(filename);
#endif

#include "event.hpp"

namespace trace
{

/**
 * @brief 性能追踪器类，用于记录程序执行过程中的事件
 */
class Tracer final
{
public:
    static constexpr size_t kMaxEvents = 1e4; ///< 最大事件数量限制

    ~Tracer()
    {
        delete[] events_;
    }

    /**
     * @brief 获取追踪器单例实例
     * @return Tracer实例的引用
     */
    static Tracer &Instance()
    {
        static Tracer inst;
        return inst;
    }

    /**
     * @brief 记录一个即时事件(Instant Event)
     * @param name 事件名称
     */
    void instant(const char *name, const char *scope = "g")
    {
        setThreadName();
        pushEvent(Event::CreateInstant(name, now_us(), pid(), tid(), scope));
    }

    /**
     * @brief 开始记录一个持续事件(Duration Event)
     * @param name 事件名称
     * @param cname 颜色名称（可选）
     * @note 采用X来表示而没有采用B+E的原因是，方便人工复查生成的json文件
     */
    void beginScope(const char *name, const char *cname = nullptr)
    {
        setThreadName();
        Samples &data = tls();
        Event event = Event::CreateComplete(name, now_us(), 0, pid(), tid());
        if (cname) {
            event.cname = cname;
        }
        data.stack.emplace(event);
    }

    /**
     * @brief 结束当前持续事件的记录
     */
    void endScope()
    {
        Samples &data = tls();
        if (data.stack.empty()) {
            return;
        }
        // 取出最近的一个持续事件，更新其持续时间
        Event e = data.stack.top();
        data.stack.pop();
        e.dur_us = now_us() - e.ts_us;
        pushEvent(e);
    }

    void begin(const char *name, const char *cname = nullptr)
    {
        setThreadName();
        Samples &data = tls();
        Event event = Event::CreateBegin(name, now_us(), pid(), tid());
        if (cname) {
            event.cname = cname;
        }
        pushEvent(event);
    }

    void end(const char *name, const char *cname = nullptr)
    {
        setThreadName();
        Samples &data = tls();
        Event event = Event::CreateEnd(name, now_us(), pid(), tid());
        if (cname) {
            event.cname = cname;
        }
        pushEvent(event);
    }

    /**
     * @brief 记录一个异步事件的开始
     * @param name 事件名称
     * @param id 异步事件ID
     * @param cat 事件分类（可选）
     */
    void asyncBegin(const char *name, const char *id, const char *cat = nullptr)
    {
        setThreadName();
        pushEvent(Event::CreateAsyncStart(name, now_us(), pid(), tid(), id, cat));
    }

    /**
     * @brief 记录一个异步事件的结束
     * @param name 事件名称
     * @param id 异步事件ID
     * @param cat 事件分类（可选）
     */
    void asyncEnd(const char *name, const char *id, const char *cat = nullptr)
    {
        setThreadName();
        pushEvent(Event::CreateAsyncEnd(name, now_us(), pid(), tid(), id, cat));
    }

    /**
     * @brief 记录一个流事件的开始
     * @param name 事件名称
     * @param id 流事件ID
     */
    void flowStart(const char *name, const char *id)
    {
        setThreadName();
        pushEvent(Event::CreateFlowStart(name, now_us(), pid(), tid(), id));
    }

    /**
     * @brief 记录一个流事件的结束
     * @param name 事件名称
     * @param id 流事件ID
     * @param bind_point 绑定点（可选）
     */
    void flowEnd(const char *name, const char *id, const char *bind_point = "e")
    {
        setThreadName();
        pushEvent(Event::CreateFlowEnd(name, now_us(), pid(), tid(), id, bind_point));
    }

    /**
     * @brief 记录一个计数器事件
     * @param name 事件名称
     * @param counter_name 计数器名称
     * @param value 计数值
     */
    void counter(const char *name, const char *counter_name, double value)
    {
        setThreadName();
        pushEvent(Event::CreateCounter(name, now_us(), pid(), tid(), counter_name, value));
    }

    /**
     * @brief 记录一个对象快照事件
     * @param name 事件名称
     * @param id 对象ID
     * @param snapshot 快照内容
     */
    void objectSnapshot(const char *name, const char *id, const char *snapshot)
    {
        setThreadName();
        Event event = Event::Create(name, Event::Phase::ObjectSnapshot, now_us(), pid(), tid());
        event.id = id;
        event.args.push_back({"snapshot", snapshot});
        pushEvent(event);
    }

    /**
     * @brief 将所有追踪事件数据序列化为字符串
     * @param out 输出字符串指针
     */
    void data(std::string *out)
    {
        if (!out) {
            return;
        }
        size_t snapshot = write_index_.load(std::memory_order_acquire);
        bool full = full_.load(std::memory_order_acquire);
        size_t count = full ? kMaxEvents : snapshot;
        size_t start = full ? snapshot % kMaxEvents : 0;

        out->reserve(out->capacity() + count * 512);
        out->append("[\n");
        for (size_t i = 0; i < count; ++i) {
            size_t idx = (start + i) % kMaxEvents;
            out->append("  ");
            out->append(events_[idx].toJSON());
            if (i != count - 1) {
                out->append(",");
            }
            out->append("\n");
        }
        out->append("]");
    }

    /**
     * @brief 将追踪数据导出到文件
     * @param filename 输出文件名
     */
    void dump(const std::string &filename)
    {
        std::ofstream out(filename);
        std::string str;
        data(&str);
        out << str;
    }

private:
    /**
     * @brief 构造函数，初始化事件缓冲区
     */
    explicit Tracer()
        : write_index_(0)
        , events_(new Event[kMaxEvents])
        , full_(false)
    {
    }
    Tracer(const Tracer &) = delete;
    Tracer &operator=(const Tracer &) = delete;

    /**
     * @brief 采样数据
     */
    struct Samples
    {
        std::stack<Event> stack;                ///< 当前跟踪的持续事件栈
        bool thread_metadata_registered = false; ///< 线程元数据是否已注册
    };

    /**
     * @brief 获取线程本地采样数据
     */
    static Samples &tls()
    {
        // thread_local 不能用于类成员变量的声明，只能用于全局或静态局部变量的声明。
        static thread_local Samples data;
        return data;
    }

    /**
     * @brief 设置线程名称元数据
     */
    void setThreadName()
    {
        Samples &data = tls();
        if (data.thread_metadata_registered) {
            return;
        }

        data.thread_metadata_registered = true;

        uint32_t thread_id = tid();
        char name[32] = {0};
#ifdef _WIN32
        // Windows doesn't have a direct equivalent to pthread_getname_np
        // We can use GetCurrentThread to get thread handle, but getting thread name is more complex
        std::snprintf(name, sizeof(name), "Thread-%u", thread_id);
#else
        pthread_getname_np(pthread_self(), name, sizeof(name));
#endif
        std::string thread_name(name);

        Event meta = Event::CreateThreadName(0, pid(), thread_id, thread_name);
        pushEvent(meta);
    }

    /**
     * @brief 获取当前时间戳(微秒)
     * @return 当前时间戳
     */
    uint64_t now_us() const
    {
        auto now = std::chrono::steady_clock::now();
        return std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count();
    }

    /**
     * @brief 获取当前CPU编号
     * @return 当前CPU编号
     */
    uint32_t cpu() const
    {
#ifdef _WIN32
        return static_cast<uint32_t>(GetCurrentProcessorNumber());
#else
        return static_cast<uint32_t>(sched_getcpu());
#endif
    }

    /**
     * @brief 获取当前线程ID
     * @return 当前线程ID
     */
    uint32_t tid() const
    {
#ifdef _WIN32
        return static_cast<uint32_t>(GetCurrentThreadId());
#else
        return gettid();
#endif
    }

    /**
     * @brief 获取当前进程ID
     * @return 当前进程ID
     */
    uint32_t pid() const
    {
#ifdef _WIN32
        static uint32_t cached_pid = static_cast<uint32_t>(GetCurrentProcessId());
#else
        static uint32_t cached_pid = static_cast<uint32_t>(getpid());
#endif
        return cached_pid;
    }

    /**
     * @brief 添加事件到追踪缓冲区
     * @param e 要添加的事件
     */
    void pushEvent(const Event &e)
    {
        size_t idx = write_index_.fetch_add(1, std::memory_order_relaxed);
        if (idx >= kMaxEvents) {
            full_.store(true, std::memory_order_release);
            if (idx % kMaxEvents == 0) {
                tls().thread_metadata_registered = false;
            }
        }
        events_[idx % kMaxEvents] = e;
    }

    std::atomic<size_t> write_index_; ///< 事件写入索引(原子操作)
    Event *events_;                   ///< 事件缓冲区
    std::atomic<bool> full_;          ///< 缓冲区是否已满
};

/**
 * @brief 跟踪作用域类，用于自动记录函数或代码块的执行时间
 * 在构造时开始记录，在析构时结束记录
 */
class TraceScope
{
public:
    /**
     * @brief 构造函数，开始一个跟踪范围
     * @param name 跟踪范围名称
     */
    TraceScope(const char *name)
    {
        Tracer::Instance().beginScope(name);
    }

    /**
     * @brief 析构函数，结束跟踪范围
     */
    ~TraceScope()
    {
        Tracer::Instance().endScope();
    }
};

}
