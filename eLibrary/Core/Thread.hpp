#pragma once

#include <Core/Concurrent.hpp>
#include <Core/Object.hpp>

#include <pthread.h>
#include <unistd.h>

namespace eLibrary::Core {
    class Thread : public Object {
    public:
        enum class ThreadPriorityEnumeration {
            PriorityIdle
        };
    private:
        pthread_t ThreadHandle;
        volatile bool ThreadFinish;
        volatile int8_t ThreadInterrupt;

        static void *doExecuteCore(void *ThreadContext) {
            ((Thread*) ThreadContext)->doExecute();
            ((Thread*) ThreadContext)->ThreadFinish = true;
            return nullptr;
        }

        constexpr Thread(pthread_t ThreadHandleSource) noexcept : ThreadHandle(ThreadHandleSource), ThreadFinish(false), ThreadInterrupt(false) {}

        doDisableCopyAssignConstruct(Thread)
    public:
        constexpr Thread() noexcept : ThreadHandle(pthread_t()), ThreadFinish(false), ThreadInterrupt(false) {}

        virtual void doExecute() {}

        void doInterrupt() {
            if (isInterrupted()) throw InterruptedException(String(u"Thread::doInterrupt() isInterrupted"));
            ConcurrentUtility::doCompareAndExchange(&ThreadInterrupt, (int8_t) 0, (int8_t) 1);
        }

        void doJoin() const {
            while (!ThreadFinish) {
                if (isInterrupted()) throw InterruptedException(String(u"Thread::doJoin() isInterrupted"));
                usleep(5000);
            }
            pthread_join(ThreadHandle, nullptr);
        }

        virtual void doStart() {
            if (ThreadHandle) throw Exception(String(u"Thread::doStart() ThreadHandle"));
            pthread_attr_t ThreadAttribute;
            pthread_attr_init(&ThreadAttribute);
            pthread_attr_setschedpolicy(&ThreadAttribute, SCHED_MAX);
            pthread_create(&ThreadHandle, &ThreadAttribute, Thread::doExecuteCore, this);
            pthread_attr_destroy(&ThreadAttribute);
        }

        static void doYield() noexcept {
            sched_yield();
        }

        bool isFinished() const noexcept {
            return ThreadFinish;
        }

        bool isInterrupted() const noexcept {
            return ThreadInterrupt;
        }

        static Thread getCurrentThread() noexcept {
            return {pthread_self()};
        }
    };
}
