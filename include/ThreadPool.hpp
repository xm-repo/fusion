#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <vector>
#include <thread>
#include <future> //packaged_task
#include <queue>
#include <functional> //bind
#include <mutex>
#include <condition_variable>
#include <type_traits> //invoke_result

namespace MegaBI
{

class ThreadPool 
{
public:
    ThreadPool(const std::size_t thread_count)
    {
        for (std::size_t i = 0; i < thread_count; ++i) 
        {            
            m_threads.emplace_back(std::thread([&]() 
            {
                std::unique_lock<std::mutex> queue_lock(m_mutex, std::defer_lock);

                while (true) 
                {
                    queue_lock.lock();
                    m_cv.wait(queue_lock, [&]() { return !m_tasks.empty() || m_stop; });

                    if (m_stop)
                    {
                        return;
                    }

                    auto temp_task = std::move(m_tasks.front());
                    
                    m_tasks.pop();
                    queue_lock.unlock();
                    
                    std::invoke(*temp_task);
                }
            }));
        }
    }

    ~ThreadPool()
    {
        m_stop = true;
        m_cv.notify_all();

        for (auto& thread : m_threads) 
        {
            thread.join();
        }
    }

    template <typename F, typename... Args,
        std::enable_if_t<std::is_invocable_v<F&&, Args&&...>, int> = 0> 
        auto execute(F&& function, Args&&... args)
    {
        std::packaged_task<std::invoke_result_t<F, Args...>()> task_pkg(
            std::bind(std::forward<F>(function), maybe_wrap(std::forward<Args>(args))...));

        auto future = task_pkg.get_future();

        {
            std::lock_guard<std::mutex> queue_lock(m_mutex);
            m_tasks.emplace(allocate_container(std::move(task_pkg)));
        }

        m_cv.notify_one();

        return future;
    }

private:

    template <class T> std::reference_wrapper<T> maybe_wrap(T& val) { return std::ref(val); }
    template <class T> T&& maybe_wrap(T&& val) { return std::forward<T>(val); }

    class task_container_base 
    {
    public:
        virtual ~task_container_base() {};
        virtual void operator()() = 0;
    };

    template <typename F>
    class task_container : public task_container_base 
    {
    public:
        task_container(F&& func) : m_f(std::forward<F>(func)) { }
        virtual void operator()() override { m_f(); }
    private:
        F m_f;
    };

    using taskPtr = std::unique_ptr<task_container_base>;

    template <typename F>
    static auto allocate_container(F&& f)
    {
        return taskPtr(new task_container<F>(std::forward<F>(f)));
    }

    std::vector<std::thread> m_threads;
    std::queue<taskPtr> m_tasks;
    std::mutex m_mutex;
    std::condition_variable m_cv;
    bool m_stop = false;
};

}

#endif // THREADPOOL_H
