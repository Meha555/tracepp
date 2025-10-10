# tracepp

导出 [Google Chrome Tracing Format](https://docs.google.com/document/d/1CvAClvFfyA5R-PhYUmn5OOQtYMH4h6I0nSsKchNAySU) (JSON Array Format) 的JSON文件的埋点性能分析库。

## 快速开始

1. 使用提供的宏跟踪代码执行：

   ```cpp
   #include "trace.hpp"
   
   void example_function() {
       // 自动作用域跟踪
       TRACE_SCOPE("example_function");
       
       // 手动开始/结束跟踪
       TRACE_BEGIN("manual_scope");
       // ... 代码执行
       TRACE_END("manual_scope");
       
       // 即时事件
       TRACE_INSTANT("important_point", "g");
       
       // 异步事件
       TRACE_ASYNC_BEGIN("background_task", "task-1", "background");
       // ... 异步操作
       TRACE_ASYNC_END("background_task", "task-1", "background");
       
       // 流事件
       TRACE_FLOW_START("data_pipeline", "pipe-1");
       // ... 数据处理
       TRACE_FLOW_END("data_pipeline", "pipe-1");
       
       // 计数器事件
       TRACE_COUNTER("MemoryUsage", "heap_size", 1024.0);
       TRACE_COUNTER("CPUUsage", "cpu_percent", 45.5);
       
       // 对象快照
       TRACE_OBJECT_SNAPSHOT("SystemState", "state-1", "System initialized");
   }
   
   int main() {
       example_function();
       
       // 导出跟踪数据
       TRACE_DUMP("trace.json");
       
       return 0;
   }
   ```

3. 在 Chrome 浏览器中查看结果：

   - 打开 Chrome
   - 访问 `chrome://tracing`
   - 点击 "Load" 按钮加载生成的 JSON 文件

## API 文档

### 宏定义

- `TRACE_SCOPE(name)`：创建一个作用域事件，自动记录进入和退出时间
- `TRACE_BEGIN(name, ...)`：开始记录一个作用域事件，需配合 `TRACE_END()` 使用，支持可选参数
- `TRACE_END(name, ...)`：结束记录当前作用域事件，与 `TRACE_BEGIN()` 配对使用，支持可选参数
- `TRACE_INSTANT(name, ...)`：立刻记录一个事件戳，用于标记重要时间点，支持可选参数
- `TRACE_ASYNC_BEGIN(name, id, ...)`：开始记录一个异步事件，需配合 `TRACE_ASYNC_END()` 使用，支持可选参数
- `TRACE_ASYNC_END(name, id, ...)`：结束记录当前异步事件，与 `TRACE_ASYNC_BEGIN()` 配对使用，支持可选参数
- `TRACE_FLOW_START(name, id)`：开始记录一个流程事件，需配合 `TRACE_FLOW_END()` 使用
- `TRACE_FLOW_END(name, id, ...)`：结束记录当前流程事件，与 `TRACE_FLOW_START()` 配对使用，支持可选参数
- `TRACE_COUNTER(name, counter_name, value)`：记录一个计数器事件，用于跟踪数值变化
- `TRACE_OBJECT_SNAPSHOT(name, id, snapshot)`：记录一个对象快照事件，用于记录对象状态
- `TRACE_DATA(p_str)`：获取已收集的跟踪数据并保存到指定字符串指针
- `TRACE_DUMP(filename)`：将跟踪数据保存到文件

### 禁用跟踪

定义 `TRACE_DISABLED` 宏可以在编译时禁用所有跟踪代码：

```cpp
#define TRACE_DISABLED
#include "tracer.hpp"
```

## 性能考虑

- Tracer 使用环形缓冲区存储事件，默认容量为 10,000 个事件
- 达到容量上限后，新事件会覆盖最旧的事件

## 参考

- [Google Chrome Tracing Format](https://docs.google.com/document/d/1CvAClvFfyA5R-PhYUmn5OOQtYMH4h6I0nSsKchNAySU)
- https://blog.csdn.net/u011331731/article/details/108354605