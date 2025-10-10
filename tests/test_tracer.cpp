#include "trace.hpp"
#include <chrono>
#include <cmath>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <curl/curl.h>

using namespace std::chrono_literals;

// 模拟一个计算密集型任务
void compute_task(int iterations) {
    TRACE_SCOPE("compute_task");

    double result = 0.0;
    for (int i = 0; i < iterations; ++i) {
        TRACE_SCOPE("compute_iteration");

        // 一些计算工作
        for (int j = 0; j < 1000000; ++j) {
            result += sin(j * 0.0001) * cos(j * 0.0001);
        }

        // 记录一个即时事件
        if (i % 5 == 0) {
            TRACE_INSTANT("iteration_milestone");
        }

        // 短暂休眠模拟 I/O 等待
        if (i % 3 == 0) {
            std::this_thread::sleep_for(10ms);
        }
    }
}

// 模拟一个 I/O 任务
void io_task(int operations) {
    TRACE_SCOPE("io_task");

    for (int i = 0; i < operations; ++i) {
        TRACE_SCOPE("io_operation");

        // 模拟 I/O 操作的延迟
        std::this_thread::sleep_for(50ms);

        // 模拟处理 I/O 结果的一些计算
        double result = 0.0;
        for (int j = 0; j < 100000; ++j) {
            result += sin(j * 0.001);
        }
    }
}

struct memory_t {
    char *response;
    size_t size;
};

static size_t write_cb(char *data, size_t size, size_t nmemb, void *userdata)
{
    size_t realsize = size * nmemb;
    struct memory_t *mem = (struct memory_t *)userdata;

    char *ptr = (char *)realloc(mem->response, mem->size + realsize + 1);
    if(!ptr)
        return 0;  /* out of memory */

    mem->response = ptr;
    memcpy(&(mem->response[mem->size]), data, realsize);
    mem->size += realsize;
    mem->response[mem->size] = 0;

    return realsize; // 这个返回值是给libcurl看的，用于检查是否有错误。
}

// 模拟一个网络请求任务
void network_request_task(int operations) {
    TRACE_SCOPE("network_request_task");
    struct memory_t chunk = {0};

    CURL *curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&chunk);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
    for (int i = 0; i < operations; ++i) {
        curl_easy_setopt(curl, CURLOPT_URL, "https://www.baidu.com");
        CURLcode ret = curl_easy_perform(curl);
    }
    curl_easy_cleanup(curl);
    free(chunk.response);
}


// 多线程工作函数
void worker_thread(int id, int compute_iterations, int io_operations, int network_operations) {
    TRACE_SCOPE("worker_thread");

    #ifndef _WIN32
    char thread_name[32];
    std::snprintf(thread_name, sizeof(thread_name), "Worker-%d", id);
    pthread_setname_np(pthread_self(), thread_name);
    #endif

    compute_task(compute_iterations);
    io_task(io_operations);
    network_request_task(network_operations);
}

int main() {
    TRACE_SCOPE("main");

    std::cout << "Tracer example program starting..." << std::endl;

    // 创建多个工作线程
    std::vector<std::thread> threads;
    const int num_threads = 4;

    for (int i = 0; i < num_threads; ++i) {
        // 每个线程执行不同工作量的任务
        int compute_iterations = 10 + i;
        int io_operations = 5 + (i % 3);
        int network_operations = 3 + (i % 2);

        threads.emplace_back(worker_thread, i, compute_iterations, io_operations, network_operations);
        std::this_thread::sleep_for(100ms);
    }

    // 主线程也做一些工作
    {
        TRACE_SCOPE("main_processing");
        std::cout << "Main thread processing..." << std::endl;
        std::this_thread::sleep_for(500ms);
        TRACE_INSTANT("main_work_checkpoint");
    }

    for (auto & t: threads) {
        t.join();
    }

    // 保存跟踪结果到文件
    std::cout <<
        "\nAll threads completed.\nSaving trace to 'trace_result.json'..." <<
        std::endl;
    TRACE_DUMP("trace_result.json");

    std::cout << "Example completed. Open 'trace_result.json' in "
    "chrome://tracing to view results." <<
    std::endl;
    return 0;
}