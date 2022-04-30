#include <atomic>
#include <condition_variable>
#include <cstdlib>
#include <gsl/gsl-lite.hpp>
#include <memory>
#include <mutex>

template <typename T>
class ThreadSafeQueue {
  public:
    ThreadSafeQueue() : mHead(new Node), mTail(mHead.get()) {}
    ThreadSafeQueue(const ThreadSafeQueue&) = delete;
    ThreadSafeQueue(ThreadSafeQueue&&) = delete;
    auto operator=(const ThreadSafeQueue&) -> ThreadSafeQueue& = delete;
    auto operator=(ThreadSafeQueue&&) -> ThreadSafeQueue& = delete;
    ~ThreadSafeQueue() { delete mTail; }

    auto pop() -> std::shared_ptr<T> {
        std::unique_ptr<Node> vOldHead = tryPopHead();
        return vOldHead ? vOldHead->data : std::make_shared<T>();
    }
    auto pop(T& rValue) -> bool {
        std::unique_ptr<Node> const vOldHead = tryPopHead(rValue);
        return static_cast<bool>(vOldHead);
    }
    auto push(T rNewValue) -> void {
        auto vNewData = std::make_shared<T>(std::move(rNewValue));
        auto vNewNode = std::make_unique<Node>();
        {
            std::lock_guard<std::mutex> vTailLock(mTailMutex);
            mTail->data = vNewData;
            Node* const pvNewTail = vNewNode.get();
            mTail->next = std::move(vNewNode);
            mTail = pvNewTail;
        }
        mDataCond.notify_one();
    }
    auto empty() -> bool {
        std::lock_guard<std::mutex> vHeadLock(mHeadMutex);
        return (mHead.get() == getTail());
    }

  private:
    struct Node {
        std::shared_ptr<T> data;
        std::unique_ptr<Node> next;
    };
    auto getTail() -> Node* {
        std::lock_guard<std::mutex> vTailLock(mTailMutex);
        return mTail;
    }
    auto popHead() -> std::unique_ptr<Node> {
        std::unique_ptr<Node> vOldHead = std::move(mHead);
        mHead = std::move(vOldHead->next);
        return vOldHead;
    }
    auto tryPopHead() -> std::unique_ptr<Node> {
        std::lock_guard<std::mutex> vHeadLock(mHeadMutex);
        if (mHead.get() == getTail()) {
            //return std::make_unique<Node>();
            return std::unique_ptr<Node>();
        }
        return popHead();
    }
    auto tryPopHead(T& rValue) -> std::unique_ptr<Node> {
        std::lock_guard<std::mutex> vHeadLock(mHeadMutex);
        if (mHead.get() == getTail()) {
            //return std::make_unique<Node>();
            return std::unique_ptr<Node>();
        }
        rValue = std::move(*mHead->data);
        return popHead();
    }

    std::mutex mHeadMutex;
    std::unique_ptr<Node> mHead;
    std::mutex mTailMutex;
    Node* mTail;
    std::condition_variable mDataCond;
};

template <typename T>
class ThreadSafeLockFreeQueue {
  public:
    ThreadSafeLockFreeQueue() {
        CountedNodePtr vHead = {.externalCount = 0, .ptr = new Node};
        mHead.store(vHead);
        mTail.store(mHead.load());
    };
    ThreadSafeLockFreeQueue(const ThreadSafeLockFreeQueue&) = delete;
    ThreadSafeLockFreeQueue(ThreadSafeLockFreeQueue&&) = delete;
    auto operator=(const ThreadSafeLockFreeQueue&) -> ThreadSafeLockFreeQueue& = delete;
    auto operator=(ThreadSafeLockFreeQueue&&) -> ThreadSafeLockFreeQueue& = delete;
    ~ThreadSafeLockFreeQueue() {
        while (Node* const pvOldHead = mHead.load().ptr) {
            mHead.store(pvOldHead->next);
            delete pvOldHead;
        }
    }
    auto pop() -> std::unique_ptr<T> {
        CountedNodePtr vOldHead = mHead.load(std::memory_order_relaxed);
        for (;;) {
            increaseExternalCount(mHead, vOldHead);
            Node* const pvPtr = vOldHead.ptr;
            if (pvPtr == mTail.load().ptr) {
                // pvPtr->release_ref();
                return std::make_unique<T>();
            }
            CountedNodePtr vNext = pvPtr->next.load();
            if (mHead.compare_exchange_strong(vOldHead, vNext)) {
                T* const pvResource = pvPtr->data.exchange(nullptr);
                freeExternalCounter(vOldHead);
                return std::make_unique<T>(pvResource);
            }
            pvPtr->release_ref();
        }
    }
    auto pop(T& rValue) -> bool {
        CountedNodePtr vOldHead = mHead.load(std::memory_order_relaxed);
        for (;;) {
            increaseExternalCount(mHead, vOldHead);
            Node* const pvPtr = vOldHead.ptr;
            if (pvPtr == mTail.load().ptr) {
                // pvPtr->release_ref();
                return false;
            }
            CountedNodePtr vNext = pvPtr->next.load();
            if (mHead.compare_exchange_strong(vOldHead, vNext)) {
                T* const pvResource = pvPtr->data.exchange(nullptr);
                freeExternalCounter(vOldHead);
                rValue = std::move(*pvResource);
                return true;
            }
            pvPtr->release_ref();
        }
    }
    auto push(T rNewValue) -> void {
        auto vNewData = std::make_unique<T>(rNewValue);
        CountedNodePtr vNewNext{.externalCount = 1, .ptr = new Node};
        CountedNodePtr vOldTail = mTail.load();
        for (;;) {
            increaseExternalCount(mTail, vOldTail);
            T* pvOldData = nullptr;

            // Set the `data` pointer in the node
            // If successful, this will break out of the loop
            if (vOldTail.ptr->data.compare_exchange_strong(
                    pvOldData,
                    vNewData.get())) { // This causes a segfault. Need to work out why
                CountedNodePtr vOldNext = {.externalCount = 0, .ptr = nullptr};
                // Update the `next` pointer
                // compare_exchange_strong will avoid looping - if it fails, we know that
                // another thread has already set the `next` pointer so we don't need the
                // new node allocated at the beginning and we can delete it
                if (!vOldTail.ptr->next.compare_exchange_strong(vOldNext, vNewNext)) {
                    delete vNewNext.ptr;
                    vNewNext = vOldNext;
                }
                // Use the `next` value that the other thread set to update the tail
                setNewTail(vOldTail, vNewNext);
                vNewData.release();
                break;
            }
            // If not successful setting the data pointer, handle the case where we're
            // being helped by another thread
            CountedNodePtr vOldNext = {.externalCount = 0, .ptr = nullptr};
            // Try to update the next pointer to the new node allocated on this thread
            if (vOldTail.ptr->next.compare_exchange_strong(vOldNext, vNewNext)) {
                // Use the node previously allocated as the new tail node
                vOldNext = vNewNext;
                // Allocate another new node in anticipation of managing to push an item
                // to the queue
                vNewNext.ptr = new Node;
            }
            // Attempt to set the new tail node
            setNewTail(vOldTail, vOldNext);
        }
    }

