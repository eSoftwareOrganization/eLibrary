#pragma once

#ifndef eLibraryHeaderCoreConcurrent
#define eLibraryHeaderCoreConcurrent

#include <Core/Container.hpp>
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

#include <condition_variable>
#include <future>
#include <mutex>
#include <queue>
#include <utility>

namespace eLibrary::Core {
    class ConcurrentUtility final : public Object, public NonConstructable {
    public:
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

        template<SizeEqual<int16_t> T>
        static T doCompareAndExchange(volatile T *ValueAddress, T ValueExpected, T ValueTarget) noexcept {
            return doCompareAndExchange16((volatile int16_t*) ValueAddress, (int16_t) ValueExpected, (int16_t) ValueTarget);
        }

        template<SizeEqual<int32_t> T>
        static T doCompareAndExchange(volatile T *ValueAddress, T ValueExpected, T ValueTarget) noexcept {
            return doCompareAndExchange32((volatile int32_t*) ValueAddress, (int32_t) ValueExpected, (int32_t) ValueTarget);
        }

        template<SizeEqual<int64_t> T>
        static T doCompareAndExchange(volatile T *ValueAddress, T ValueExpected, T ValueTarget) noexcept {
            return doCompareAndExchange64((volatile int64_t*) ValueAddress, (int64_t) ValueExpected, (int64_t) ValueTarget);
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
    class Thread : public Object, public NonCopyable {
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

        [[deprecated]] void doInterrupt() {
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

        [[deprecated]] bool isInterrupted() const noexcept {
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
        ::std::tuple<Ts...> ThreadParameter;
    public:
        constexpr explicit FunctionThread(F ThreadFunctionSource, Ts ...ThreadParameterSource) noexcept : ThreadFunction(ThreadFunctionSource), ThreadParameter(std::make_tuple(ThreadParameterSource...)) {}

        void doExecute() noexcept override {
            ::std::apply(ThreadFunction, ThreadParameter);
        }

        const char *getClassName() const noexcept override {
            return "FunctionThread";
        }
    };

    template <typename T>
    class ConcurrentQueue final : public Object {
    private:
        ::std::queue<T> QueueObject;
        ::std::mutex QueueMutex;
    public:
        bool isEmpty() {
            ::std::unique_lock<::std::mutex> QueueLock(QueueMutex);
            return QueueObject.empty();
        }

        bool doDequeue(T &QueueSource) {
            ::std::unique_lock<::std::mutex> QueueLock(QueueMutex);
            if (QueueObject.empty()) return false;
            QueueSource = Objects::doMove(QueueObject.front());
            QueueObject.pop();
            return true;
        }

        void doEnqueue(T &QueueSource) {
            ::std::unique_lock<::std::mutex> QueueLock(QueueMutex);
            QueueObject.push(QueueSource);
        }
    };

    class ThreadExecutor final : public Object, public NonCopyable, public NonMovable {
    private:
        class ThreadExecutorCore final : public Thread {
        private:
            ThreadExecutor *ExecutorObject;
        public:
            ThreadExecutorCore(ThreadExecutor *ExecutorSource) : ExecutorObject(ExecutorSource) {}

            void doExecute() noexcept override {
                ::std::function<void()> ExecutorFunction;
                bool ExecutorFunctionAvailable;
                while (!ExecutorObject->ExecutorShutdown) {
                    {
                        ::std::unique_lock<::std::mutex> ExecutorLock(ExecutorObject->ExecutorMutex);
                        if (ExecutorObject->ExecutorQueue.isEmpty()) ExecutorObject->ExecutorVariable.wait(ExecutorLock);
                        ExecutorFunctionAvailable = ExecutorObject->ExecutorQueue.doDequeue(ExecutorFunction);
                    }
                    if (ExecutorFunctionAvailable) ExecutorFunction();
                }
            }
        };

        ::std::mutex ExecutorMutex;
        ConcurrentQueue<::std::function<void()>> ExecutorQueue;
        bool ExecutorShutdown = false;
        ::std::vector<ThreadExecutorCore*> ExecutorThread;
        ::std::condition_variable ExecutorVariable;
    public:
        ThreadExecutor(uintmax_t ExecutorThreadCount) {
            ExecutorThread.reserve(ExecutorThreadCount);
            for (uintmax_t ExecutorThreadIndex = 0;ExecutorThreadIndex < ExecutorThreadCount;++ExecutorThreadIndex) {
                ExecutorThread[ExecutorThreadIndex] = new ThreadExecutorCore(this);
                ExecutorThread[ExecutorThreadIndex]->doStart();
            }
        }

        ~ThreadExecutor() {
            doShutdown();
        }

        void doShutdown() noexcept {
            ExecutorShutdown = true;
            ExecutorVariable.notify_all();
            Collections::doTraverse(ExecutorThread.begin(), ExecutorThread.end(), [](ThreadExecutorCore *ExecutorCore){
                ExecutorCore->doJoin();
                delete ExecutorCore;
            });
            ExecutorThread.clear();
        }

        template<typename F, typename ...Ps>
        auto doSubmit(F &&ExecutorFunction, Ps &&...ExecutorFunctionParameter) -> ::std::future<decltype(ExecutorFunction(ExecutorFunctionParameter...))> {
            if (ExecutorShutdown) throw Exception(String(u"ThreadExecutor::doSubmit<F, Ps...>(F&&, Ps&&...) ExecutorShutdown"));
            auto ExecutorTarget(::std::bind(Objects::doForward<F>(ExecutorFunction), Objects::doForward<Ps>(ExecutorFunctionParameter)...));
            auto ExecutorTask(::std::make_shared<::std::packaged_task<decltype(ExecutorTarget(ExecutorFunctionParameter...))()>>(ExecutorTarget));
            ::std::function<void()> ExecutorWrapper = [ExecutorTask] {
                (*ExecutorTask)();
            };
            ExecutorQueue.doEnqueue(ExecutorWrapper);
            ExecutorVariable.notify_one();
            return ExecutorTask->get_future();
        }

        bool isShutdown() const noexcept {
            return ExecutorShutdown;
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
        };
        volatile AbstractQueuedNode *NodeHead = nullptr, *NodeTail = nullptr;
    public:
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
