#pragma once

#include <Core/Exception.hpp>

#include <condition_variable>
#include <pthread.h>
#include <semaphore.h>

namespace eLibrary {
    class Mutex final : public Object {
    private:
        pthread_mutex_t MutexHandle;
    public:
        Mutex() {
            if (pthread_mutex_init(&MutexHandle, nullptr))
                throw Exception(String(u"Mutex::Mutex() pthread_mutex_init"));
        }

        ~Mutex() noexcept {
            pthread_mutex_destroy(&MutexHandle);
        }

        void doLock() {
            if (pthread_mutex_lock(&MutexHandle)) throw Exception(String(u"Mutex::doLock() pthread_mutex_lock"));
        }

        void doUnlock() noexcept {
            pthread_mutex_unlock(&MutexHandle);
        }

        pthread_mutex_t *getHandle() noexcept {
            return &MutexHandle;
        }

        Mutex &operator=(const Mutex&) noexcept = delete;

        bool tryLock() noexcept {
            return !pthread_mutex_trylock(&MutexHandle);
        }
    };

    class MutexExecutor final : public Object {
    public:
        MutexExecutor() noexcept = delete;

        template<typename OperationType>
        static auto doExecute(Mutex &MutexSource, OperationType &&OperationFunction) {
            MutexSource.doLock();
            auto &&OperationResult = OperationFunction();
            MutexSource.doUnlock();
            return std::move(OperationResult);
        }

        template<typename OperationType>
        static void doExecuteVoid(Mutex &MutexSource, OperationType &&OperationFunction) {
            MutexSource.doLock();
            OperationFunction();
            MutexSource.doUnlock();
        }
    };

    class ConditionVariable final : public Object {
    private:
        pthread_cond_t VariableHandle;
    public:
        ConditionVariable() {
            if (pthread_cond_init(&VariableHandle, nullptr))
                throw Exception(String(u"ConditionVariable::ConditionVariable() pthread_cond_init"));
        }

        ~ConditionVariable() noexcept {
            pthread_cond_destroy(&VariableHandle);
        }

        void doWait(Mutex &MutexSource) {
            if (pthread_cond_wait(&VariableHandle, MutexSource.getHandle()))
                throw Exception(String(u"ConditionVariable::doWait(Mutex&) pthread_cond_wait"));
        }

        pthread_cond_t *getHandle() noexcept {
            return &VariableHandle;
        }

        void notifyAll() {
            if (pthread_cond_broadcast(&VariableHandle)) throw Exception(String(u"ConditionVariable::notifyAll() pthread_cond_broadcast"));
        }

        void notifyOne() {
            if (pthread_cond_signal(&VariableHandle)) throw Exception(String(u"ConditionVariable::notifyOne() pthread_cond_signal"));
        }

        ConditionVariable &operator=(const ConditionVariable&) noexcept = delete;
    };

    template<typename E>
    class ConcurrentArrayList final : public Object {
    private:
        ArrayList<E> ElementList;
        mutable Mutex ElementMutex;
    public:
        ConcurrentArrayList(const ArrayList<E> &ElementListSource) : ElementList(ElementListSource) {}

        void addElement(const E &ElementSource) noexcept {
            MutexExecutor::doExecuteVoid(ElementMutex, [&] {
                ElementList.addElement(ElementSource);
            });
        }

        void addElement(intmax_t ElementIndex, const E &ElementSource) {
            MutexExecutor::doExecuteVoid(ElementMutex, [&] {
                ElementList.addElement(ElementIndex, ElementSource);
            });
        }

        void doAssign(const ConcurrentArrayList<E> &ElementSource) noexcept {
            MutexExecutor::doExecuteVoid(ElementMutex, [&] {
                ElementList.doAssign(ElementSource.ElementList);
            });
        }

        void doClear() noexcept {
            MutexExecutor::doExecuteVoid(ElementMutex, [&] {
                ElementList.doClear();
            });
        }

        ConcurrentArrayList<E> doConcat(const ConcurrentArrayList<E> &ElementSource) const noexcept {
            return MutexExecutor::doExecute(ElementMutex, [&] {
                return ElementList.doConcat(ElementSource);
            });
        }

        void doReverse() noexcept {
            MutexExecutor::doExecuteVoid(ElementMutex, [&] {
                ElementList.doReverse();
            });
        }

        E getElement(intmax_t ElementIndex) const {
            return MutexExecutor::doExecute(ElementMutex, [&] {
                return ElementList.getElement(ElementIndex);
            });
        }

        intmax_t getElementSize() {
            return MutexExecutor::doExecute(ElementMutex, [&] {
                return ElementList.getElementSize();
            });
        }

        intmax_t indexOf(const E &ElementSource) const noexcept {
            return MutexExecutor::doExecute(ElementMutex, [&] {
                return ElementList.doFind(ElementSource);
            });
        }

        ConcurrentArrayList<E> &operator=(const ConcurrentArrayList<E> &ElementSource) noexcept {
            doAssign(ElementSource);
            return *this;
        }

        void removeElement(const E &ElementSource) {
            MutexExecutor::doExecuteVoid(ElementMutex, [&] {
                ElementList.removeElement(ElementSource);
            });
        }

        void removeIndex(intmax_t ElementIndex) {
            MutexExecutor::doExecuteVoid(ElementMutex, [&] {
                ElementList.removeIndex(ElementIndex);
            });
        }

        void setElement(intmax_t ElementIndex, const E &ElementSource) {
            MutexExecutor::doExecuteVoid(ElementMutex, [&] {
                ElementList.setElement(ElementIndex, ElementSource);
            });
        }

        auto toArray() const noexcept {
            return MutexExecutor::doExecute(ElementMutex, [&] {
                return ElementList.toArray();
            });
        }

        std::vector<E> toSTLVector() const noexcept {
            return MutexExecutor::doExecute(ElementMutex, [&] {
                return ElementList.toSTLVector();
            });
        }

        String toString() const noexcept override {
            return MutexExecutor::doExecute(ElementMutex, [&] {
                return ElementList.toString();
            });
        }
    };

    class Semaphore final : public Object {
    private:
        sem_t SemaphoreHandle;
    public:
        explicit Semaphore(unsigned SemaphoreValue) {
            if (sem_init(&SemaphoreHandle, 0, SemaphoreValue))
                throw Exception(String(u"Semaphore::Semaphore() sem_init"));
        }

        ~Semaphore() noexcept {
            sem_destroy(&SemaphoreHandle);
        }

        void doAcquire() noexcept {
            sem_wait(&SemaphoreHandle);
        }

        void doRelease() noexcept {
            sem_post(&SemaphoreHandle);
        }

        sem_t *getHandle() noexcept {
            return &SemaphoreHandle;
        }
    };
}
