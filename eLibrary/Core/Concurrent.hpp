#pragma once

#ifndef eLibraryHeaderCoreConcurrent
#define eLibraryHeaderCoreConcurrent

#include <Core/Number.hpp>

#if eLibraryCompiler(MSVC)
#include <intrin.h>
#endif

#if eLibrarySystem(Windows)
#include <Windows.h>
#else
#include <pthread.h>
#include <unistd.h>
#endif

namespace eLibrary::Core {
    class ConcurrentUtility final : public Object {
    public:
        constexpr ConcurrentUtility() noexcept = delete;

#if eLibraryCompiler(MSVC)
        static int16_t doCompareAndExchange16(volatile int16_t *ValueAddress, int16_t ValueExpected, int16_t ValueTarget) noexcept {
            return InterlockedCompareExchange16(ValueAddress, ValueTarget, ValueExpected);
        }

        static int32_t doCompareAndExchange32(volatile int32_t *ValueAddress, int32_t ValueExpected, int32_t ValueTarget) noexcept {
            return InterlockedCompareExchange((volatile LONG*) ValueAddress, ValueTarget, ValueExpected);
        }

        static int64_t doCompareAndExchange64(volatile int64_t *ValueAddress, int64_t ValueExpected, int64_t ValueTarget) noexcept {
            return InterlockedCompareExchange64(ValueAddress, ValueTarget, ValueExpected);
        }
#else
        static int16_t doCompareAndExchange16(volatile int16_t *ValueAddress, int16_t ValueExpected, int16_t ValueTarget) noexcept {
            int16_t ValueResult;
            asm volatile("lock\n\tcmpxchgw %2, (%3)":"=a"(ValueResult):"a"(ValueExpected), "r"(ValueTarget), "r"(ValueAddress):"cc", "memory");
            return ValueResult;
        }

        static int32_t doCompareAndExchange32(volatile int32_t *ValueAddress, int32_t ValueExpected, int32_t ValueTarget) noexcept {
            int32_t ValueResult;
            asm volatile("lock\n\tcmpxchgl %2, (%3)":"=a"(ValueResult):"a"(ValueExpected), "r"(ValueTarget), "r"(ValueAddress):"cc", "memory");
            return ValueResult;
        }

        static int64_t doCompareAndExchange64(volatile int64_t *ValueAddress, int64_t ValueExpected, int64_t ValueTarget) noexcept {
            int64_t ValueResult;
            asm volatile("lock\n\tcmpxchgq %2, (%3)":"=a"(ValueResult):"a"(ValueExpected), "r"(ValueTarget), "r"(ValueAddress):"cc", "memory");
            return ValueResult;
        }
#endif

        template<typename T>
        static T doCompareAndExchange(volatile T *ValueAddress, T ValueExpected, T ValueTarget) noexcept {
            if constexpr (sizeof(T) == sizeof(int16_t)) return doCompareAndExchange16((volatile int16_t*) ValueAddress, (int16_t) ValueExpected, (int16_t) ValueTarget);
            else if constexpr (sizeof(T) == sizeof(int32_t)) return doCompareAndExchange32((volatile int32_t*) ValueAddress, (int32_t) ValueExpected, (int32_t) ValueTarget);
            else if constexpr (sizeof(T) == sizeof(int64_t)) return doCompareAndExchange64((volatile int64_t*) ValueAddress, (int64_t) ValueExpected, (int64_t) ValueTarget);
        }

        template<typename T>
        static auto doCompareAndExchangeReference(volatile T **ValueAddress, T *ValueExpected, T *ValueTarget) noexcept {
            return doCompareAndExchange((volatile intptr_t*) ValueAddress, (intptr_t) ValueExpected, (intptr_t) ValueTarget);
        }

        template<Arithmetic T>
        static bool doCompareAndSet(volatile T *ValueAddress, T ValueExpected, T ValueTarget) noexcept {
            doCompareAndExchange(ValueAddress, ValueExpected, ValueTarget);
            return *ValueAddress == ValueTarget;
        }

        template<typename T>
        static bool doCompareAndSetReference(volatile T **ValueAddress, T *ValueExpected, T *ValueTarget) noexcept {
            doCompareAndExchangeReference(ValueAddress, ValueExpected, ValueTarget);
            return *ValueAddress == ValueTarget;
        }