  private:
    struct Node;
    struct CountedNodePtr {
        int externalCount;
        gsl::owner<Node*> ptr;
    };
    struct NodeCounter {
        unsigned internalCount : 30;
        unsigned externalCounters : 2;
    };
    struct Node {
        Node() {
            count.store({.internalCount = 0, .externalCounters = 2});
            next.store({.externalCount = 0, .ptr = nullptr});
        }
        auto release_ref() -> void {
            NodeCounter vOldCounter = count.load(std::memory_order_relaxed);
            NodeCounter vNewCounter;
            do {
                vNewCounter = vOldCounter;
                --vNewCounter.internalCount;
            } while (!count.compare_exchange_strong(vOldCounter,
                                                    vNewCounter,
                                                    std::memory_order_acquire,
                                                    std::memory_order_relaxed));
            if (!vNewCounter.internalCount && !vNewCounter.externalCounters) {
                delete this;
            }
        }
        std::atomic<T*> data;
        std::atomic<NodeCounter> count;
        std::atomic<CountedNodePtr> next;
    };

    std::atomic<CountedNodePtr> mHead;
    std::atomic<CountedNodePtr> mTail;
    auto pop_head() -> Node* {
        Node* const pvOldHead = mHead.load();
        if (pvOldHead == mTail.load()) {
            return nullptr;
        }
        mHead.store(pvOldHead->next);
        return pvOldHead;
    }
    static auto increaseExternalCount(std::atomic<CountedNodePtr>& rCounter,
                                      CountedNodePtr& rOldCounter) -> void {
        CountedNodePtr vNewCounter;
        do {
            vNewCounter = rOldCounter;
            ++vNewCounter.externalCount;
        } while (!rCounter.compare_exchange_strong(rOldCounter,
                                                   vNewCounter,
                                                   std::memory_order_acquire,
                                                   std::memory_order_relaxed));
        rOldCounter.externalCount = vNewCounter.externalCount;
    }
    static auto freeExternalCounter(CountedNodePtr& rOldNodePtr) -> void {
        Node* const pvPtr = rOldNodePtr.ptr;
        int const vCountIncrease = rOldNodePtr.externalCount - 2;
        NodeCounter vOldCounter = pvPtr->count.load(std::memory_order_relaxed);
        NodeCounter vNewCounter;
        do {
            vNewCounter = vOldCounter;
            --vNewCounter.externalCounters;
            vNewCounter.internalCount += vCountIncrease;
        } while (!pvPtr->count.compare_exchange_strong(vOldCounter,
                                                       vNewCounter,
                                                       std::memory_order_acquire,
                                                       std::memory_order_relaxed));
        if (!vNewCounter.internalCount && !vNewCounter.externalCounters) {
            delete pvPtr;
        }
    }
    auto setNewTail(CountedNodePtr& rOldTail, CountedNodePtr const& rNewTail) -> void {
        Node* const pvCurrentTail = rOldTail.ptr;
        // Use compare_exchange_weak to update the tail, because if other threads are
        // trying to `push()` a new node, the externalCount may have changed, and we don't
        // want to lose it. We also need to take care that we don't replace the value if
        // another thread has successfully changed it already; otherwise, we may end up
        // with loops in the queue
        while (!mTail.compare_exchange_weak(rOldTail, rNewTail)
               && rOldTail.ptr == pvCurrentTail) {};
        // If compare_exchange_weak fails, we need to ensure that `ptr` of the loaded
        // value is the same
        if (rOldTail.ptr == pvCurrentTail) {
            // This thread must have successfully set the tail, so need to free old
            // external counter
            freeExternalCounter(rOldTail);
        } else {
            // Another thread has freed the counter, so need to release the reference held
            // by this thread
            pvCurrentTail->release_ref();
        }
    }
};
