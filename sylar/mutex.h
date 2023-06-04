#ifndef __SYLAR_MUTEX_H__
#define __SYLAR_MUTEX_H__

#include <semaphore.h>
#include <atomic>
#include<pthread.h>
#include "noncopyable.h"
namespace sylar {

/// 信号量封装, RAII思想
class Semaphore : Noncopyable {
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
class Mutex : Noncopyable {
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
class RWMutex : Noncopyable {
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
class Spinlock : Noncopyable {
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

}
#endif
