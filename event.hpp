#pragma once

#include <cmath>
#include <cstdint>
#include <iomanip>
#include <list>
#include <sstream>
#include <string>
#include <unordered_map>

namespace trace
{

class Event
{
public:
    enum class Phase {
        Begin,          // B - 开始事件
        End,            // E - 结束事件
        Instant,        // I - 即时事件
        Complete,       // X - 完整事件（包含持续时间）
        Metadata,       // M - 元数据事件
        Counter,        // C - 计数器事件
        AsyncStart,     // b - 异步事件开始
        AsyncEnd,       // e - 异步事件结束
        AsyncInstant,   // n - 异步即时事件
        FlowStart,      // s - 流事件开始
        FlowEnd,        // f - 流事件结束
        ObjectCreated,  // N - 对象创建
        ObjectSnapshot, // O - 对象快照
        ObjectDeleted,  // D - 对象删除
    };

    /**
     * @brief 键值对结构体，用于存储事件的附加参数
     */
    struct Arg
    {
        enum class Type {
            String,
            Number,
            Boolean,
            Json,
        };

        std::string name;  ///< 参数名称
        std::string value; ///< 参数值
        Type type = Type::String;

        Arg(const std::string &n, const std::string &v)
            : name(n)
            , value(v)
        {
        }

        Arg(const std::string &n, const std::string &v, Type t)
            : name(n)
            , value(v)
            , type(t)
        {
        }

        static Arg Number(const std::string &n, double v)
        {
            if (!std::isfinite(v)) {
                return {n, "0", Type::Number};
            }
            std::ostringstream out;
            out << std::setprecision(15) << v;
            return {n, out.str(), Type::Number};
        }

        static Arg Boolean(const std::string &n, bool v)
        {
            return {n, v ? "true" : "false", Type::Boolean};
        }

        static Arg Json(const std::string &n, const std::string &v)
        {
            return {n, v, Type::Json};
        }
    };

    // 主要参数
    const char *name = nullptr; ///< 事件名称
    Phase phase;                ///< 事件阶段类型
    uint64_t ts_us;             ///< 事件时间戳 (微秒)
    uint64_t dur_us;            ///< 事件持续时间 (微秒，用于Complete事件)
    uint32_t pid;               ///< 进程ID
    uint32_t tid;               ///< 线程ID

    // 可选参数
    const char *cat = nullptr;   ///< 事件分类
    const char *id = nullptr;    ///< 异步事件或流事件的ID
    const char *cname = nullptr; ///< 颜色名称（用于可视化）
    const char *scope = nullptr; ///< 即时事件作用域（g/p/t）
    std::list<Arg> args;         ///< 附加参数列表

    // 流事件特定参数
    const char *bind_point = nullptr; ///< 流事件绑定点 ("s" 或 "e")

    /**
     * @brief 创建基本事件
     */
    static Event Create(const char *name, Phase ph, uint64_t ts, uint32_t pid, uint32_t tid)
    {
        Event e{0};
        e.name = name;
        e.phase = ph;
        e.ts_us = ts;
        e.pid = pid;
        e.tid = tid;
        return e;
    }

    /**
     * @brief 创建开始事件
     */
    static Event CreateBegin(const char *name, uint64_t ts, uint32_t pid, uint32_t tid)
    {
        return Create(name, Phase::Begin, ts, pid, tid);
    }

    /**
     * @brief 创建结束事件
     */
    static Event CreateEnd(const char *name, uint64_t ts, uint32_t pid, uint32_t tid)
    {
        return Create(name, Phase::End, ts, pid, tid);
    }

    /**
     * @brief 创建即时事件
     */
    static Event CreateInstant(const char *name, uint64_t ts, uint32_t pid, uint32_t tid,
                               const char *scope = "g")
    {
        Event e = Create(name, Phase::Instant, ts, pid, tid);
        e.scope = scope; // g=global, p=process, t=thread
        return e;
    }

    /**
     * @brief 创建完整事件（包含持续时间）
     */
    static Event CreateComplete(const char *name, uint64_t ts, uint64_t dur, uint32_t pid, uint32_t tid)
    {
        Event e = Create(name, Phase::Complete, ts, pid, tid);
        e.dur_us = dur;
        return e;
    }

    /**
     * @brief 创建元数据事件
     */
    static Event CreateMetadata(const char *name, uint64_t ts, uint32_t pid, uint32_t tid,
                                const std::string &meta_name, const std::string &meta_value)
    {
        Event e = Create(name, Phase::Metadata, ts, pid, tid);
        e.args.push_back({meta_name, meta_value});
        return e;
    }

