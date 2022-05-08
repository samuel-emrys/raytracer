#include <atomic>
#include <condition_variable>
#include <cstdlib>
#include <deque>
#include <gsl/gsl-lite.hpp>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>

/**
 * @brief A class to wrap functions for use in a thread pool
 *
 * @details This class allows std::packaged_task<> instances to be wrapped and used in a
 * thread pool. Unfortunately, std::packaged_task<> instances are not copyable, just
 * movable, and so std::function<> is not suitable for queue entries where
 * std::packaged_task<> is used, as std::function<> requires that stored function objects
 * are copy-constructible. This wrapper is able to handle move-only types. It is a simple
 * type-erasure class with a function call operator.
 */
class FunctionWrapper {
  public:
    template <typename F>
    // NOLINT [google-implicit-constructor]
    FunctionWrapper(F&& f) : mImpl(new ImplType<F>(std::move(f))) {} // NOLINT
    /**
     * @brief Default constructor
     */
    FunctionWrapper() = default;
    /**
     * @brief Move constructor
     */
    FunctionWrapper(FunctionWrapper&& rOther) noexcept : mImpl(std::move(rOther.mImpl)) {}
    /**
     * @brief Move assignment operator
     */
    auto operator=(FunctionWrapper&& rOther) noexcept -> FunctionWrapper& {
        mImpl = std::move(rOther.mImpl);
        return *this;
    }
    /**
     * @brief Copy constructor
     */
    FunctionWrapper(const FunctionWrapper&) = delete;
    /**
     * @brief Copy constructor
     */
    FunctionWrapper(FunctionWrapper&) = delete;
    /**
     * @brief Copy assignment operator
     */
    auto operator=(const FunctionWrapper&) -> FunctionWrapper& = delete;
    /**
     * @brief Default destructor
     */
    ~FunctionWrapper() = default;
    /**
     * @brief Make object callable and invoke the wrapped function
     */
    void operator()() { mImpl->call(); }

  private:
    struct ImplBase {
        ImplBase() = default;
        ImplBase(const ImplBase&) = delete;
        ImplBase(ImplBase&&) = delete;
        auto operator=(const ImplBase&) -> ImplBase& = delete;
        auto operator=(ImplBase&&) -> ImplBase& = delete;
        virtual ~ImplBase() = default;

        virtual void call() = 0;
    };
    template <typename F>
    struct ImplType : ImplBase {
        F f;
        ImplType(F&& f_) : f(std::move(f_)){}; // NOLINT [google-explicit-constructor]
        void call() override { f(); }
    };
    std::unique_ptr<ImplBase> mImpl;
};

/**
 * @brief A thread safe queue to be used in concurrent operations
 *
 * @tparam T The type of element to be stored in the queue
 */
template <typename T>
class ThreadSafeQueue {
  public:
    ThreadSafeQueue() : mHead(new Node), mTail(mHead.get()) {}
    ThreadSafeQueue(const ThreadSafeQueue&) = delete;
    ThreadSafeQueue(ThreadSafeQueue&&) = delete;
    auto operator=(const ThreadSafeQueue&) -> ThreadSafeQueue& = delete;
    auto operator=(ThreadSafeQueue&&) -> ThreadSafeQueue& = delete;
    ~ThreadSafeQueue() = default;
    /**
     * @brief Retrieve and remove the first element from the queue
     *
     * @return A shared_ptr to the first element in the queue
     */
    auto pop() -> std::shared_ptr<T> {
        std::unique_ptr<Node> vOldHead = tryPopHead();
        return vOldHead ? vOldHead->data : std::make_shared<T>();
    }
    /**
     * @brief Retrieve and remove the first element from the queue
     *
     * @param[in] rValue The object in which to place the head element
     * @return True if the object was successfully popped, false otherwise
     */
    auto pop(T& rValue) -> bool {
        std::unique_ptr<Node> const vOldHead = tryPopHead(rValue);
        return static_cast<bool>(vOldHead);
    }
    /**
     * @brief Add an element to the queue
     *
     * @param[in] rNewValue The element to add to the queue
     */
    auto push(T rNewValue) -> void {
        auto vNewData = std::make_shared<T>(std::move(rNewValue));
        auto vNewNode = std::make_unique<Node>();
        // Scope the operations that need the mutex to be locked
        {
            Node* const pvNewTail = vNewNode.get();
            std::lock_guard<std::mutex> vTailLock(mTailMutex);
            mTail->data = vNewData;
            mTail->next = std::move(vNewNode);
            mTail = pvNewTail;
        }
        // Notify a thread that it can now modify the queue
        mDataCond.notify_one();
    }
    /**
     * @brief Query if the queue is empty
     *
     * @return True if the queue is empty, false otherwise
     */
    [[nodiscard]] auto empty() -> bool {
        std::lock_guard<std::mutex> vHeadLock(mHeadMutex);
        return (mHead.get() == getTail());
    }

