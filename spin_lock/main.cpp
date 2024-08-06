#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

int g_counter;

class SpinLock
{
  public:
    SpinLock() = default;

    void lock()
    {
        // std::memory_order_relaxed seems right
        // Is memory order constraint necessary?
        while (m_flag.test_and_set(std::memory_order_acquire)) {}
    }

    void unlock()
    {
        // set flag to false
        m_flag.clear(std::memory_order_release);
    }

  private:
    std::atomic_flag m_flag {ATOMIC_FLAG_INIT};
};

// spin lock that use stad::atomic<bool> without TAS instruction.
// actually this is wrong.
// atomic value with memory order can not implement spin lock in following way.
// read then write are two operations, they could be interleaved by another thread.
// that's why test and set atomic instrunction be used, read and write in single undivisiable instruction.
class SpinLockWrong
{
  public:
    SpinLockWrong() = default;

    void lock()
    {
        while (m_is_locked.load(std::memory_order_acquire)) {}

        // set true, after m_is_locked is false
        // memory order just ensures that store operation is after load
        // but can not avoid interleaving between two operation from another thread
        m_is_locked.store(true, std::memory_order_release);
    }

    void unlock() { m_is_locked.store(false, std::memory_order_release); }

  private:
    std::atomic<bool> m_is_locked {false};
};

// add function without spin lock
// data race exists
void add(int num)
{
    for (int i = 0; i < num; i++)
    {
        g_counter = g_counter + 1;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

// add function with spin lock
// right implementation
SpinLock g_spin_lock;
void     add_with_spinlock(int num)
{
    for (int i = 0; i < num; i++)
    {
        g_spin_lock.lock();
        g_counter = g_counter + 1;
        g_spin_lock.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

// add function with spin lock which is wrong implementation
SpinLockWrong g_spin_lock_atomic;
void          add_with_spinlock_wrong(int num)
{
    for (int i = 0; i < num; i++)
    {
        g_spin_lock_atomic.lock();
        g_counter = g_counter + 1;
        g_spin_lock_atomic.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

std::vector<std::thread> worker_threads;

void main()
{
    int thread_num = 8;

    std::cout << "begin...\n";

    g_counter = 0;
    for (int i = 0; i < thread_num; i++) { worker_threads.emplace_back(std::thread(add, 100)); }

    for (auto& thread : worker_threads) { thread.join(); }
    std::cout << "add without spin lock, counter: " << g_counter << std::endl;

    g_counter = 0;
    worker_threads.clear();
    for (int i = 0; i < thread_num; i++) { worker_threads.emplace_back(std::thread(add_with_spinlock, 100)); }

    for (auto& thread : worker_threads) { thread.join(); }
    std::cout << "add with spin lock, counter: " << g_counter << std::endl;

    g_counter = 0;
    worker_threads.clear();
    for (int i = 0; i < 8; i++) { worker_threads.emplace_back(std::thread(add_with_spinlock_wrong, 100)); }

    for (auto& thread : worker_threads) { thread.join(); }
    std::cout << "add with spin lock wrong, counter: " << g_counter << std::endl;
}