    /**
     * @brief 创建进程名称元数据事件
     * @see CreateMetadata
     */
    static Event CreateProcessName(uint64_t ts, uint32_t pid, const std::string &process_name)
    {
        return CreateMetadata("process_name", ts, pid, 0, "name", process_name);
    }

    /**
     * @brief 创建线程名称元数据事件
     * @see CreateMetadata
     */
    static Event CreateThreadName(uint64_t ts, uint32_t pid, uint32_t tid, const std::string &thread_name)
    {
        return CreateMetadata("thread_name", ts, pid, tid, "name", thread_name);
    }

    /**
     * @brief 创建计数器事件
     */
    static Event CreateCounter(const char *name, uint64_t ts, uint32_t pid, uint32_t tid,
                               const std::string &counter_name, double value)
    {
        Event e = Create(name, Phase::Counter, ts, pid, tid);
        e.args.push_back(Arg::Number(counter_name, value));
        return e;
    }

    /**
     * @brief 创建异步开始事件
     */
    static Event CreateAsyncStart(const char *name, uint64_t ts, uint32_t pid,
                                  const char *async_id, const char *cat = nullptr)
    {
        return CreateAsync(name, Phase::AsyncStart, ts, pid, async_id, cat);
    }

    static Event CreateAsyncStart(const char *name, uint64_t ts, uint32_t pid, uint32_t tid,
                                  const char *async_id, const char *cat = nullptr)
    {
        return CreateAsync(name, Phase::AsyncStart, ts, pid, tid, async_id, cat);
    }

    /**
     * @brief 创建异步结束事件
     */
    static Event CreateAsyncEnd(const char *name, uint64_t ts, uint32_t pid,
                                const char *async_id, const char *cat = nullptr)
    {
        return CreateAsync(name, Phase::AsyncEnd, ts, pid, async_id, cat);
    }

    static Event CreateAsyncEnd(const char *name, uint64_t ts, uint32_t pid, uint32_t tid,
                                const char *async_id, const char *cat = nullptr)
    {
        return CreateAsync(name, Phase::AsyncEnd, ts, pid, tid, async_id, cat);
    }

    /**
     * @brief 创建异步即时事件
     */
    static Event CreateAsyncInstant(const char *name, uint64_t ts, uint32_t pid,
                                    const char *async_id, const char *cat = nullptr)
    {
        return CreateAsync(name, Phase::AsyncInstant, ts, pid, async_id, cat);
    }

    static Event CreateAsyncInstant(const char *name, uint64_t ts, uint32_t pid, uint32_t tid,
                                    const char *async_id, const char *cat = nullptr)
    {
        return CreateAsync(name, Phase::AsyncInstant, ts, pid, tid, async_id, cat);
    }

    /**
     * @brief 创建流开始事件
     */
    static Event CreateFlowStart(const char *name, uint64_t ts, uint32_t pid, uint32_t tid,
                                 const char *flow_id)
    {
        return CreateFlow(name, Phase::FlowStart, ts, pid, tid, flow_id);
    }

    /**
     * @brief 创建流结束事件
     */
    static Event CreateFlowEnd(const char *name, uint64_t ts, uint32_t pid, uint32_t tid,
                               const char *flow_id, const char *bind_point = "e")
    {
        Event e = CreateFlow(name, Phase::FlowEnd, ts, pid, tid, flow_id);
        e.bind_point = bind_point;
        return e;
    }

    /**
     * @brief 创建对象事件
     */
    static Event CreateObject(const char *name, Phase ph, uint64_t ts, uint32_t pid,
                              const char *object_id, const char *cat = nullptr)
    {
        Event e = Create(name, ph, ts, pid, 0); // tid=0 for object events
        e.id = object_id;
        e.cat = cat;
        return e;
    }

