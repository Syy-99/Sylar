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
#include <semaphore.h>
#include <atomic>

namespace sylar {

/// 信号量封装, RAII思想
class Semaphore {
public:
    Semaphore(uint32_t count = 0);
    ~Semaphore();

    /// 获取信号量
    void wait();
    /// 释放信号量
    void notify();

private:

    Semaphore(const Semaphore&) = delete;
    Semaphore(const Semaphore&&) = delete;
    Semaphore& operator=(const Semaphore&) = delete;

private:

    sem_t m_semaphore;
};

/// !!!通过模版实现范围锁：指用类的构造函数来加锁，用析造函数来释放锁。这种方式可以简化锁的操作，也可以避免忘记解锁导致的死锁问题
/// 局部锁的模板实现
template<class T>
struct ScopedLockImpl {
public:

    ScopedLockImpl(T& mutex) : m_mutex(mutex) {
        m_mutex.lock();
        m_locked = true;
    }

    ~ScopedLockImpl() {
        unlock();
    }

    void lock() {
        if (!m_locked) {
            m_mutex.lock();
            m_locked = true; 
        }
    }

    void unlock() {
        if (m_locked) {
            m_mutex.unlock();
            m_locked = false;
        }
    }

private:
    /// mutex
    T& m_mutex;

    /// 是否已上锁
    bool m_locked;
};


/// 局部读锁模板实现
template<class T>
struct ReadScopedLockImpl {
public:

    ReadScopedLockImpl(T& mutex) : m_mutex(mutex) {
        m_mutex.rdlock();
        m_locked = true;
    }

    ~ReadScopedLockImpl() {
        unlock();
    }

    void lock() {
        if (!m_locked) {
            m_mutex.rdlock();
            m_locked = true; 
        }
    }

    void unlock() {
        if (m_locked) {
            m_mutex.unlock();
            m_locked = false;
        }
    }

private:
    /// mutex
    T& m_mutex;

    /// 是否已上锁
    bool m_locked;
};

/// 为什么要这些模版??? -> 感觉像是统一调用的接口，lock(), unlock();
/// 局部写锁模版实现
template<class T>
struct WriteScopedLockImpl {
public:

    WriteScopedLockImpl(T& mutex) : m_mutex(mutex) {
        m_mutex.wrlock();
        m_locked = true;
    }

    ~WriteScopedLockImpl() {
        unlock();
    }

    void lock() {
        if (!m_locked) {
            m_mutex.wrlock();
            m_locked = true; 
        }
    }

    void unlock() {
        if (m_locked) {
            m_mutex.unlock();
            m_locked = false;
        }
    }

private:
    /// mutex
    T& m_mutex;

    /// 是否已上锁
    bool m_locked;
};


/**
 * @brief 空锁(用于调试)
 */
class NullMutex {
public:
    /// 局部锁
    typedef ScopedLockImpl<NullMutex> Lock;

    /**
     * @brief 构造函数
     */
    NullMutex() {}

    /**
     * @brief 析构函数
     */
    ~NullMutex() {}

    /**
     * @brief 加锁
     */
    void lock() {}

    /**
     * @brief 解锁
     */
    void unlock() {}
};

/// 互斥锁
class Mutex {
public:

    /// 局部锁: 利用局部锁对象，确保锁一定被释放
    /// 问题: 锁的力度很大
    typedef ScopedLockImpl<Mutex> Lock;

    Mutex() {
        pthread_mutex_init(&m_mutex, nullptr);
    }

    ~Mutex() {
        pthread_mutex_destroy(&m_mutex);
    }

    void lock() {
        pthread_mutex_lock(&m_mutex);
    }

    void unlock() {
        pthread_mutex_unlock(&m_mutex);
    }

private:

    pthread_mutex_t m_mutex;
};


/**
 * @brief 空读写锁(用于调试)
 */
class NullRWMutex {
public:
    /// 局部读锁
    typedef ReadScopedLockImpl<NullMutex> ReadLock;
    /// 局部写锁
    typedef WriteScopedLockImpl<NullMutex> WriteLock;

    /**
     * @brief 构造函数
     */
    NullRWMutex() {}
    /**
     * @brief 析构函数
     */
    ~NullRWMutex() {}

    /**
     * @brief 上读锁
     */
    void rdlock() {}

    /**
     * @brief 上写锁
     */
    void wrlock() {}
    /**
     * @brief 解锁
     */
    void unlock() {}
};

/// 读写锁
class RWMutex {
public:

    /// 局部读锁
    typedef ReadScopedLockImpl<RWMutex> ReadLock;

    /// 局部写锁
    typedef WriteScopedLockImpl<RWMutex> WriteLock;

    RWMutex() {
        pthread_rwlock_init(&m_lock, nullptr);
    }

    ~RWMutex() {
        pthread_rwlock_destroy(&m_lock);
    }

    /// 上读锁
    void rdlock() {
        pthread_rwlock_rdlock(&m_lock);
    }
    /// 上写锁
    void wrlock() {
        pthread_rwlock_wrlock(&m_lock);
    }

    void unlock() {
        pthread_rwlock_unlock(&m_lock);
    }
private:

    pthread_rwlock_t m_lock;
};


/**
 * @brief 自旋锁
 */
class Spinlock {
public:
    /// 局部锁
    typedef ScopedLockImpl<Spinlock> Lock;

    /**
     * @brief 构造函数
     */
    Spinlock() {
        pthread_spin_init(&m_mutex, 0);
    }

    /**
     * @brief 析构函数
     */
    ~Spinlock() {
        pthread_spin_destroy(&m_mutex);
    }

    /**
     * @brief 上锁
     */
    void lock() {
        pthread_spin_lock(&m_mutex);
    }

    /**
     * @brief 解锁
     */
    void unlock() {
        pthread_spin_unlock(&m_mutex);
    }
private:
    /// 自旋锁
    pthread_spinlock_t m_mutex;
};


class Thread {
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