  private:
    /**
     * @brief An object to represent each node in the linked list representing the queue
     */
    struct Node {
        /**
         * @brief The data associated with the node
         */
        std::shared_ptr<T> data;
        /**
         * @brief A pointer to the next element in the queue
         *
         * @details The unique_ptr ensures that any reference to the next node is
         * destructed when the Node object is destructed.
         */
        std::unique_ptr<Node> next;
    };
    /**
     * @brief Get the last element in the queue
     *
     * @return A raw pointer to the last Node in the queue
     */
    auto getTail() -> Node* {
        std::lock_guard<std::mutex> vTailLock(mTailMutex);
        return mTail;
    }
    /**
     * @brief Get the head of the queue, and remove it from the queue
     *
     * @return A unique pointer to the head Node of the queue
     */
    auto popHead() -> std::unique_ptr<Node> {
        std::unique_ptr<Node> vOldHead = std::move(mHead);
        mHead = std::move(vOldHead->next);
        return vOldHead;
    }
    /**
     * @brief Attempt to pop the head of the queue
     *
     * @details This is designed to be used for concurrent access
     * @return nullptr if the head is the same element as the tail, a unique_ptr to the
     * head node otherwise.
     */
    auto tryPopHead() -> std::unique_ptr<Node> {
        std::lock_guard<std::mutex> vHeadLock(mHeadMutex);
        if (mHead.get() == getTail()) {
            return nullptr;
        }
        return popHead();
    }
    /**
     * @brief Attempt to pop the head of the queue
     *
     * @details This is designed to be used for concurrent access
     * @param[out] rValue The data held in the head node
     * @return nullptr if the head is the same element as the tail, a unique_ptr to the
     * head node otherwise.
     */
    auto tryPopHead(T& rValue) -> std::unique_ptr<Node> {
        std::lock_guard<std::mutex> vHeadLock(mHeadMutex);
        if (mHead.get() == getTail()) {
            return nullptr;
        }
        rValue = std::move(*mHead->data);
        return popHead();
    }
    /**
     * @brief A mutex to control access to the head element of the queue
     */
    std::mutex mHeadMutex;
    /**
     * @brief Condition variable to block and release async access by threads
     */
    std::condition_variable mDataCond;
    /**
     * @brief A mutex to control access to the tail element of the queue
     */
    std::mutex mTailMutex;
    /**
     * @brief Pointer to the head element of the queue
     */
    std::unique_ptr<Node> mHead;
    /**
     * @brief Pointer to the tail element of the queue
     */
    Node* mTail;
};

/**
 * @brief A queue that allows elements to be stolen from the back of the queue
 */
class WorkStealingQueue {
  public:
    using DataType = FunctionWrapper;
    WorkStealingQueue() = default;
    WorkStealingQueue(WorkStealingQueue&&) = delete;
    auto operator=(WorkStealingQueue&&) = delete;
    WorkStealingQueue(const WorkStealingQueue&) = delete;
    auto operator=(const WorkStealingQueue&) = delete;
    ~WorkStealingQueue() = default;

    /**
     * @brief Push an element to the front of the queue
     *
     * @param[in] rData The data to push to the front of the queue
     */
    auto push(DataType rData) -> void {
        std::lock_guard<std::mutex> vLock(mMutex);
        mQueue.push_front(std::move(rData));
    }
    /**
     * @brief Retrieve and remove the first element from the queue
     *
     * @param[out] rResource The object in which to store the resource removed from the
     * queue
     * @return True if the element is successfully popped, false otherwise
     */
    auto pop(DataType& rResource) -> bool {
        std::lock_guard<std::mutex> vLock(mMutex);
        if (mQueue.empty()) {
            return false;
        }
        rResource = std::move(mQueue.front());
        mQueue.pop_front();
        return true;
    }
    /**
     * @brief Steal an element from the back of the queue
     *
     * @param[out] rResource The object in which to store the resource removed from the
     * queue
     * @return True if the element is successfully stolen, false otherwise
     */
    auto steal(DataType& rResource) -> bool {
        std::lock_guard<std::mutex> vLock(mMutex);
        if (mQueue.empty()) {
            return false;
        }
        rResource = std::move(mQueue.back());
        mQueue.pop_back();
        return true;
    }
    /**
     * @brief Query if the queue is empty
     *
     * @return True if the queue is empty, false otherwise
     */
    [[nodiscard]] auto empty() const -> bool {
        std::lock_guard<std::mutex> vLock(mMutex);
        return mQueue.empty();
    }

  private:
    /**
     * @brief The queue
     */
    std::deque<DataType> mQueue;
    /**
     * @brief The mutex to lock the object from being modified by more than one thread at
     * a time. This is mutable to allow the mutable to be modified by functions marked
     * const.
     */
    mutable std::mutex mMutex;
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
