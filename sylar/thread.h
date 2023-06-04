/**
 * @file thread.h
 * @brief 线程相关的封装

 */


#ifndef __SYLAR_THREAD_H__
#define __SYLAR_THREAD_H__

// C++11之前: pthread_xxx
// C++11: std::thread, 实际使用pthread

#include<thread>
#include<functional>
#include<memory>
#include<string>
#include<pthread.h>

#include "mutex.h"
#include "noncopyable.h"

namespace sylar {

class Thread : Noncopyable{
public:
    typedef std::shared_ptr<Thread> ptr;

    Thread(std::function<void()> cb, const std::string& name);
    ~Thread();


    pid_t getId() const { return m_id; }
    const std::string getName() const { return m_name; }

    /// 等待线程执行完成
    void join();

    /// 获得当前线程的指针
    static Thread* GetThis();
    /// 获取当前线程的名称，日志用
    static const std::string GetName();

    static void SetName(const std::string& name);

    /// 线程执行函数
    static void* run(void* arg);
private:
    // 互斥量和信号量都是不能拷贝的，所以删除拷贝方法（但是可以拷贝指向互斥量的指针）
    Thread(const Thread&) = delete;
    Thread(const Thread&&) = delete;
    Thread& operator=(const Thread&) = delete;
private:
    pid_t m_id = -1;     // 线程id
    pthread_t m_thread = 0; // 线程结构
    std::function<void()>  m_cb;    // 线程执行函数

    std::string m_name;  // 线程名称

    Semaphore m_semaphore;  // 信号量

};


} // sylar


#endif 