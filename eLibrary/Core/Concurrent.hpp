#pragma once

#include <Core/Number.hpp>

namespace eLibrary::Core {
    class ConcurrentUtility final : public Object {
    public:
        constexpr ConcurrentUtility() noexcept = delete;

        static int8_t doCompareAndExchange(volatile int8_t *ValueAddress, int8_t ValueExpected, int8_t ValueTarget) noexcept {
            int8_t ValueResult;
            asm volatile("lock\n\tcmpxchgb %2, (%3)":"=a"(ValueResult):"a"(ValueExpected), "r"(ValueTarget), "r"(ValueAddress):"cc", "memory");
            return ValueResult;
        }

        static int16_t doCompareAndExchange(volatile int16_t *ValueAddress, int16_t ValueExpected, int16_t ValueTarget) noexcept {
            int16_t ValueResult;
            asm volatile("lock\n\tcmpxchgw %2, (%3)":"=a"(ValueResult):"a"(ValueExpected), "r"(ValueTarget), "r"(ValueAddress):"cc", "memory");
            return ValueResult;
        }

        static int32_t doCompareAndExchange(volatile int32_t *ValueAddress, int32_t ValueExpected, int32_t ValueTarget) noexcept {
            int32_t ValueResult;
            asm volatile("lock\n\tcmpxchgl %2, (%3)":"=a"(ValueResult):"a"(ValueExpected), "r"(ValueTarget), "r"(ValueAddress):"cc", "memory");
            return ValueResult;
        }

        static int64_t doCompareAndExchange(volatile int64_t *ValueAddress, int64_t ValueExpected, int64_t ValueTarget) noexcept {
            int64_t ValueResult;
            asm volatile("lock\n\tcmpxchgq %2, (%3)":"=a"(ValueResult):"a"(ValueExpected), "r"(ValueTarget), "r"(ValueAddress):"cc", "memory");
            return ValueResult;
        }

        template<typename T>
        static auto doCompareAndExchangeReference(volatile T **ValueAddress, T *ValueExpected, T *ValueTarget) noexcept {
            return doCompareAndExchange((volatile intmax_t*) ValueAddress, (intmax_t) ValueExpected, (intmax_t) ValueTarget);
        }

        template<typename T>
        static bool doCompareAndSet(volatile T *ValueAddress, T ValueExpected, T ValueTarget) noexcept {
            doCompareAndExchange(ValueAddress, ValueExpected, ValueTarget);
            return *ValueAddress == ValueTarget;
        }

        template<typename T>
        static bool doCompareAndSetReference(volatile T **ValueAddress, T *ValueExpected, T *ValueTarget) noexcept {
            doCompareAndExchangeReference(ValueAddress, ValueExpected, ValueTarget);
            return *ValueAddress == ValueTarget;
        }

        template<std::integral T>
        static T getAndAddNumber(volatile T *ValueAddress, T ValueDelta) noexcept {
            T ValueExpected;
            do {
                ValueExpected = *ValueAddress;
            } while (ConcurrentUtility::doCompareAndExchange(ValueAddress, ValueExpected, ValueExpected + ValueDelta) != ValueExpected);
            return ValueExpected;
        }

        template<std::integral T>
        static T getAndAndNumber(volatile T *ValueAddress, T ValueSource) noexcept {
            T ValueExpected;
            do {
                ValueExpected = *ValueAddress;
            } while (ConcurrentUtility::doCompareAndExchange(ValueAddress, ValueExpected, ValueExpected & ValueSource) != ValueExpected);
            return ValueExpected;
        }

        template<std::integral T>
        static T getAndOrNumber(volatile T *ValueAddress, T ValueSource) noexcept {
            T ValueExpected;
            do {
                ValueExpected = *ValueAddress;
            } while (ConcurrentUtility::doCompareAndExchange(ValueAddress, ValueExpected, ValueExpected | ValueSource) != ValueExpected);
            return ValueExpected;
        }

        template<std::integral T>
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

        template<std::integral T>
        static T getAndXorNumber(volatile T *ValueAddress, T ValueSource) noexcept {
            T ValueExpected;
            do {
                ValueExpected = *ValueAddress;
            } while (ConcurrentUtility::doCompareAndExchange(ValueAddress, ValueExpected, ValueExpected ^ ValueSource) != ValueExpected);
            return ValueExpected;
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
                delete NodeHead;
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
            return {std::to_string(NumberValue)};
        }
    };

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

        T *getValue() const noexcept {
            return ObjectValue;
        }

        String toString() const noexcept override {
            return String::valueOf(*ObjectValue);
        }
    };
}