        template<Arithmetic T>
        static T getAndAddNumber(volatile T *ValueAddress, T ValueDelta) noexcept {
            T ValueExpected;
            do {
                ValueExpected = *ValueAddress;
            } while (ConcurrentUtility::doCompareAndExchange(ValueAddress, ValueExpected, ValueExpected + ValueDelta) != ValueExpected);
            return ValueExpected;
        }

        template<Arithmetic T>
        static T getAndAndNumber(volatile T *ValueAddress, T ValueSource) noexcept {
            T ValueExpected;
            do {
                ValueExpected = *ValueAddress;
            } while (ConcurrentUtility::doCompareAndExchange(ValueAddress, ValueExpected, ValueExpected & ValueSource) != ValueExpected);
            return ValueExpected;
        }

        template<Arithmetic T>
        static T getAndOrNumber(volatile T *ValueAddress, T ValueSource) noexcept {
            T ValueExpected;
            do {
                ValueExpected = *ValueAddress;
            } while (ConcurrentUtility::doCompareAndExchange(ValueAddress, ValueExpected, ValueExpected | ValueSource) != ValueExpected);
            return ValueExpected;
        }

        template<Arithmetic T>
        static T getAndSetNumber(volatile T *ValueAddress, T ValueTarget) noexcept {
            T ValueExpected;
            do {
                ValueExpected = *ValueAddress;
            } while (ConcurrentUtility::doCompareAndExchange(ValueAddress, ValueExpected, ValueTarget) != ValueExpected);
            return ValueExpected;
        }

        template<typename T>
        static T *getAndSetReference(volatile T **ValueAddress, T *ValueTarget) noexcept {
            T *ValueExpected;
            do {
                ValueExpected = *ValueAddress;
            } while (ConcurrentUtility::doCompareAndExchangeReference(ValueAddress, ValueExpected, ValueTarget) != ValueExpected);
            return ValueExpected;
        }

        template<Arithmetic T>
        static T getAndXorNumber(volatile T *ValueAddress, T ValueSource) noexcept {
            T ValueExpected;
            do {
                ValueExpected = *ValueAddress;
            } while (ConcurrentUtility::doCompareAndExchange(ValueAddress, ValueExpected, ValueExpected ^ ValueSource) != ValueExpected);
            return ValueExpected;
        }
    };

#if eLibrarySystem(Windows)
    typedef HANDLE ThreadHandleType;
#else
    typedef pthread_t ThreadHandleType;
#endif

    /**
     * Support for multithreading
     */
    class Thread : public Object {
    private:
        ThreadHandleType ThreadHandle;
        volatile bool ThreadFinish;
        volatile bool ThreadInterrupt;

        static void *doExecuteCore(void *ThreadContext) noexcept {
            ((Thread*) ThreadContext)->ThreadFinish = false;
            ((Thread*) ThreadContext)->doExecute();
            ((Thread*) ThreadContext)->ThreadFinish = true;
            return nullptr;
        }

        constexpr Thread(ThreadHandleType ThreadHandleSource) noexcept : ThreadHandle(ThreadHandleSource), ThreadFinish(false), ThreadInterrupt(false) {}
    protected:
        void doStartCore() noexcept {
#if eLibrarySystem(Windows)
            if (ThreadHandle) CloseHandle(ThreadHandle);
            ThreadHandle = ::CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE) &Thread::doExecuteCore, this, 0, nullptr);
#else
            pthread_attr_t ThreadAttribute;
            pthread_attr_init(&ThreadAttribute);
            pthread_create(&ThreadHandle, &ThreadAttribute, Thread::doExecuteCore, this);
            pthread_attr_destroy(&ThreadAttribute);
#endif
        }

        doDisableCopyAssignConstruct(Thread)
    public:
        constexpr Thread() noexcept : ThreadHandle(ThreadHandleType()), ThreadFinish(false), ThreadInterrupt(false) {}

#if eLibrarySystem(Windows)
        ~Thread() noexcept {
            if (ThreadHandle) {
                CloseHandle(ThreadHandle);
                ThreadHandle = nullptr;
            }
        }
