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
        static void getAndSetNumber(volatile T *ValueAddress, T ValueTarget) noexcept {
            T ValueExpected;
            do {
                ValueExpected = *ValueAddress;
            } while (ConcurrentUtility::doCompareAndExchange(ValueAddress, ValueExpected, ValueTarget) != ValueExpected);
        }
    };

    class AbstractQueuedSynchronizer : public Object {
    public:
        struct AbstractQueuedNode {
            volatile AbstractQueuedNode *NodeNext = nullptr;
            volatile AbstractQueuedNode *NodePrevious = nullptr;

            bool setNodeNextCAS(AbstractQueuedNode *NodeExpected, AbstractQueuedNode *NodeTarget) noexcept {
                return ConcurrentUtility::doCompareAndSetReference(&NodeNext, NodeExpected, NodeTarget);
            }

            bool setNodePreviousCAS(AbstractQueuedNode *NodeExpected, AbstractQueuedNode *NodeTarget) noexcept {
                return ConcurrentUtility::doCompareAndSetReference(&NodePrevious, NodeExpected, NodeTarget);
            }
        };
        volatile AbstractQueuedNode *NodeHead = nullptr, *NodeTail = nullptr;
    public:
        virtual bool tryAcquireExclusive(int) noexcept = 0;

        virtual bool tryAcquireShared(int) noexcept = 0;

        virtual bool tryReleaseExclusive(int) noexcept = 0;

        virtual bool tryReleaseShared(int) noexcept = 0;
    };

    template<std::integral T>
    class AtomicNumber final : public Object {
    private:
        volatile T NumberValue;
    public:
        T addAndGet(T NumberDelta) noexcept {
            return ConcurrentUtility::getAndAddNumber(&NumberValue, NumberDelta) + NumberDelta;
        }

        bool compareAndSet(T ValueExpected, T ValueTarget) noexcept {
            return ConcurrentUtility::doCompareAndSet(&NumberValue, ValueExpected, ValueTarget);
        }

        T decrementAndGet() noexcept {
            return ConcurrentUtility::getAndAddNumber(&NumberValue, -1) - 1;
        }

        T getAndAdd(T NumberDelta) noexcept {
            return ConcurrentUtility::getAndAddNumber(&NumberValue, NumberDelta);
        }

        T getAndDecrement() noexcept {
            return ConcurrentUtility::getAndAddNumber(&NumberValue, -1);
        }

        T getAndIncrement() noexcept {
            return ConcurrentUtility::getAndAddNumber(&NumberValue, 1);
        }

        T getAndSet(T ValueTarget) noexcept {
            return ConcurrentUtility::getAndSetNumber(&NumberValue, ValueTarget);
        }

        T getValue() const noexcept {
            return NumberValue;
        }

        T incrementAndGet() noexcept {
            return ConcurrentUtility::getAndAddNumber(&NumberValue, 1) + 1;
        }

        String toString() const noexcept override {
            return Integer(NumberValue).toString();
        }
    };
}
