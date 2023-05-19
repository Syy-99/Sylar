#include "thread.h"
#include "log.h"
#include "util.h"

namespace sylar {

    Semaphore::Semaphore(uint32_t count) {
        if(sem_init(&m_semaphore, 0, count)) {
            throw std::logic_error("sem_init error");
        }
    }

    Semaphore::~Semaphore() {
        sem_destroy(&m_semaphore);
    }

    void Semaphore::wait() {
        if(sem_wait(&m_semaphore)) {
            throw std::logic_error("sem_wait error");
        }
    }
    void Semaphore::notify() {
        if(sem_post(&m_semaphore)) {
            throw std::logic_error("sem_post error");
        }
    }

    // thread_local定义成线程局部变量,在run中设置
    static thread_local Thread* t_thread = nullptr;
    static thread_local std::string t_thread_name = "UNKNOW";

    /// 系统库使用"system"日志
    static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");  


    /// 获得当前线程的指针
    Thread* Thread::GetThis() {
        // 因为是静态成员函数，无法访问数据成员
        return t_thread;        
    }
    /// 获取当前线程的名称，日志用
    const std::string Thread::GetName() {
         return t_thread_name;
    }


    void Thread::SetName(const std::string& name) {
        if (t_thread) {
            t_thread->m_name = name;
        }

        t_thread_name = name;
    }


    void* Thread::run(void* arg) {
        Thread* thread =(Thread*)arg;   // 这里arg就是this指针，所以可以直接获得数据成员
        t_thread = thread;
        thread->m_id = sylar::GetThreadId();
        // pthread_setname_np: 给线程命名，实际top命令中也会显示
        // pthread_self: 获取当前线程ID
        pthread_setname_np(pthread_self(), thread->m_name.substr(0, 15).c_str());
        t_thread_name = thread->m_name;

        thread->m_semaphore.notify();       // 初始化完成，唤醒线程

        std::function<void()> cb;
        cb.swap(thread->m_cb);     // 当函数内有智能指针时，可以释放内部的引用 ?
        cb();
        return 0;
     }

    Thread::Thread(std::function<void()> cb, const std::string& name) :m_cb(cb) ,m_name(name) {
        if (name.empty()) {
            m_name = "UNKNOW";
        }

        int rt = pthread_create(&m_thread, nullptr, &Thread::run, this);
        if(rt) {
            SYLAR_LOG_ERROR(g_logger) << "pthread_create thread fail, rt=" << rt
                                      << " name=" << name;
             throw std::logic_error("pthread_create error");
        }


        // 确保线程已经开始执行，才结束构造函数
        m_semaphore.wait();
    }

    Thread::~Thread() {
        if(m_thread) {
            pthread_detach(m_thread);       // 子线程结束同时子线程的资源自动回收，不阻塞父线程
        }
    }
    

    /// 等待线程执行完成
    void Thread::join() {
        if(m_thread) {
            int rt = pthread_join(m_thread, nullptr);
            if(rt) {
                SYLAR_LOG_ERROR(g_logger) << "pthread_join thread fail, rt=" << rt
                << " name=" << m_name;
                throw std::logic_error("pthread_join error");
            }
        }
        m_thread = 0;
    }


}   // sylar