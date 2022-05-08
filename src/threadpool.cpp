#include "threadpool.h"
#include <functional>
#include <memory>
#include <stdexcept>
#include <thread>

ThreadPool::ThreadPool() : mDone(false), mJoiner(mThreads) {
    auto vThreadCount = std::thread::hardware_concurrency();
    try {
        for (size_t vI = 0; vI < vThreadCount; ++vI) {
            mQueues.push_back(std::make_unique<WorkStealingQueue>());
        }
        // Add worker threads only once all queues have been initialised.
        // Adding worker threads prior to queue intialisation leads to worker threads
        // attempting to steal from queues whilst they're still being constructed.
        // I think this is caused by the fact that a worker thread will attempt to steal
        // from the next thread in mQueues, which is not a thread safe action during
        // construction This results in an attempt to lock mMutex of the other threads
        // queue, which is not yet valid, and thus results in a segmentation fault.
        for (size_t vI = 0; vI < vThreadCount; ++vI) {
            mThreads.emplace_back(&ThreadPool::workerThread, this, vI);
        }
    } catch (...) {
        mDone = true;
        throw;
    }
};

ThreadPool::~ThreadPool() {
    mDone = true;
}

auto ThreadPool::workerThread(size_t rIndex) -> void {
    mThreadIndex = rIndex;
    mLocalWorkQueue = mQueues[mThreadIndex].get();
    // Loop forever until mDone = true
    while (!mDone) {
        runPendingTask();
    }
}

// [readability-convert-member-functions-to-static]
auto ThreadPool::popTaskFromLocalQueue(TaskType& rTask) -> bool { // NOLINT
    return (mLocalWorkQueue != nullptr) && !mLocalWorkQueue->empty()
        && mLocalWorkQueue->pop(rTask);
}

auto ThreadPool::popTaskFromGlobalQueue(TaskType& rTask) -> bool {
    return mGlobalWorkQueue.pop(rTask);
}

auto ThreadPool::stealTaskFromOtherThread(TaskType& rTask) -> bool {
    for (size_t vI = 0; vI < mQueues.size(); ++vI) {
        // Sweep from current thread index to reduce thread contention
        auto vIndex = (mThreadIndex + vI + 1) % mQueues.size();
        if (mQueues.at(vIndex) && mQueues.at(vIndex)->steal(rTask)) {
            return true;
        }
    }
    return false;
}

auto ThreadPool::runPendingTask() -> void {
    TaskType vTask;
    // Attempt to pop a task off the queue
    if (popTaskFromLocalQueue(vTask) || popTaskFromGlobalQueue(vTask)
        || stealTaskFromOtherThread(vTask)) {
        vTask();
    } else {
        // No tasks to be stolen, yield the thread
        std::this_thread::yield();
    }
}
