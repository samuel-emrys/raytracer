#include "threadsafequeue.h"
#include <atomic>
#include <functional>
#include <future>
#include <iostream>
#include <thread>
#include <vector>

class FunctionWrapper {
  public:
    template <typename F>
    FunctionWrapper(F&& f) : mImpl(new ImplType<F>(std::move(f))) {}
    FunctionWrapper() = default;
    FunctionWrapper(FunctionWrapper&& rOther) noexcept : mImpl(std::move(rOther.mImpl)) {}
    auto operator=(FunctionWrapper&& rOther) noexcept -> FunctionWrapper& {
        mImpl = std::move(rOther.mImpl);
        return *this;
    }
    FunctionWrapper(const FunctionWrapper&) = delete;
    FunctionWrapper(FunctionWrapper&) = delete;
    ~FunctionWrapper() = default;
    auto operator=(const FunctionWrapper&) -> FunctionWrapper& = delete;
    void operator()() { mImpl->call(); }

  private:
    struct ImplBase {
        virtual void call() = 0;
        virtual ~ImplBase() = default;
    };
    std::unique_ptr<ImplBase> mImpl;
    template <typename F>
    struct ImplType : ImplBase {
        F f;
        ImplType(F&& f_) : f(std::move(f_)){};
        void call() override { f(); }
    };
};

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
    std::vector<std::thread>& mThreads;
};

class ThreadPool {
  public:
    ThreadPool();
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
     * @param rFunction A callable object
     * @return A std::future<> to hold the return value of the task
     */
    template <typename FunctionType>
    auto submit(FunctionType rFunction)
        -> std::future<typename std::result_of<FunctionType()>::type> {
        using ResultType = typename std::result_of<FunctionType()>::type;
        std::packaged_task<ResultType()> vTask(std::move(rFunction));
        std::future<ResultType> vResult(vTask.get_future());
        mWorkQueue.push(std::move(vTask));
        return vResult;
    }
    auto runPendingTask() -> void;

  private:
    std::atomic_bool mDone;
    ThreadSafeQueue<FunctionWrapper> mWorkQueue;
    std::vector<std::thread> mThreads;
    JoinThreads mJoiner;
    auto workerThread() -> void;
};