#endif

        virtual void doExecute() noexcept {}

        void doInterrupt() {
            if (isInterrupted()) throw InterruptedException(String(u"Thread::doInterrupt() isInterrupted"));
            ThreadInterrupt = true;
        }

        void doJoin() const {
#if eLibrarySystem(Windows)
            while (!ThreadFinish) {
                if (isInterrupted()) throw InterruptedException(String(u"Thread::doJoin() isInterrupted"));
                Sleep(10);
            }
            WaitForSingleObject(ThreadHandle, INFINITE);
#else
            while (!ThreadFinish) {
                if (isInterrupted()) throw InterruptedException(String(u"Thread::doJoin() isInterrupted"));
                usleep(10000);
            }
            pthread_join(ThreadHandle, nullptr);
#endif
        }

        virtual void doStart() {
            if (ThreadHandle) throw Exception(String(u"Thread::doStart() ThreadHandle"));
            doStartCore();
        }

        static void doYield() noexcept {
#if eLibrarySystem(Windows)
            SwitchToThread();
#else
            sched_yield();
#endif
        }

        const char *getClassName() const noexcept override {
            return "Thread";
        }

        bool isFinished() const noexcept {
            return ThreadFinish;
        }

        bool isInterrupted() const noexcept {
            return ThreadInterrupt != 0;
        }

        static Thread getCurrentThread() noexcept {
#if eLibrarySystem(Windows)
            return {GetCurrentThread()};
#else
            return {pthread_self()};
#endif
        }
    };

    template<typename F, typename ...Ts>
    class FunctionThread final : public Thread {
    private:
        F ThreadFunction;
        std::tuple<Ts...> ThreadParameter;

        doDisableCopyAssignConstruct(FunctionThread)
    public:
        constexpr explicit FunctionThread(F ThreadFunctionSource, Ts ...ThreadParameterSource) noexcept : ThreadFunction(ThreadFunctionSource), ThreadParameter(std::make_tuple(ThreadParameterSource...)) {}

        void doExecute() noexcept override {
            std::apply(ThreadFunction, ThreadParameter);
        }

        const char *getClassName() const noexcept override {
            return "FunctionThread";
        }
    };

    class ReentrantThread : public Thread {
    private:
        doDisableCopyAssignConstruct(ReentrantThread)
    public:
        constexpr ReentrantThread() noexcept = default;

        void doStart() override {
            doStartCore();
        }

        const char *getClassName() const noexcept override {
            return "ReentrantThread";
        }
    };

    template<typename F, typename ...Ts>
    class ReentrantFunctionThread final : public ReentrantThread {
    private:
        F ThreadFunction;
        std::tuple<Ts...> ThreadParameter;

        doDisableCopyAssignConstruct(ReentrantFunctionThread)
    public:
        constexpr explicit ReentrantFunctionThread(F ThreadFunctionSource, Ts ...ThreadParameterSource) noexcept : ThreadFunction(ThreadFunctionSource), ThreadParameter(std::make_tuple(ThreadParameterSource...)) {}

        void doExecute() noexcept override {
            std::apply(ThreadFunction, ThreadParameter);
        }

        const char *getClassName() const noexcept override {
            return "ReentrantFunctionThread";
        }
    };

    /**
     * A framework for implementing blocking locks and related synchronizers that rely on first-in-first-out (FIFO) wait queues
     */
    class AbstractQueuedSynchronizer : public Object {
    protected:
        enum AbstractQueuedNodeStatus {
            StatusCancelled = 0, StatusCondition, StatusPropagate, StatusSingal
        };

        struct AbstractQueuedNode {
            volatile int8_t NodeStatus;
            volatile AbstractQueuedNode *NodeNext = nullptr;
            volatile AbstractQueuedNode *NodePrevious = nullptr;

            bool setNodeNextCAS(AbstractQueuedNode *NodeExpected, AbstractQueuedNode *NodeTarget) noexcept {
                return ConcurrentUtility::doCompareAndSetReference(&NodeNext, NodeExpected, NodeTarget);
            }

            bool setNodePreviousCAS(AbstractQueuedNode *NodeExpected, AbstractQueuedNode *NodeTarget) noexcept {
                return ConcurrentUtility::doCompareAndSetReference(&NodePrevious, NodeExpected, NodeTarget);
            }

            bool setNodeStatusCAS(int8_t NodeStatusExpected, int8_t NodeStatusTarget) noexcept {
                return ConcurrentUtility::doCompareAndSet(&NodeStatus, NodeStatusExpected, NodeStatusTarget);
            }
        };
        volatile AbstractQueuedNode *NodeHead = nullptr, *NodeTail = nullptr;
    public:
        ~AbstractQueuedSynchronizer() noexcept {
            NodeTail = nullptr;
            if (NodeHead) {
                MemoryAllocator::deleteObject(NodeHead);
                NodeHead = nullptr;
            }
        }

        virtual bool tryAcquireExclusive(int) noexcept = 0;

        virtual bool tryAcquireShared(int) noexcept = 0;

        virtual bool tryReleaseExclusive(int) noexcept = 0;

        virtual bool tryReleaseShared(int) noexcept = 0;
    };

    /**
     * Support for safely operating integers in concurrent environments
     */
    template<Arithmetic T>
    class AtomicNumber final : public Object {
    private:
        volatile T NumberValue;
    public:
        explicit constexpr AtomicNumber(T NumberSource) noexcept : NumberValue(NumberSource) {}

        T addAndGet(T NumberDelta) noexcept {
            return ConcurrentUtility::getAndAddNumber(&NumberValue, NumberDelta) + NumberDelta;
        }

        T andAndGet(T NumberSource) noexcept {
            return ConcurrentUtility::getAndAddNumber(&NumberValue, NumberSource) & NumberSource;
        }

        bool compareAndSet(T ValueExpected, T ValueTarget) noexcept {
            return ConcurrentUtility::doCompareAndSet(&NumberValue, ValueExpected, ValueTarget);
        }

        T decrementAndGet() noexcept {
            return getAndDecrement() - 1;
        }

        T getAndAdd(T NumberDelta) noexcept {
            return ConcurrentUtility::getAndAddNumber(&NumberValue, NumberDelta);
        }

        T getAndAnd(T NumberSource) noexcept {
            return ConcurrentUtility::getAndAndNumber(&NumberValue, NumberSource);
        }

        T getAndDecrement() noexcept {
            return ConcurrentUtility::getAndAddNumber(&NumberValue, (T) -1);
        }

        T getAndIncrement() noexcept {
            return ConcurrentUtility::getAndAddNumber(&NumberValue, (T) 1);
        }

        T getAndOr(T NumberSource) noexcept {
            return ConcurrentUtility::getAndOrNumber(&NumberValue, NumberSource);
        }

        T getAndSet(T ValueTarget) noexcept {
            return ConcurrentUtility::getAndSetNumber(&NumberValue, ValueTarget);
        }

        T getAndXor(T NumberSource) noexcept {
            return ConcurrentUtility::getAndXorNumber(&NumberValue, NumberSource);
        }

        const char *getClassName() const noexcept override {
            return "AtomicNumber";
        }

        T getValue() const noexcept {
            return NumberValue;
        }

        T incrementAndGet() noexcept {
            return getAndIncrement() + 1;
        }

        T orAndGet(T NumberSource) noexcept {
            return ConcurrentUtility::getAndOrNumber(&NumberValue, NumberSource) | NumberSource;
        }

        T xorAndGet(T NumberSource) noexcept {
            return ConcurrentUtility::getAndXorNumber(&NumberValue, NumberSource) ^ NumberSource;
        }

        String toString() const noexcept override {
            return String::valueOf(NumberValue);
        }
    };

    template<>
    class AtomicNumber<bool> {};

    template<>
    class AtomicNumber<int8_t> {};

    template<>
    class AtomicNumber<uint8_t> {};

    template<typename T>
    class AtomicReference final : public Object {
    private:
        volatile T *ObjectValue;
    public:
        bool doCompareAndSet(const T &ObjectExpected, const T &ObjectTarget) noexcept {
            return ConcurrentUtility::doCompareAndSetReference(&ObjectValue, &ObjectExpected, &ObjectTarget);
        }

        T *getAndSet(const T &ObjectTarget) noexcept {
            return ConcurrentUtility::getAndSetReference(&ObjectValue, &ObjectTarget);
        }

        const char *getClassName() const noexcept override {
            return "AtomicReference";
        }

        T *getValue() const noexcept {
            return ObjectValue;
        }

        String toString() const noexcept override {
            return String::valueOf(*ObjectValue);
        }
    };
}

#endif
