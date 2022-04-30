#include "threadpool.h"
#include <functional>

ThreadPool::ThreadPool() :
    mDone(false),
    mJoiner(mThreads)
{
    auto vThreadCount = std::thread::hardware_concurrency();
    try {
        for (size_t vI = 0; vI < vThreadCount; ++vI) {
            mThreads.emplace_back(&ThreadPool::workerThread, this);
        }
    } catch (...) {
        mDone = true;
        throw;
    }
};

ThreadPool::~ThreadPool() {
    mDone = true;
}

auto ThreadPool::workerThread() -> void {
    // Loop forever until mDone = true
    while (!mDone) {
        FunctionWrapper vTask;
        // Attempt to pop a task off the queue
        if (mWorkQueue.pop(vTask)) {
            // Execute the popped task if successful
            vTask();
        }
        else {
            std::this_thread::yield();
        }
    }
}

auto ThreadPool::runPendingTask() -> void {
    FunctionWrapper vTask;

    if (mWorkQueue.pop(vTask)) {
        vTask();
    }
    else {
        std::this_thread::yield();
    }
}
