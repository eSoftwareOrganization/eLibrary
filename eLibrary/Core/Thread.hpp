#pragma once

#include <Core/Concurrent.hpp>

#if eLibrarySystem(Windows)
#include <Windows.h>
#else
#include <pthread.h>
#include <unistd.h>
#endif

namespace eLibrary::Core {
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
        volatile int8_t ThreadInterrupt;

#if eLibrarySystem(Windows)
        static DWORD doExecuteCore(void *ThreadContext) noexcept {
#else
        static void *doExecuteCore(void *ThreadContext) noexcept {
#endif
            ((Thread *) ThreadContext)->doExecute();
            ((Thread*) ThreadContext)->ThreadFinish = true;
#if eLibrarySystem(Windows)
            return 0;
#else
            return nullptr;
#endif
        }

        constexpr Thread(ThreadHandleType ThreadHandleSource) noexcept : ThreadHandle(ThreadHandleSource), ThreadFinish(false), ThreadInterrupt(false) {}

        doDisableCopyAssignConstruct(Thread)
    public:
        constexpr Thread() noexcept : ThreadHandle(ThreadHandleType()), ThreadFinish(false), ThreadInterrupt(false) {}

        virtual void doExecute() noexcept {}

        void doInterrupt() {
            if (isInterrupted()) throw InterruptedException(String(u"Thread::doInterrupt() isInterrupted"));
            ConcurrentUtility::doCompareAndExchange(&ThreadInterrupt, (int8_t) 0, (int8_t) 1);
        }

        void doJoin() const {
#if eLibrarySystem(Windows)
            while (!ThreadFinish) {
                if (isInterrupted()) throw InterruptedException(String(u"Thread::doJoin() isInterrupted"));
                Sleep(5);
            }
            WaitForSingleObject(ThreadHandle, INFINITE);
#else
            while (!ThreadFinish) {
                if (isInterrupted()) throw InterruptedException(String(u"Thread::doJoin() isInterrupted"));
                usleep(5000);
            }
            pthread_join(ThreadHandle, nullptr);
#endif
        }

        virtual void doStart() {
            if (ThreadHandle) throw Exception(String(u"Thread::doStart() ThreadHandle"));
#if eLibrarySystem(Windows)
            ThreadHandle = ::CreateThread(nullptr, 0, Thread::doExecuteCore, this, 0, nullptr);
#else
            pthread_attr_t ThreadAttribute;
            pthread_attr_init(&ThreadAttribute);
            pthread_attr_setschedpolicy(&ThreadAttribute, SCHED_MAX);
            pthread_create(&ThreadHandle, &ThreadAttribute, Thread::doExecuteCore, this);
            pthread_attr_destroy(&ThreadAttribute);
#endif
        }

        static void doYield() noexcept {
#if eLibrarySystem(Windows)
            SwitchToThread();
#else
            sched_yield();
#endif
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
}
