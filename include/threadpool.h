#include "threadsafequeue.h"
#include <atomic>
#include <functional>
#include <future>
#include <iostream>
#include <memory>
#include <queue>
#include <thread>
#include <vector>

/**
 * @brief Provides RAII management of threads by joining all threads managed on
 * destruction.
 */
class JoinThreads {
  public:
    explicit JoinThreads(std::vector<std::thread>& rThreads) : mThreads(rThreads) {}
    JoinThreads(const JoinThreads&) = delete;
    JoinThreads(JoinThreads&&) = delete;
    auto operator=(const JoinThreads&) -> JoinThreads& = delete;
    auto operator=(JoinThreads&&) -> JoinThreads& = delete;
    ~JoinThreads() {
        // Join all threads on destruction
        for (auto& vThread : mThreads) {
            if (vThread.joinable()) {
                vThread.join();
            }
        }
    }

  private:
    /**
     * @brief Reference to the threads to be joined
     */
    std::vector<std::thread>& mThreads;
};

/**
 * @brief Manages a pool of threads and coordinates their tasking
 */
class ThreadPool {
  public:
    /**
     * @brief Constructor
     */
    ThreadPool();
    /**
     * @brief Destructor
     */
    ~ThreadPool();
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&) = delete;
    auto operator=(const ThreadPool&) -> ThreadPool& = delete;
    auto operator=(ThreadPool&&) -> ThreadPool& = delete;
    /**
     * @brief Submits a task to the work queue and provides a std::future<> to allow the
     * caller to wait for the task to complete.
     *
     * @tparam FunctionType A callable object type
     * @param[in] rFunction A callable object
     * @return A std::future<> to hold the return value of the task
     */
    template <typename FunctionType>
    auto submit(FunctionType rFunction)
        -> std::future<typename std::invoke_result<FunctionType>::type> {
        using ResultType = typename std::invoke_result<FunctionType>::type;
        // Wrap rFunction in a std::packaged_task<> so that we can store and access a std::future
        // vTask takes no input parameters
        std::packaged_task<ResultType()> vTask(std::move(rFunction));
        std::future<ResultType> vResult(vTask.get_future());
        if (mLocalWorkQueue) {
            mLocalWorkQueue->push(std::move(vTask));
        } else {
            mGlobalWorkQueue.push(std::move(vTask));
        }
        return vResult;
    }
    /**
     * @brief Pop a task off the queue and execute it
     */
    auto runPendingTask() -> void;

  private:
    using TaskType = FunctionWrapper;
    /**
     * @brief Function to run as a task on each thread in the threadpool.
     *
     * @details This function will continue to attempt to pop tasks from the work queues
     * until mDone=true.
     * @param[in] rIndex The index of the work queue in mQueues assigned to the thread
     */
    auto workerThread(size_t rIndex) -> void;
    /**
     * @brief Retrieve a task from the local work queue
     *
     * @param[out] rTask A handle to the task popped
     * @return True if a task was successfully popped, false otherwise.
     */
    auto popTaskFromLocalQueue(TaskType& rTask) -> bool;
    /**
     * @brief Retrieve a task from the global work queue
     *
     * @param[out] rTask A handle to the task popped
     * @return True if a task was successfully popped, false otherwise.
     */
    auto popTaskFromGlobalQueue(TaskType& rTask) -> bool;
    /**
     * @brief Steal a task from another thread's work queue
     *
     * @param[out] rTask A handle to the task stolen
     * @return True if a task was successfully stolen, false otherwise.
     */
    auto stealTaskFromOtherThread(TaskType& rTask) -> bool;

    /**
     * @brief Flag indicating that execution has concluded
     */
    std::atomic_bool mDone;
    /**
     * @brief The pool of threads
     */
    std::vector<std::thread> mThreads;
    /**
     * @brief The local work queue for each thread
     */
    std::vector<std::unique_ptr<WorkStealingQueue>> mQueues;
    /**
     * @brief The global work queue
     */
    ThreadSafeQueue<FunctionWrapper> mGlobalWorkQueue;
    /**
     * @brief The RAII join manager of the threads in the pool
     */
    JoinThreads mJoiner;
    /**
     * @brief The local work queue. This is unique to each thread.
     */
    // NOLINT [-Wc++17-extensions]
    inline static thread_local WorkStealingQueue* mLocalWorkQueue = nullptr; // NOLINT
    /**
     * @brief The index of the local work queue in the mQueues structure. This is unique
     * to each thread.
     */
    inline static thread_local size_t mThreadIndex = 0; // NOLINT [-Wc++17 extensions]
};