    /**
     * @brief 将事件转换为JSON格式字符串
     */
    std::string toJSON() const
    {
        std::ostringstream json;
        json << "{";

        json << "\"name\":" << quote(name ? name : "");
        json << ",\"ph\":\"" << phaseToChar(phase) << "\"";
        json << ",\"ts\":" << ts_us;
        json << ",\"pid\":" << pid;
        json << ",\"tid\":" << tid;

        if (cat) {
            json << ",\"cat\":" << quote(cat);
        }
        if (cname) {
            json << ",\"cname\":" << quote(cname);
        }
        if (phase == Phase::Complete) {
            json << ",\"dur\":" << dur_us;
        }
        if (id) {
            json << ",\"id\":" << quote(id);
        }
        if (scope) {
            json << ",\"s\":" << quote(scope);
        }
        if (bind_point) {
            json << ",\"bp\":" << quote(bind_point);
        }

        json << ",\"args\":{";
        bool first = true;
        for (const auto &arg : args) {
            if (!first) {
                json << ",";
            }
            json << quote(arg.name) << ":" << argValueToJSON(arg);
            first = false;
        }
        json << "}";

        json << "}";
        return json.str();
    }

private:
    /**
     * @brief 创建异步事件
     */
    static Event CreateAsync(const char *name, Phase ph, uint64_t ts, uint32_t pid,
                             const char *async_id, const char *cat = nullptr)
    {
        return CreateAsync(name, ph, ts, pid, 0, async_id, cat);
    }

    static Event CreateAsync(const char *name, Phase ph, uint64_t ts, uint32_t pid, uint32_t tid,
                             const char *async_id, const char *cat = nullptr)
    {
        Event e = Create(name, ph, ts, pid, tid);
        e.id = async_id;
        e.cat = cat;
        return e;
    }

    /**
     * @brief 创建流事件
     */
    static Event CreateFlow(const char *name, Phase ph, uint64_t ts, uint32_t pid, uint32_t tid,
                            const char *flow_id, const char *bind_point = nullptr)
    {
        Event e = Create(name, ph, ts, pid, tid);
        e.id = flow_id;
        e.bind_point = bind_point;
        return e;
    }

    /**
     * @brief 将事件阶段转换为字符
     */
    static char phaseToChar(Phase phase)
    {
        static const std::unordered_map<Phase, char> phase_map = {
            {Phase::Begin, 'B'},
            {Phase::End, 'E'},
            {Phase::Instant, 'I'},
            {Phase::Complete, 'X'},
            {Phase::Metadata, 'M'},
            {Phase::Counter, 'C'},
            {Phase::AsyncStart, 'b'},
            {Phase::AsyncEnd, 'e'},
            {Phase::AsyncInstant, 'n'},
            {Phase::FlowStart, 's'},
            {Phase::FlowEnd, 'f'},
            {Phase::ObjectCreated, 'N'},
            {Phase::ObjectSnapshot, 'O'},
            {Phase::ObjectDeleted, 'D'}};

        auto it = phase_map.find(phase);
        return (it != phase_map.end()) ? it->second : '?';
    }

    /**
     * @brief 转义JSON字符串
     */
    static std::string quote(const std::string &str)
    {
        return "\"" + escapeJSON(str) + "\"";
    }

    static std::string argValueToJSON(const Arg &arg)
    {
        switch (arg.type) {
        case Arg::Type::String:
            return quote(arg.value);
        case Arg::Type::Number:
        case Arg::Type::Boolean:
        case Arg::Type::Json:
            return arg.value;
        }
        return quote(arg.value);
    }

    static std::string escapeJSON(const std::string &str)
    {
        std::ostringstream result;
        for (size_t i = 0; i < str.length();) {
            unsigned char c = str[i];

            if (c < 0x80) {
                // ASCII字符
                switch (c) {
                case '"':
                    result << "\\\"";
                    break;
                case '\\':
                    result << "\\\\";
                    break;
                case '\b':
                    result << "\\b";
                    break;
                case '\f':
                    result << "\\f";
                    break;
                case '\n':
                    result << "\\n";
                    break;
                case '\r':
                    result << "\\r";
                    break;
                case '\t':
                    result << "\\t";
                    break;
                default:
                    if (c >= 0x20) {
                        result << c;
                    } else {
                        result << "\\u" << std::hex << std::setw(4) << std::setfill('0') << static_cast<int>(c);
                    }
                }
                i++;
            } else {
                // UTF-8字符，直接输出（Chrome Trace Viewer支持UTF-8）
                result << c;
                i++;

                // 继续读取UTF-8字符的其他字节
                if ((c & 0xE0) == 0xC0) {
                    // 2字节UTF-8
                    if (i < str.length())
                        result << str[i++];
                } else if ((c & 0xF0) == 0xE0) {
                    // 3字节UTF-8
                    if (i < str.length())
                        result << str[i++];
                    if (i < str.length())
                        result << str[i++];
                } else if ((c & 0xF8) == 0xF0) {
                    // 4字节UTF-8
                    if (i < str.length())
                        result << str[i++];
                    if (i < str.length())
                        result << str[i++];
                    if (i < str.length())
                        result << str[i++];
                }
            }
        }
        return result.str();
    }
};

}
