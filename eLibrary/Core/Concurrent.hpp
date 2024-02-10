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
#include <Platform/Windows.hpp>
#else
#include <pthread.h>
#include <unistd.h>
#endif

#if defined(YieldProcessor)
#define doYieldCpu YieldProcessor
#elif __has_builtin(__yield)
#define doYieldCpu __yield
#elif __has_builtin(__builtin_ia32_pause)
#define doYieldCpu __builtin_ia32_pause
#endif

#include <condition_variable>
#include <cstring>
#include <future>
#include <mutex>
#include <queue>
#include <utility>

namespace eLibrary::Core {
    enum class MemoryOrder : int {
        OrderRelaxed = 0,
        OrderSeqCst = 5
    };

    template<typename T, size_t>
    struct ConcurrentOperation {
        template<typename O, typename I>
        static O *doCastAddress(I &AddressSource) noexcept {
            return &reinterpret_cast<O&>(AddressSource);
        }

        template<typename O, typename I>
        static volatile O *doCastAddress(volatile I &AddressSource) noexcept {
            return &reinterpret_cast<volatile O&>(AddressSource);
        }

        template<typename O, typename I>
        static O doCastObject(const I &ObjectSource) noexcept {
            if constexpr ((::std::is_integral_v<I> || ::std::is_pointer_v<I>) && sizeof(O) == sizeof(I))
                return (O) ObjectSource;
            else {
                O ObjectResult{};
                ::memcpy(&ObjectResult, Objects::getAddress(ObjectSource), sizeof(ObjectSource));
                return ObjectResult;
            }
        }
    };

    template<typename T>
    struct ConcurrentOperation<T, 1> {
        using ParameterType = char;

#if eLibraryCompiler(MSVC)
        static T doAddFetch(volatile T &ValueAddress, const T ValueTargetSource, MemoryOrder=MemoryOrder::OrderSeqCst) noexcept {
            auto ValueTarget(ConcurrentOperation<T, 0>::template doCastObject<ParameterType>(ValueTargetSource));
            auto ValueResult(_InterlockedExchangeAdd8(ConcurrentOperation<T, 0>::template doCastAddress<ParameterType>(ValueAddress), ValueTarget) + ValueTarget);
            return reinterpret_cast<T&>(ValueResult);
        }

        static T doAndFetch(volatile T &ValueAddress, const T ValueTargetSource, MemoryOrder=MemoryOrder::OrderSeqCst) noexcept {
            auto ValueTarget(ConcurrentOperation<T, 0>::template doCastObject<ParameterType>(ValueTargetSource));
            auto ValueResult(_InterlockedAnd8(ConcurrentOperation<T, 0>::template doCastAddress<ParameterType>(ValueAddress), ValueTarget) & ValueTarget);
            return reinterpret_cast<T&>(ValueResult);
        }

        static T doFetchAdd(volatile T &ValueAddress, const T ValueTarget, MemoryOrder=MemoryOrder::OrderSeqCst) noexcept {
            auto ValueResult(_InterlockedExchangeAdd8(ConcurrentOperation<T, 0>::template doCastAddress<ParameterType>(ValueAddress), ConcurrentOperation<T, 0>::template doCastObject<ParameterType>(ValueTarget)));
            return reinterpret_cast<T&>(ValueResult);
        }

        static T doFetchAnd(volatile T &ValueAddress, const T ValueTarget, MemoryOrder=MemoryOrder::OrderSeqCst) noexcept {
            auto ValueResult(_InterlockedAnd8(ConcurrentOperation<T, 0>::template doCastAddress<ParameterType>(ValueAddress), ConcurrentOperation<T, 0>::template doCastObject<ParameterType>(ValueTarget)));
            return reinterpret_cast<T&>(ValueResult);
        }

        static T doFetchOr(volatile T &ValueAddress, const T ValueTarget, MemoryOrder=MemoryOrder::OrderSeqCst) noexcept {
            auto ValueResult(_InterlockedOr8(ConcurrentOperation<T, 0>::template doCastAddress<ParameterType>(ValueAddress), ConcurrentOperation<T, 0>::template doCastObject<ParameterType>(ValueTarget)));
            return reinterpret_cast<T&>(ValueResult);
        }

        static T doFetchSub(volatile T &ValueAddress, const T ValueTarget, MemoryOrder=MemoryOrder::OrderSeqCst) noexcept {
            auto ValueResult(_InterlockedExchangeAdd8(ConcurrentOperation<T, 0>::template doCastAddress<ParameterType>(ValueAddress), -ConcurrentOperation<T, 0>::template doCastObject<ParameterType>(ValueTarget)));
            return reinterpret_cast<T&>(ValueResult);
        }

        static T doFetchXor(volatile T &ValueAddress, const T ValueTarget, MemoryOrder=MemoryOrder::OrderSeqCst) noexcept {
            auto ValueResult(_InterlockedXor8(ConcurrentOperation<T, 0>::template doCastAddress<ParameterType>(ValueAddress), ConcurrentOperation<T, 0>::template doCastObject<ParameterType>(ValueTarget)));
            return reinterpret_cast<T&>(ValueResult);
        }

        static bool doCompareExchange(volatile T &ValueAddress, T &ValueExpectedSource, const T ValueTarget, MemoryOrder=MemoryOrder::OrderSeqCst, MemoryOrder=MemoryOrder::OrderSeqCst) noexcept {
            auto ValueExpected(ConcurrentOperation<T, 0>::template doCastObject<ParameterType>(ValueExpectedSource));
            auto ValuePrevious(_InterlockedCompareExchange8(ConcurrentOperation<T, 0>::template doCastAddress<ParameterType>(ValueAddress), ValueExpected, ConcurrentOperation<T, 0>::template doCastObject<ParameterType>(ValueTarget)));
            if (ValuePrevious == ValueExpected) return true;
            reinterpret_cast<ParameterType&>(ValueExpectedSource) = ValuePrevious;
            return false;
        }

        static void doExchange(volatile T &ValueAddress, T ValueTarget, MemoryOrder ValueOrder = MemoryOrder::OrderSeqCst) noexcept {
            switch (ValueOrder) {
                case MemoryOrder::OrderRelaxed:
                    __iso_volatile_store8(ConcurrentOperation<T, 0>::template doCastAddress<ParameterType>(ValueAddress), ConcurrentOperation<T, 0>::template doCastObject<ParameterType>(ValueTarget));
                    break;
                case MemoryOrder::OrderSeqCst:
                    _InterlockedExchange8(ConcurrentOperation<T, 0>::template doCastAddress<ParameterType>(ValueAddress), ConcurrentOperation<T, 0>::template doCastObject<ParameterType>(ValueTarget));
                    break;
            }
        }

        static T doLoad(volatile T &ValueAddress, MemoryOrder ValueOrder = MemoryOrder::OrderSeqCst) noexcept {
            auto ValueResult(__iso_volatile_load8(ConcurrentOperation<T, 0>::template doCastAddress<ParameterType>(ValueAddress)));
            switch (ValueOrder) {
                case MemoryOrder::OrderRelaxed:
                    break;
                case MemoryOrder::OrderSeqCst:
                    _Compiler_or_memory_barrier();
                    break;
            }
            return reinterpret_cast<T&>(ValueResult);
        }

        static T doOrFetch(volatile T &ValueAddress, const T ValueTargetSource, MemoryOrder=MemoryOrder::OrderSeqCst) noexcept {
            auto ValueTarget(ConcurrentOperation<T, 0>::template doCastObject<ParameterType>(ValueTargetSource));
            auto ValueResult(_InterlockedOr8(ConcurrentOperation<T, 0>::template doCastAddress<ParameterType>(ValueAddress), ValueTarget) | ValueTarget);
            return reinterpret_cast<T&>(ValueResult);
        }

        static T doSubFetch(volatile T &ValueAddress, const T ValueTargetSource, MemoryOrder=MemoryOrder::OrderSeqCst) noexcept {
            auto ValueTarget(ConcurrentOperation<T, 0>::template doCastObject<ParameterType>(ValueTargetSource));
            auto ValueResult(_InterlockedExchangeAdd8(ConcurrentOperation<T, 0>::template doCastAddress<ParameterType>(ValueAddress), -ValueTarget) - ValueTarget);
            return reinterpret_cast<T&>(ValueResult);
        }

        static T doXorFetch(volatile T &ValueAddress, const T ValueTargetSource, MemoryOrder=MemoryOrder::OrderSeqCst) noexcept {
            auto ValueTarget(ConcurrentOperation<T, 0>::template doCastObject<ParameterType>(ValueTargetSource));
            auto ValueResult(_InterlockedXor8(ConcurrentOperation<T, 0>::template doCastAddress<ParameterType>(ValueAddress), ValueTarget) ^ ValueTarget);
            return reinterpret_cast<T&>(ValueResult);
        }
#else
        static T doAddFetch(volatile T &ValueAddress, T ValueTarget, MemoryOrder ValueOrder = MemoryOrder::OrderSeqCst) noexcept {
            return __atomic_add_fetch(ConcurrentOperation<T, 0>::template doCastAddress<T>(ValueAddress), ValueTarget, int(ValueOrder));
        }

        static T doAndFetch(volatile T &ValueAddress, T ValueTarget, MemoryOrder ValueOrder = MemoryOrder::OrderSeqCst) noexcept {
            return __atomic_and_fetch(ConcurrentOperation<T, 0>::template doCastAddress<T>(ValueAddress), ValueTarget, int(ValueOrder));
        }

        static T doFetchAdd(volatile T &ValueAddress, T ValueTarget, MemoryOrder ValueOrder = MemoryOrder::OrderSeqCst) noexcept {
            return __atomic_fetch_add(ConcurrentOperation<T, 0>::template doCastAddress<T>(ValueAddress), ValueTarget, int(ValueOrder));
        }

        static T doFetchAnd(volatile T &ValueAddress, T ValueTarget, MemoryOrder ValueOrder = MemoryOrder::OrderSeqCst) noexcept {
            return __atomic_fetch_and(ConcurrentOperation<T, 0>::template doCastAddress<T>(ValueAddress), ValueTarget, int(ValueOrder));
        }

        static T doFetchOr(volatile T &ValueAddress, T ValueTarget, MemoryOrder ValueOrder = MemoryOrder::OrderSeqCst) noexcept {
            return __atomic_fetch_or(ConcurrentOperation<T, 0>::template doCastAddress<T>(ValueAddress), ValueTarget, int(ValueOrder));
        }

        static T doFetchSub(volatile T &ValueAddress, T ValueTarget, MemoryOrder ValueOrder = MemoryOrder::OrderSeqCst) noexcept {
            return __atomic_fetch_sub(ConcurrentOperation<T, 0>::template doCastAddress<T>(ValueAddress), ValueTarget, int(ValueOrder));
        }

        static T doFetchXor(volatile T &ValueAddress, T ValueTarget, MemoryOrder ValueOrder = MemoryOrder::OrderSeqCst) noexcept {
            return __atomic_fetch_xor(ConcurrentOperation<T, 0>::template doCastAddress<T>(ValueAddress), ValueTarget, int(ValueOrder));
        }

        static bool doCompareExchange(volatile T &ValueAddress, T &ValueExpected, const T ValueTarget, MemoryOrder ValueOrder1 = MemoryOrder::OrderSeqCst, MemoryOrder ValueOrder2 = MemoryOrder::OrderSeqCst) noexcept {
            return __atomic_compare_exchange_n(ConcurrentOperation<T, 0>::template doCastAddress<T>(ValueAddress), ConcurrentOperation<T, 0>::template doCastAddress<T>(ValueExpected), ValueTarget, false, int(ValueOrder1), int(ValueOrder2));
        }

        static void doExchange(volatile T &ValueAddress, const T ValueTarget, MemoryOrder ValueOrder = MemoryOrder::OrderSeqCst) noexcept {
            __atomic_store_n(ConcurrentOperation<T, 0>::template doCastAddress<ParameterType>(ValueAddress), ConcurrentOperation<T, 0>::template doCastObject<ParameterType>(ValueTarget), int(ValueOrder));
        }

        static T doLoad(volatile T &ValueAddress, MemoryOrder ValueOrder = MemoryOrder::OrderSeqCst) noexcept {
            return __atomic_load_n(ConcurrentOperation<T, 0>::template doCastAddress<ParameterType>(ValueAddress), int(ValueOrder));
        }

        static T doOrFetch(volatile T &ValueAddress, T ValueTarget, MemoryOrder ValueOrder = MemoryOrder::OrderSeqCst) noexcept {
            return __atomic_or_fetch(ConcurrentOperation<T, 0>::template doCastAddress<T>(ValueAddress), ValueTarget, int(ValueOrder));
        }

        static T doSubFetch(volatile T &ValueAddress, T ValueTarget, MemoryOrder ValueOrder = MemoryOrder::OrderSeqCst) noexcept {
            return __atomic_sub_fetch(ConcurrentOperation<T, 0>::template doCastAddress<T>(ValueAddress), ValueTarget, int(ValueOrder));
        }

        static T doXorFetch(volatile T &ValueAddress, T ValueTarget, MemoryOrder ValueOrder = MemoryOrder::OrderSeqCst) noexcept {
            return __atomic_xor_fetch(ConcurrentOperation<T, 0>::template doCastAddress<T>(ValueAddress), ValueTarget, int(ValueOrder));
        }
#endif
    };

    template<typename T>
    struct ConcurrentOperation<T, 2> {
        using ParameterType = short;

#if eLibraryCompiler(MSVC)
        static T doAddFetch(volatile T &ValueAddress, const T ValueTargetSource, MemoryOrder=MemoryOrder::OrderSeqCst) noexcept {
            auto ValueTarget(ConcurrentOperation<T, 0>::template doCastObject<ParameterType>(ValueTargetSource));
            auto ValueResult(_InterlockedExchangeAdd16(ConcurrentOperation<T, 0>::template doCastAddress<ParameterType>(ValueAddress), ValueTarget) + ValueTarget);
            return reinterpret_cast<T&>(ValueResult);
        }

        static T doAndFetch(volatile T &ValueAddress, const T ValueTargetSource, MemoryOrder=MemoryOrder::OrderSeqCst) noexcept {
            auto ValueTarget(ConcurrentOperation<T, 0>::template doCastObject<ParameterType>(ValueTargetSource));
            auto ValueResult(_InterlockedAnd16(ConcurrentOperation<T, 0>::template doCastAddress<ParameterType>(ValueAddress), ValueTarget) & ValueTarget);
            return reinterpret_cast<T&>(ValueResult);
        }

        static T doFetchAdd(volatile T &ValueAddress, const T ValueTarget, MemoryOrder=MemoryOrder::OrderSeqCst) noexcept {
            auto ValueResult(_InterlockedExchangeAdd16(ConcurrentOperation<T, 0>::template doCastAddress<ParameterType>(ValueAddress), ConcurrentOperation<T, 0>::template doCastObject<ParameterType>(ValueTarget)));
            return reinterpret_cast<T&>(ValueResult);
        }

        static T doFetchAnd(volatile T &ValueAddress, const T ValueTarget, MemoryOrder=MemoryOrder::OrderSeqCst) noexcept {
            auto ValueResult(_InterlockedAnd16(ConcurrentOperation<T, 0>::template doCastAddress<ParameterType>(ValueAddress), ConcurrentOperation<T, 0>::template doCastObject<ParameterType>(ValueTarget)));
            return reinterpret_cast<T&>(ValueResult);
        }

        static T doFetchOr(volatile T &ValueAddress, const T ValueTarget, MemoryOrder=MemoryOrder::OrderSeqCst) noexcept {
            auto ValueResult(_InterlockedOr16(ConcurrentOperation<T, 0>::template doCastAddress<ParameterType>(ValueAddress), ConcurrentOperation<T, 0>::template doCastObject<ParameterType>(ValueTarget)));
            return reinterpret_cast<T&>(ValueResult);
        }

        static T doFetchSub(volatile T &ValueAddress, const T ValueTarget, MemoryOrder=MemoryOrder::OrderSeqCst) noexcept {
            auto ValueResult(_InterlockedExchangeAdd16(ConcurrentOperation<T, 0>::template doCastAddress<ParameterType>(ValueAddress), -ConcurrentOperation<T, 0>::template doCastObject<ParameterType>(ValueTarget)));
            return reinterpret_cast<T&>(ValueResult);
        }

        static T doFetchXor(volatile T &ValueAddress, const T ValueTarget, MemoryOrder=MemoryOrder::OrderSeqCst) noexcept {
            auto ValueResult(_InterlockedXor16(ConcurrentOperation<T, 0>::template doCastAddress<ParameterType>(ValueAddress), ConcurrentOperation<T, 0>::template doCastObject<ParameterType>(ValueTarget)));
            return reinterpret_cast<T&>(ValueResult);
        }

        static bool doCompareExchange(volatile T &ValueAddress, T &ValueExpectedSource, const T ValueTarget, MemoryOrder=MemoryOrder::OrderSeqCst, MemoryOrder=MemoryOrder::OrderSeqCst) noexcept {
            auto ValueExpected(ConcurrentOperation<T, 0>::template doCastObject<ParameterType>(ValueExpectedSource));
            auto ValuePrevious(_InterlockedCompareExchange16(ConcurrentOperation<T, 0>::template doCastAddress<ParameterType>(ValueAddress), ValueExpected, ConcurrentOperation<T, 0>::template doCastObject<ParameterType>(ValueTarget)));
            if (ValuePrevious == ValueExpected) return true;
            reinterpret_cast<ParameterType&>(ValueExpectedSource) = ValuePrevious;
            return false;
        }

        static void doExchange(volatile T &ValueAddress, T ValueTarget, MemoryOrder ValueOrder = MemoryOrder::OrderSeqCst) noexcept {
            switch (ValueOrder) {
                case MemoryOrder::OrderRelaxed:
                    __iso_volatile_store16(ConcurrentOperation<T, 0>::template doCastAddress<ParameterType>(ValueAddress), ConcurrentOperation<T, 0>::template doCastObject<ParameterType>(ValueTarget));
                    break;
                case MemoryOrder::OrderSeqCst:
                    _InterlockedExchange16(ConcurrentOperation<T, 0>::template doCastAddress<ParameterType>(ValueAddress), ConcurrentOperation<T, 0>::template doCastObject<ParameterType>(ValueTarget));
                    break;
            }
        }

        static T doLoad(volatile T &ValueAddress, MemoryOrder ValueOrder = MemoryOrder::OrderSeqCst) noexcept {
            auto ValueResult(__iso_volatile_load16(ConcurrentOperation<T, 0>::template doCastAddress<ParameterType>(ValueAddress)));
            switch (ValueOrder) {
                case MemoryOrder::OrderRelaxed:
                    break;
                case MemoryOrder::OrderSeqCst:
                    _Compiler_or_memory_barrier();
                    break;
            }
            return reinterpret_cast<T&>(ValueResult);
        }

        static T doOrFetch(volatile T &ValueAddress, const T ValueTargetSource, MemoryOrder=MemoryOrder::OrderSeqCst) noexcept {
            auto ValueTarget(ConcurrentOperation<T, 0>::template doCastObject<ParameterType>(ValueTargetSource));
            auto ValueResult(_InterlockedOr16(ConcurrentOperation<T, 0>::template doCastAddress<ParameterType>(ValueAddress), ValueTarget) | ValueTarget);
            return reinterpret_cast<T&>(ValueResult);
        }

        static T doSubFetch(volatile T &ValueAddress, const T ValueTargetSource, MemoryOrder=MemoryOrder::OrderSeqCst) noexcept {
            auto ValueTarget(ConcurrentOperation<T, 0>::template doCastObject<ParameterType>(ValueTargetSource));
            auto ValueResult(_InterlockedExchangeAdd16(ConcurrentOperation<T, 0>::template doCastAddress<ParameterType>(ValueAddress), -ValueTarget) - ValueTarget);
            return reinterpret_cast<T&>(ValueResult);
        }

        static T doXorFetch(volatile T &ValueAddress, const T ValueTargetSource, MemoryOrder=MemoryOrder::OrderSeqCst) noexcept {
            auto ValueTarget(ConcurrentOperation<T, 0>::template doCastObject<ParameterType>(ValueTargetSource));
            auto ValueResult(_InterlockedXor16(ConcurrentOperation<T, 0>::template doCastAddress<ParameterType>(ValueAddress), ValueTarget) ^ ValueTarget);
            return reinterpret_cast<T&>(ValueResult);
        }
#else
        static T doAddFetch(volatile T &ValueAddress, T ValueTarget, MemoryOrder ValueOrder = MemoryOrder::OrderSeqCst) noexcept {
            return __atomic_add_fetch(ConcurrentOperation<T, 0>::template doCastAddress<T>(ValueAddress), ValueTarget, int(ValueOrder));
        }

        static T doAndFetch(volatile T &ValueAddress, T ValueTarget, MemoryOrder ValueOrder = MemoryOrder::OrderSeqCst) noexcept {
            return __atomic_and_fetch(ConcurrentOperation<T, 0>::template doCastAddress<T>(ValueAddress), ValueTarget, int(ValueOrder));
        }

        static T doFetchAdd(volatile T &ValueAddress, T ValueTarget, MemoryOrder ValueOrder = MemoryOrder::OrderSeqCst) noexcept {
            return __atomic_fetch_add(ConcurrentOperation<T, 0>::template doCastAddress<T>(ValueAddress), ValueTarget, int(ValueOrder));
        }

        static T doFetchAnd(volatile T &ValueAddress, T ValueTarget, MemoryOrder ValueOrder = MemoryOrder::OrderSeqCst) noexcept {
            return __atomic_fetch_and(ConcurrentOperation<T, 0>::template doCastAddress<T>(ValueAddress), ValueTarget, int(ValueOrder));
        }

        static T doFetchOr(volatile T &ValueAddress, T ValueTarget, MemoryOrder ValueOrder = MemoryOrder::OrderSeqCst) noexcept {
            return __atomic_fetch_or(ConcurrentOperation<T, 0>::template doCastAddress<T>(ValueAddress), ValueTarget, int(ValueOrder));
        }

        static T doFetchSub(volatile T &ValueAddress, T ValueTarget, MemoryOrder ValueOrder = MemoryOrder::OrderSeqCst) noexcept {
            return __atomic_fetch_sub(ConcurrentOperation<T, 0>::template doCastAddress<T>(ValueAddress), ValueTarget, int(ValueOrder));
        }

        static T doFetchXor(volatile T &ValueAddress, T ValueTarget, MemoryOrder ValueOrder = MemoryOrder::OrderSeqCst) noexcept {
            return __atomic_fetch_xor(ConcurrentOperation<T, 0>::template doCastAddress<T>(ValueAddress), ValueTarget, int(ValueOrder));
        }

        static bool doCompareExchange(volatile T &ValueAddress, T &ValueExpected, const T ValueTarget, MemoryOrder ValueOrder1 = MemoryOrder::OrderSeqCst, MemoryOrder ValueOrder2 = MemoryOrder::OrderSeqCst) noexcept {
            return __atomic_compare_exchange_n(ConcurrentOperation<T, 0>::template doCastAddress<T>(ValueAddress), ConcurrentOperation<T, 0>::template doCastAddress<T>(ValueExpected), ValueTarget, false, int(ValueOrder1), int(ValueOrder2));
        }

        static void doExchange(volatile T &ValueAddress, const T ValueTarget, MemoryOrder ValueOrder = MemoryOrder::OrderSeqCst) noexcept {
            __atomic_store_n(ConcurrentOperation<T, 0>::template doCastAddress<ParameterType>(ValueAddress), ConcurrentOperation<T, 0>::template doCastObject<ParameterType>(ValueTarget), int(ValueOrder));
        }

        static T doLoad(volatile T &ValueAddress, MemoryOrder ValueOrder = MemoryOrder::OrderSeqCst) noexcept {
            return __atomic_load_n(ConcurrentOperation<T, 0>::template doCastAddress<ParameterType>(ValueAddress), int(ValueOrder));
        }

        static T doOrFetch(volatile T &ValueAddress, T ValueTarget, MemoryOrder ValueOrder = MemoryOrder::OrderSeqCst) noexcept {
            return __atomic_or_fetch(ConcurrentOperation<T, 0>::template doCastAddress<T>(ValueAddress), ValueTarget, int(ValueOrder));
        }

        static T doSubFetch(volatile T &ValueAddress, T ValueTarget, MemoryOrder ValueOrder = MemoryOrder::OrderSeqCst) noexcept {
            return __atomic_sub_fetch(ConcurrentOperation<T, 0>::template doCastAddress<T>(ValueAddress), ValueTarget, int(ValueOrder));
        }

        static T doXorFetch(volatile T &ValueAddress, T ValueTarget, MemoryOrder ValueOrder = MemoryOrder::OrderSeqCst) noexcept {
            return __atomic_xor_fetch(ConcurrentOperation<T, 0>::template doCastAddress<T>(ValueAddress), ValueTarget, int(ValueOrder));
        }
#endif
    };

    template<typename T>
    struct ConcurrentOperation<T, 4> {
        using ParameterType = long;

#if eLibraryCompiler(MSVC)
        static T doAddFetch(volatile T &ValueAddress, const T ValueTargetSource, MemoryOrder=MemoryOrder::OrderSeqCst) noexcept {
            auto ValueTarget(ConcurrentOperation<T, 0>::template doCastObject<ParameterType>(ValueTargetSource));
            auto ValueResult(_InterlockedExchangeAdd(ConcurrentOperation<T, 0>::template doCastAddress<ParameterType>(ValueAddress), ValueTarget) + ValueTarget);
            return reinterpret_cast<T&>(ValueResult);
        }

        static T doAndFetch(volatile T &ValueAddress, const T ValueTargetSource, MemoryOrder=MemoryOrder::OrderSeqCst) noexcept {
            auto ValueTarget(ConcurrentOperation<T, 0>::template doCastObject<ParameterType>(ValueTargetSource));
            auto ValueResult(_InterlockedAnd(ConcurrentOperation<T, 0>::template doCastAddress<ParameterType>(ValueAddress), ValueTarget) & ValueTarget);
            return reinterpret_cast<T&>(ValueResult);
        }

        static T doFetchAdd(volatile T &ValueAddress, const T ValueTarget, MemoryOrder=MemoryOrder::OrderSeqCst) noexcept {
            auto ValueResult(_InterlockedExchangeAdd(ConcurrentOperation<T, 0>::template doCastAddress<ParameterType>(ValueAddress), ConcurrentOperation<T, 0>::template doCastObject<ParameterType>(ValueTarget)));
            return reinterpret_cast<T&>(ValueResult);
        }

        static T doFetchAnd(volatile T &ValueAddress, const T ValueTarget, MemoryOrder=MemoryOrder::OrderSeqCst) noexcept {
            auto ValueResult(_InterlockedAnd(ConcurrentOperation<T, 0>::template doCastAddress<ParameterType>(ValueAddress), ConcurrentOperation<T, 0>::template doCastObject<ParameterType>(ValueTarget)));
            return reinterpret_cast<T&>(ValueResult);
        }

        static T doFetchOr(volatile T &ValueAddress, const T ValueTarget, MemoryOrder=MemoryOrder::OrderSeqCst) noexcept {
            auto ValueResult(_InterlockedOr(ConcurrentOperation<T, 0>::template doCastAddress<ParameterType>(ValueAddress), ConcurrentOperation<T, 0>::template doCastObject<ParameterType>(ValueTarget)));
            return reinterpret_cast<T&>(ValueResult);
        }

        static T doFetchSub(volatile T &ValueAddress, const T ValueTarget, MemoryOrder=MemoryOrder::OrderSeqCst) noexcept {
            auto ValueResult(_InterlockedExchangeAdd(ConcurrentOperation<T, 0>::template doCastAddress<ParameterType>(ValueAddress), -ConcurrentOperation<T, 0>::template doCastObject<ParameterType>(ValueTarget)));
            return reinterpret_cast<T&>(ValueResult);
        }

        static T doFetchXor(volatile T &ValueAddress, const T ValueTarget, MemoryOrder=MemoryOrder::OrderSeqCst) noexcept {
            auto ValueResult(_InterlockedXor(ConcurrentOperation<T, 0>::template doCastAddress<ParameterType>(ValueAddress), ConcurrentOperation<T, 0>::template doCastObject<ParameterType>(ValueTarget)));
            return reinterpret_cast<T&>(ValueResult);
        }

        static bool doCompareExchange(volatile T &ValueAddress, T &ValueExpectedSource, const T ValueTarget, MemoryOrder=MemoryOrder::OrderSeqCst, MemoryOrder=MemoryOrder::OrderSeqCst) noexcept {
            auto ValueExpected(ConcurrentOperation<T, 0>::template doCastObject<ParameterType>(ValueExpectedSource));
            auto ValuePrevious(_InterlockedCompareExchange(ConcurrentOperation<T, 0>::template doCastAddress<ParameterType>(ValueAddress), ValueExpected, ConcurrentOperation<T, 0>::template doCastObject<ParameterType>(ValueTarget)));
            if (ValuePrevious == ValueExpected) return true;
            reinterpret_cast<ParameterType&>(ValueExpectedSource) = ValuePrevious;
            return false;
        }

        static void doExchange(volatile T &ValueAddress, T ValueTarget, MemoryOrder ValueOrder = MemoryOrder::OrderSeqCst) noexcept {
            switch (ValueOrder) {
                case MemoryOrder::OrderRelaxed:
                    __iso_volatile_store32((volatile int*) ConcurrentOperation<T, 0>::template doCastAddress<ParameterType>(ValueAddress), (int) ConcurrentOperation<T, 0>::template doCastObject<ParameterType>(ValueTarget));
                    break;
                case MemoryOrder::OrderSeqCst:
                    _InterlockedExchange(ConcurrentOperation<T, 0>::template doCastAddress<ParameterType>(ValueAddress), ConcurrentOperation<T, 0>::template doCastObject<ParameterType>(ValueTarget));
                    break;
            }
        }

        static T doLoad(volatile T &ValueAddress, MemoryOrder ValueOrder = MemoryOrder::OrderSeqCst) noexcept {
            auto ValueResult(__iso_volatile_load32((volatile int*) ConcurrentOperation<T, 0>::template doCastAddress<ParameterType>(ValueAddress)));
            switch (ValueOrder) {
                case MemoryOrder::OrderRelaxed:
                    break;
                case MemoryOrder::OrderSeqCst:
                    _Compiler_or_memory_barrier();
                    break;
            }
            return reinterpret_cast<T&>(ValueResult);
        }

        static T doOrFetch(volatile T &ValueAddress, const T ValueTargetSource, MemoryOrder=MemoryOrder::OrderSeqCst) noexcept {
            auto ValueTarget(ConcurrentOperation<T, 0>::template doCastObject<ParameterType>(ValueTargetSource));
            auto ValueResult(_InterlockedOr(ConcurrentOperation<T, 0>::template doCastAddress<ParameterType>(ValueAddress), ValueTarget) | ValueTarget);
            return reinterpret_cast<T&>(ValueResult);
        }

        static T doSubFetch(volatile T &ValueAddress, const T ValueTargetSource, MemoryOrder=MemoryOrder::OrderSeqCst) noexcept {
            auto ValueTarget(ConcurrentOperation<T, 0>::template doCastObject<ParameterType>(ValueTargetSource));
            auto ValueResult(_InterlockedExchangeAdd(ConcurrentOperation<T, 0>::template doCastAddress<ParameterType>(ValueAddress), -ValueTarget) - ValueTarget);
            return reinterpret_cast<T&>(ValueResult);
        }

        static T doXorFetch(volatile T &ValueAddress, const T ValueTargetSource, MemoryOrder=MemoryOrder::OrderSeqCst) noexcept {
            auto ValueTarget(ConcurrentOperation<T, 0>::template doCastObject<ParameterType>(ValueTargetSource));
            auto ValueResult(_InterlockedXor(ConcurrentOperation<T, 0>::template doCastAddress<ParameterType>(ValueAddress), ValueTarget) ^ ValueTarget);
            return reinterpret_cast<T&>(ValueResult);
        }
#else
        static T doAddFetch(volatile T &ValueAddress, T ValueTarget, MemoryOrder ValueOrder = MemoryOrder::OrderSeqCst) noexcept {
            return __atomic_add_fetch(ConcurrentOperation<T, 0>::template doCastAddress<T>(ValueAddress), ValueTarget, int(ValueOrder));
        }

        static T doAndFetch(volatile T &ValueAddress, T ValueTarget, MemoryOrder ValueOrder = MemoryOrder::OrderSeqCst) noexcept {
            return __atomic_and_fetch(ConcurrentOperation<T, 0>::template doCastAddress<T>(ValueAddress), ValueTarget, int(ValueOrder));
        }

        static T doFetchAdd(volatile T &ValueAddress, T ValueTarget, MemoryOrder ValueOrder = MemoryOrder::OrderSeqCst) noexcept {
            return __atomic_fetch_add(ConcurrentOperation<T, 0>::template doCastAddress<T>(ValueAddress), ValueTarget, int(ValueOrder));
        }

        static T doFetchAnd(volatile T &ValueAddress, T ValueTarget, MemoryOrder ValueOrder = MemoryOrder::OrderSeqCst) noexcept {
            return __atomic_fetch_and(ConcurrentOperation<T, 0>::template doCastAddress<T>(ValueAddress), ValueTarget, int(ValueOrder));
        }

        static T doFetchOr(volatile T &ValueAddress, T ValueTarget, MemoryOrder ValueOrder = MemoryOrder::OrderSeqCst) noexcept {
            return __atomic_fetch_or(ConcurrentOperation<T, 0>::template doCastAddress<T>(ValueAddress), ValueTarget, int(ValueOrder));
        }

        static T doFetchSub(volatile T &ValueAddress, T ValueTarget, MemoryOrder ValueOrder = MemoryOrder::OrderSeqCst) noexcept {
            return __atomic_fetch_sub(ConcurrentOperation<T, 0>::template doCastAddress<T>(ValueAddress), ValueTarget, int(ValueOrder));
        }

        static T doFetchXor(volatile T &ValueAddress, T ValueTarget, MemoryOrder ValueOrder = MemoryOrder::OrderSeqCst) noexcept {
            return __atomic_fetch_xor(ConcurrentOperation<T, 0>::template doCastAddress<T>(ValueAddress), ValueTarget, int(ValueOrder));
        }

        static bool doCompareExchange(volatile T &ValueAddress, T &ValueExpected, const T ValueTarget, MemoryOrder ValueOrder1 = MemoryOrder::OrderSeqCst, MemoryOrder ValueOrder2 = MemoryOrder::OrderSeqCst) noexcept {
            return __atomic_compare_exchange_n(ConcurrentOperation<T, 0>::template doCastAddress<T>(ValueAddress), ConcurrentOperation<T, 0>::template doCastAddress<T>(ValueExpected), ValueTarget, false, int(ValueOrder1), int(ValueOrder2));
        }

        static void doExchange(volatile T &ValueAddress, const T ValueTarget, MemoryOrder ValueOrder = MemoryOrder::OrderSeqCst) noexcept {
            __atomic_store_n(ConcurrentOperation<T, 0>::template doCastAddress<ParameterType>(ValueAddress), ConcurrentOperation<T, 0>::template doCastObject<ParameterType>(ValueTarget), int(ValueOrder));
        }

        static T doLoad(volatile T &ValueAddress, MemoryOrder ValueOrder = MemoryOrder::OrderSeqCst) noexcept {
            return __atomic_load_n(ConcurrentOperation<T, 0>::template doCastAddress<ParameterType>(ValueAddress), int(ValueOrder));
        }

        static T doOrFetch(volatile T &ValueAddress, T ValueTarget, MemoryOrder ValueOrder = MemoryOrder::OrderSeqCst) noexcept {
            return __atomic_or_fetch(ConcurrentOperation<T, 0>::template doCastAddress<T>(ValueAddress), ValueTarget, int(ValueOrder));
        }

        static T doSubFetch(volatile T &ValueAddress, T ValueTarget, MemoryOrder ValueOrder = MemoryOrder::OrderSeqCst) noexcept {
            return __atomic_sub_fetch(ConcurrentOperation<T, 0>::template doCastAddress<T>(ValueAddress), ValueTarget, int(ValueOrder));
        }

        static T doXorFetch(volatile T &ValueAddress, T ValueTarget, MemoryOrder ValueOrder = MemoryOrder::OrderSeqCst) noexcept {
            return __atomic_xor_fetch(ConcurrentOperation<T, 0>::template doCastAddress<T>(ValueAddress), ValueTarget, int(ValueOrder));
        }
#endif
    };

    template<typename T>
    struct ConcurrentOperation<T, 8> {
        using ParameterType = long long;

#if eLibraryCompiler(MSVC)
        static T doAddFetch(volatile T &ValueAddress, const T ValueTargetSource, MemoryOrder=MemoryOrder::OrderSeqCst) noexcept {
            auto ValueTarget(ConcurrentOperation<T, 0>::template doCastObject<ParameterType>(ValueTargetSource));
            auto ValueResult(_InterlockedExchangeAdd64(ConcurrentOperation<T, 0>::template doCastAddress<ParameterType>(ValueAddress), ValueTarget) + ValueTarget);
            return reinterpret_cast<T&>(ValueResult);
        }

        static T doAndFetch(volatile T &ValueAddress, const T ValueTargetSource, MemoryOrder=MemoryOrder::OrderSeqCst) noexcept {
            auto ValueTarget(ConcurrentOperation<T, 0>::template doCastObject<ParameterType>(ValueTargetSource));
            auto ValueResult(_InterlockedAnd64(ConcurrentOperation<T, 0>::template doCastAddress<ParameterType>(ValueAddress), ValueTarget) & ValueTarget);
            return reinterpret_cast<T&>(ValueResult);
        }

        static T doFetchAdd(volatile T &ValueAddress, const T ValueTarget, MemoryOrder=MemoryOrder::OrderSeqCst) noexcept {
            auto ValueResult(_InterlockedExchangeAdd64(ConcurrentOperation<T, 0>::template doCastAddress<ParameterType>(ValueAddress), ConcurrentOperation<T, 0>::template doCastObject<ParameterType>(ValueTarget)));
            return reinterpret_cast<T&>(ValueResult);
        }

        static T doFetchAnd(volatile T &ValueAddress, const T ValueTarget, MemoryOrder=MemoryOrder::OrderSeqCst) noexcept {
            auto ValueResult(_InterlockedAnd64(ConcurrentOperation<T, 0>::template doCastAddress<ParameterType>(ValueAddress), ConcurrentOperation<T, 0>::template doCastObject<ParameterType>(ValueTarget)));
            return reinterpret_cast<T&>(ValueResult);
        }

        static T doFetchOr(volatile T &ValueAddress, const T ValueTarget, MemoryOrder=MemoryOrder::OrderSeqCst) noexcept {
            auto ValueResult(_InterlockedOr64(ConcurrentOperation<T, 0>::template doCastAddress<ParameterType>(ValueAddress), ConcurrentOperation<T, 0>::template doCastObject<ParameterType>(ValueTarget)));
            return reinterpret_cast<T&>(ValueResult);
        }

        static T doFetchSub(volatile T &ValueAddress, const T ValueTarget, MemoryOrder=MemoryOrder::OrderSeqCst) noexcept {
            auto ValueResult(_InterlockedExchangeAdd64(ConcurrentOperation<T, 0>::template doCastAddress<ParameterType>(ValueAddress), -ConcurrentOperation<T, 0>::template doCastObject<ParameterType>(ValueTarget)));
            return reinterpret_cast<T&>(ValueResult);
        }

        static T doFetchXor(volatile T &ValueAddress, const T ValueTarget, MemoryOrder=MemoryOrder::OrderSeqCst) noexcept {
            auto ValueResult(_InterlockedXor64(ConcurrentOperation<T, 0>::template doCastAddress<ParameterType>(ValueAddress), ConcurrentOperation<T, 0>::template doCastObject<ParameterType>(ValueTarget)));
            return reinterpret_cast<T&>(ValueResult);
        }

        static bool doCompareExchange(volatile T &ValueAddress, T &ValueExpectedSource, const T ValueTarget, MemoryOrder=MemoryOrder::OrderSeqCst, MemoryOrder=MemoryOrder::OrderSeqCst) noexcept {
            auto ValueExpected(ConcurrentOperation<T, 0>::template doCastObject<ParameterType>(ValueExpectedSource));
            auto ValuePrevious(_InterlockedCompareExchange64(ConcurrentOperation<T, 0>::template doCastAddress<ParameterType>(ValueAddress), ValueExpected, ConcurrentOperation<T, 0>::template doCastObject<ParameterType>(ValueTarget)));
            if (ValuePrevious == ValueExpected) return true;
            reinterpret_cast<ParameterType&>(ValueExpectedSource) = ValuePrevious;
            return false;
        }

        static void doExchange(volatile T &ValueAddress, T ValueTarget, MemoryOrder ValueOrder = MemoryOrder::OrderSeqCst) noexcept {
            switch (ValueOrder) {
                case MemoryOrder::OrderRelaxed:
                    __iso_volatile_store64(ConcurrentOperation<T, 0>::template doCastAddress<ParameterType>(ValueAddress), ConcurrentOperation<T, 0>::template doCastObject<ParameterType>(ValueTarget));
                    break;
                case MemoryOrder::OrderSeqCst:
                    _InterlockedExchange64(ConcurrentOperation<T, 0>::template doCastAddress<ParameterType>(ValueAddress), ConcurrentOperation<T, 0>::template doCastObject<ParameterType>(ValueTarget));
                    break;
            }
        }

        static T doLoad(volatile T &ValueAddress, MemoryOrder ValueOrder = MemoryOrder::OrderSeqCst) noexcept {
            auto ValueResult(__iso_volatile_load64(ConcurrentOperation<T, 0>::template doCastAddress<ParameterType>(ValueAddress)));
            switch (ValueOrder) {
                case MemoryOrder::OrderRelaxed:
                    break;
                case MemoryOrder::OrderSeqCst:
                    _Compiler_or_memory_barrier();
                    break;
            }
            return reinterpret_cast<T&>(ValueResult);
        }

        static T doOrFetch(volatile T &ValueAddress, const T ValueTargetSource, MemoryOrder=MemoryOrder::OrderSeqCst) noexcept {
            auto ValueTarget(ConcurrentOperation<T, 0>::template doCastObject<ParameterType>(ValueTargetSource));
            auto ValueResult(_InterlockedOr64(ConcurrentOperation<T, 0>::template doCastAddress<ParameterType>(ValueAddress), ValueTarget) | ValueTarget);
            return reinterpret_cast<T&>(ValueResult);
        }

        static T doSubFetch(volatile T &ValueAddress, const T ValueTargetSource, MemoryOrder=MemoryOrder::OrderSeqCst) noexcept {
            auto ValueTarget(ConcurrentOperation<T, 0>::template doCastObject<ParameterType>(ValueTargetSource));
            auto ValueResult(_InterlockedExchangeAdd64(ConcurrentOperation<T, 0>::template doCastAddress<ParameterType>(ValueAddress), -ValueTarget) - ValueTarget);
            return reinterpret_cast<T&>(ValueResult);
        }

        static T doXorFetch(volatile T &ValueAddress, const T ValueTargetSource, MemoryOrder=MemoryOrder::OrderSeqCst) noexcept {
            auto ValueTarget(ConcurrentOperation<T, 0>::template doCastObject<ParameterType>(ValueTargetSource));
            auto ValueResult(_InterlockedXor64(ConcurrentOperation<T, 0>::template doCastAddress<ParameterType>(ValueAddress), ValueTarget) ^ ValueTarget);
            return reinterpret_cast<T&>(ValueResult);
        }
#else
        static T doAddFetch(volatile T &ValueAddress, T ValueTarget, MemoryOrder ValueOrder = MemoryOrder::OrderSeqCst) noexcept {
            return __atomic_add_fetch(ConcurrentOperation<T, 0>::template doCastAddress<T>(ValueAddress), ValueTarget, int(ValueOrder));
        }

        static T doAndFetch(volatile T &ValueAddress, T ValueTarget, MemoryOrder ValueOrder = MemoryOrder::OrderSeqCst) noexcept {
            return __atomic_and_fetch(ConcurrentOperation<T, 0>::template doCastAddress<T>(ValueAddress), ValueTarget, int(ValueOrder));
        }

        static T doFetchAdd(volatile T &ValueAddress, T ValueTarget, MemoryOrder ValueOrder = MemoryOrder::OrderSeqCst) noexcept {
            return __atomic_fetch_add(ConcurrentOperation<T, 0>::template doCastAddress<T>(ValueAddress), ValueTarget, int(ValueOrder));
        }

        static T doFetchAnd(volatile T &ValueAddress, T ValueTarget, MemoryOrder ValueOrder = MemoryOrder::OrderSeqCst) noexcept {
            return __atomic_fetch_and(ConcurrentOperation<T, 0>::template doCastAddress<T>(ValueAddress), ValueTarget, int(ValueOrder));
        }

        static T doFetchOr(volatile T &ValueAddress, T ValueTarget, MemoryOrder ValueOrder = MemoryOrder::OrderSeqCst) noexcept {
            return __atomic_fetch_or(ConcurrentOperation<T, 0>::template doCastAddress<T>(ValueAddress), ValueTarget, int(ValueOrder));
        }

        static T doFetchSub(volatile T &ValueAddress, T ValueTarget, MemoryOrder ValueOrder = MemoryOrder::OrderSeqCst) noexcept {
            return __atomic_fetch_sub(ConcurrentOperation<T, 0>::template doCastAddress<T>(ValueAddress), ValueTarget, int(ValueOrder));
        }

        static T doFetchXor(volatile T &ValueAddress, T ValueTarget, MemoryOrder ValueOrder = MemoryOrder::OrderSeqCst) noexcept {
            return __atomic_fetch_xor(ConcurrentOperation<T, 0>::template doCastAddress<T>(ValueAddress), ValueTarget, int(ValueOrder));
        }

        static bool doCompareExchange(volatile T &ValueAddress, T &ValueExpected, const T ValueTarget, MemoryOrder ValueOrder1 = MemoryOrder::OrderSeqCst, MemoryOrder ValueOrder2 = MemoryOrder::OrderSeqCst) noexcept {
            return __atomic_compare_exchange_n(ConcurrentOperation<T, 0>::template doCastAddress<T>(ValueAddress), ConcurrentOperation<T, 0>::template doCastAddress<T>(ValueExpected), ValueTarget, false, int(ValueOrder1), int(ValueOrder2));
        }

        static void doExchange(volatile T &ValueAddress, const T ValueTarget, MemoryOrder ValueOrder = MemoryOrder::OrderSeqCst) noexcept {
            __atomic_store_n(ConcurrentOperation<T, 0>::template doCastAddress<ParameterType>(ValueAddress), ConcurrentOperation<T, 0>::template doCastObject<ParameterType>(ValueTarget), int(ValueOrder));
        }

        static T doLoad(volatile T &ValueAddress, MemoryOrder ValueOrder = MemoryOrder::OrderSeqCst) noexcept {
            return __atomic_load_n(ConcurrentOperation<T, 0>::template doCastAddress<ParameterType>(ValueAddress), int(ValueOrder));
        }

        static T doOrFetch(volatile T &ValueAddress, T ValueTarget, MemoryOrder ValueOrder = MemoryOrder::OrderSeqCst) noexcept {
            return __atomic_or_fetch(ConcurrentOperation<T, 0>::template doCastAddress<T>(ValueAddress), ValueTarget, int(ValueOrder));
        }

        static T doSubFetch(volatile T &ValueAddress, T ValueTarget, MemoryOrder ValueOrder = MemoryOrder::OrderSeqCst) noexcept {
            return __atomic_sub_fetch(ConcurrentOperation<T, 0>::template doCastAddress<T>(ValueAddress), ValueTarget, int(ValueOrder));
        }

        static T doXorFetch(volatile T &ValueAddress, T ValueTarget, MemoryOrder ValueOrder = MemoryOrder::OrderSeqCst) noexcept {
            return __atomic_xor_fetch(ConcurrentOperation<T, 0>::template doCastAddress<T>(ValueAddress), ValueTarget, int(ValueOrder));
        }
#endif
    };

#if eLibraryArchitecture(AMD64)
    template<typename T>
    struct ConcurrentOperation<T, 16> {
#if eLibraryCompiler(MSVC)
        struct int128_t {
            alignas(16) long long LowPart;
            long long HighPart;
        };

        static bool doCompareExchange(volatile T &ValueAddress, T &ValueExpectedSource, const T ValueTargetSource, MemoryOrder=MemoryOrder::OrderSeqCst, MemoryOrder=MemoryOrder::OrderSeqCst) noexcept {
            int128_t ValueTarget{};
            ::memcpy(&ValueTarget, Objects::getAddress(ValueTargetSource), sizeof(T));
            int128_t ValueExpected{};
            ::memcpy(&ValueExpected, Objects::getAddress(ValueExpectedSource), sizeof(T));
            auto ValueResult(_InterlockedCompareExchange128(ConcurrentOperation<T, 0>::template doCastAddress<long long>(ValueAddress), ValueTarget.HighPart, ValueTarget.LowPart, &ValueExpected.LowPart));
            if (ValueResult == 0) ::memcpy(Objects::getAddress(ValueExpectedSource), &ValueExpected, sizeof(T));
            return ValueResult != 0;
        }

        static void doExchange(volatile T &ValueAddress, T ValueTarget, MemoryOrder ValueOrder = MemoryOrder::OrderSeqCst) noexcept {
            T ValueResult{ValueTarget};
            while (!doCompareExchange(ValueAddress, ValueResult, ValueTarget, ValueOrder));
        }

        static T doLoad(volatile T &ValueAddress, MemoryOrder=MemoryOrder::OrderSeqCst) noexcept {
            int128_t ValueResult;
            _InterlockedCompareExchange128(ConcurrentOperation<T, 0>::template doCastObject<long long>(ValueAddress), 0, 0, &ValueResult.LowPart);
            return reinterpret_cast<T&>(ValueResult);
        }
#else
        using ParameterType = __int128;

        static T doAddFetch(volatile T &ValueAddress, T ValueTarget, MemoryOrder ValueOrder = MemoryOrder::OrderSeqCst) noexcept {
            return __atomic_add_fetch(ConcurrentOperation<T, 0>::template doCastAddress<T>(ValueAddress), ValueTarget, int(ValueOrder));
        }

        static T doAndFetch(volatile T &ValueAddress, T ValueTarget, MemoryOrder ValueOrder = MemoryOrder::OrderSeqCst) noexcept {
            return __atomic_and_fetch(ConcurrentOperation<T, 0>::template doCastAddress<T>(ValueAddress), ValueTarget, int(ValueOrder));
        }

        static T doFetchAdd(volatile T &ValueAddress, T ValueTarget, MemoryOrder ValueOrder = MemoryOrder::OrderSeqCst) noexcept {
            return __atomic_fetch_add(ConcurrentOperation<T, 0>::template doCastAddress<T>(ValueAddress), ValueTarget, int(ValueOrder));
        }

        static T doFetchAnd(volatile T &ValueAddress, T ValueTarget, MemoryOrder ValueOrder = MemoryOrder::OrderSeqCst) noexcept {
            return __atomic_fetch_and(ConcurrentOperation<T, 0>::template doCastAddress<T>(ValueAddress), ValueTarget, int(ValueOrder));
        }

        static T doFetchOr(volatile T &ValueAddress, T ValueTarget, MemoryOrder ValueOrder = MemoryOrder::OrderSeqCst) noexcept {
            return __atomic_fetch_or(ConcurrentOperation<T, 0>::template doCastAddress<T>(ValueAddress), ValueTarget, int(ValueOrder));
        }

        static T doFetchSub(volatile T &ValueAddress, T ValueTarget, MemoryOrder ValueOrder = MemoryOrder::OrderSeqCst) noexcept {
            return __atomic_fetch_sub(ConcurrentOperation<T, 0>::template doCastAddress<T>(ValueAddress), ValueTarget, int(ValueOrder));
        }

        static T doFetchXor(volatile T &ValueAddress, T ValueTarget, MemoryOrder ValueOrder = MemoryOrder::OrderSeqCst) noexcept {
            return __atomic_fetch_xor(ConcurrentOperation<T, 0>::template doCastAddress<T>(ValueAddress), ValueTarget, int(ValueOrder));
        }

        static bool doCompareExchange(volatile T &ValueAddress, T &ValueExpected, const T ValueTarget, MemoryOrder ValueOrder1 = MemoryOrder::OrderSeqCst, MemoryOrder ValueOrder2 = MemoryOrder::OrderSeqCst) noexcept {
            return __atomic_compare_exchange_n(ConcurrentOperation<T, 0>::template doCastAddress<T>(ValueAddress), ConcurrentOperation<T, 0>::template doCastAddress<T>(ValueExpected), ValueTarget, false, int(ValueOrder1), int(ValueOrder2));
        }

        static void doExchange(volatile T &ValueAddress, const T ValueTarget, MemoryOrder ValueOrder = MemoryOrder::OrderSeqCst) noexcept {
            __atomic_store_n(ConcurrentOperation<T, 0>::template doCastAddress<ParameterType>(ValueAddress), ConcurrentOperation<T, 0>::template doCastObject<ParameterType>(ValueTarget), int(ValueOrder));
        }

        static T doLoad(volatile T &ValueAddress, MemoryOrder ValueOrder = MemoryOrder::OrderSeqCst) noexcept {
            return __atomic_load_n(ConcurrentOperation<T, 0>::template doCastAddress<ParameterType>(ValueAddress), int(ValueOrder));
        }

        static T doOrFetch(volatile T &ValueAddress, T ValueTarget, MemoryOrder ValueOrder = MemoryOrder::OrderSeqCst) noexcept {
            return __atomic_or_fetch(ConcurrentOperation<T, 0>::template doCastAddress<T>(ValueAddress), ValueTarget, int(ValueOrder));
        }

        static T doSubFetch(volatile T &ValueAddress, T ValueTarget, MemoryOrder ValueOrder = MemoryOrder::OrderSeqCst) noexcept {
            return __atomic_sub_fetch(ConcurrentOperation<T, 0>::template doCastAddress<T>(ValueAddress), ValueTarget, int(ValueOrder));
        }

        static T doXorFetch(volatile T &ValueAddress, T ValueTarget, MemoryOrder ValueOrder = MemoryOrder::OrderSeqCst) noexcept {
            return __atomic_xor_fetch(ConcurrentOperation<T, 0>::template doCastAddress<T>(ValueAddress), ValueTarget, int(ValueOrder));
        }
#endif
    };
#endif

    template<typename T, size_t=sizeof(::std::remove_reference_t<T>)>
    class AtomicStorage;

    template<typename T, size_t S>
    class AtomicStorage : public NonCopyable {
    private:
        alignas(alignof(T)) volatile mutable T StorageValue{};
    public:
        AtomicStorage() = default;

        constexpr AtomicStorage(const T &StorageSource) : StorageValue(StorageSource) {}

        T doAddFetch(T StorageTarget, MemoryOrder StorageOrder = MemoryOrder::OrderSeqCst) noexcept {
            return ConcurrentOperation<T, S>::doAddFetch(StorageValue, StorageTarget, StorageOrder);
        }

        T doAndFetch(T StorageTarget, MemoryOrder StorageOrder = MemoryOrder::OrderSeqCst) noexcept {
            return ConcurrentOperation<T, S>::doAndFetch(StorageValue, StorageTarget, StorageOrder);
        }

        bool doCompareExchangeValue(T &StorageExpected, const T StorageTarget, MemoryOrder StorageOrder1 = MemoryOrder::OrderSeqCst, MemoryOrder StorageOrder2 = MemoryOrder::OrderSeqCst) {
            return ConcurrentOperation<T, S>::doCompareExchange(StorageValue, StorageExpected, StorageTarget, StorageOrder1, StorageOrder2);
        }

        T doFetchAdd(T StorageTarget, MemoryOrder StorageOrder = MemoryOrder::OrderSeqCst) noexcept {
            return ConcurrentOperation<T, S>::doFetchAdd(StorageValue, StorageTarget, StorageOrder);
        }

        T doFetchAnd(T StorageTarget, MemoryOrder StorageOrder = MemoryOrder::OrderSeqCst) noexcept {
            return ConcurrentOperation<T, S>::doFetchAnd(StorageValue, StorageTarget, StorageOrder);
        }

        T doFetchOr(T StorageTarget, MemoryOrder StorageOrder = MemoryOrder::OrderSeqCst) noexcept {
            return ConcurrentOperation<T, S>::doFetchOr(StorageValue, StorageTarget, StorageOrder);
        }

        T doFetchSub(T StorageTarget, MemoryOrder StorageOrder = MemoryOrder::OrderSeqCst) noexcept {
            return ConcurrentOperation<T, S>::doFetchSub(StorageValue, StorageTarget, StorageOrder);
        }

        T doFetchXor(T StorageTarget, MemoryOrder StorageOrder = MemoryOrder::OrderSeqCst) noexcept {
            return ConcurrentOperation<T, S>::doFetchXor(StorageValue, StorageTarget, StorageOrder);
        }

        T doOrFetch(T StorageTarget, MemoryOrder StorageOrder = MemoryOrder::OrderSeqCst) noexcept {
            return ConcurrentOperation<T, S>::doOrFetch(StorageValue, StorageTarget, StorageOrder);
        }

        T doSubFetch(T StorageTarget, MemoryOrder StorageOrder = MemoryOrder::OrderSeqCst) noexcept {
            return ConcurrentOperation<T, S>::doSubFetch(StorageValue, StorageTarget, StorageOrder);
        }

        T doXorFetch(T StorageTarget, MemoryOrder StorageOrder = MemoryOrder::OrderSeqCst) noexcept {
            return ConcurrentOperation<T, S>::doXorFetch(StorageValue, StorageTarget, StorageOrder);
        }

        T getValue(MemoryOrder StorageOrder = MemoryOrder::OrderSeqCst) const {
            return ConcurrentOperation<T, S>::doLoad(StorageValue, StorageOrder);
        }

        void setValue(const T StorageSource, MemoryOrder StorageOrder = MemoryOrder::OrderSeqCst) {
            ConcurrentOperation<T, S>::doExchange(StorageValue, StorageSource, StorageOrder);
        }
    };

    class Mutex final : public AtomicStorage<bool> {
    public:
        void doLock() noexcept {
            while (this->getValue()) doYieldCpu();
            this->setValue(true);
        }

        void doUnlock() noexcept {
            this->setValue(false);
        }

        bool isLocked() const noexcept {
            return this->getValue();
        }

        bool tryLock() noexcept {
            bool MutexSource(false);
            return this->doCompareExchangeValue(MutexSource, true);
        }
    };

    class MutexLocker final : public NonCopyable, public NonMovable {
    private:
        Mutex &LockerMutex;
    public:
        explicit MutexLocker(Mutex &LockerMutexSource) noexcept : LockerMutex(LockerMutexSource) {
            LockerMutex.doLock();
        }

        ~MutexLocker() noexcept {
            LockerMutex.doUnlock();
        }
    };

    class MutexLockerUnique final : public NonCopyable {
    private:
        Mutex *LockerMutex = nullptr;
        bool LockerOwnership = false;
    public:
        doEnableMoveAssignConstruct(MutexLockerUnique)

        explicit MutexLockerUnique(Mutex &LockerMutexSource) noexcept : LockerMutex(Objects::getAddress(LockerMutexSource)), LockerOwnership(false) {
            doLock();
            LockerOwnership = true;
        }

        ~MutexLockerUnique() noexcept {
            if (LockerOwnership) doUnlock();
        }

        void doAssign(MutexLockerUnique &&LockerSource) noexcept {
            if (LockerOwnership) doUnlock();
            LockerMutex = LockerSource.LockerMutex;
            LockerOwnership = LockerSource.LockerOwnership;
            LockerSource.LockerMutex = nullptr;
            LockerSource.LockerOwnership = false;
        }

        void doLock() {
            if (!LockerMutex || LockerOwnership) throw Exception(u"MutexLockerUnique::doLock() LockerMutex || LockerOwnership"_S);
            LockerMutex->doLock();
            LockerOwnership = true;
        }

        void doUnlock() {
            if (!LockerOwnership) throw Exception(u"MutexLockerUnique::doUnlock() LockerOwnership"_S);
            if (LockerMutex) LockerMutex->doUnlock();
        }

        bool tryLock() {
            if (!LockerMutex || LockerOwnership) throw Exception(u"MutexLockerUnique::tryLock() LockerMutex || LockerOwnership"_S);
            return LockerOwnership = LockerMutex->tryLock();
        }
    };

    template<typename T>
    class AtomicStorage<T, 0> {
    private:
        T StorageValue;
        mutable Mutex StorageMutex;
    public:
        AtomicStorage() = default;

        constexpr AtomicStorage(const T &StorageSource) : StorageValue(StorageSource) {}

        T getValue() const {
            MutexLocker StorageGuard(StorageMutex);
            return StorageValue;
        }

        void setValue(const T StorageSource) {
            MutexLocker StorageGuard(StorageMutex);
            StorageValue = StorageSource;
        }
    };

#if eLibrarySystem(Windows)
    typedef Platform::Windows::NtHandle ThreadHandleType;
#else
    typedef pthread_t ThreadHandleType;
#endif

    class ThreadStateManager final {
    private:
        AtomicStorage<bool> StateFinish{false};
        AtomicStorage<bool> StateInterrupt{false};
    public:
        bool isFinished() const noexcept {
            return StateFinish.getValue();
        }

        bool isInterrupted() const noexcept {
            return StateInterrupt.getValue();
        }

        void setFinished(bool StateValue) noexcept {
            StateFinish.setValue(StateValue);
        }

        void setInterrupted(bool StateValue) noexcept {
            StateInterrupt.setValue(StateValue);
        }
    };

    /**
     * Support for multithreading
     */
    class Thread : public Object, public NonCopyable {
    private:
        ThreadHandleType ThreadHandle{};
        ThreadStateManager ThreadState;

        static void *doExecuteCore(void *ThreadContext) noexcept {
            ((Thread*) ThreadContext)->doExecute();
            ((Thread*) ThreadContext)->ThreadState.setFinished(true);
            return nullptr;
        }

#if eLibrarySystem(Windows)
        Thread(const Platform::Windows::NtHandle &ThreadHandleSource) noexcept : ThreadHandle(ThreadHandleSource) {}
#else
        constexpr Thread(ThreadHandleType ThreadHandleSource) noexcept : ThreadHandle(ThreadHandleSource) {}
#endif
    protected:
        void doStartCore() noexcept {
#if eLibrarySystem(Windows)
            ThreadHandle = ::CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE) &Thread::doExecuteCore, this, 0, nullptr);
#else
            pthread_attr_t ThreadAttribute;
            pthread_attr_init(&ThreadAttribute);
            pthread_create(&ThreadHandle, &ThreadAttribute, Thread::doExecuteCore, this);
            pthread_attr_destroy(&ThreadAttribute);
#endif
        }
    public:
        constexpr Thread() noexcept = default;

        virtual void doExecute() noexcept {}

        void doInterrupt() {
            if (isInterrupted()) throw InterruptedException(u"Thread::doInterrupt() isInterrupted"_S);
            ThreadState.setInterrupted(true);
        }

        void doJoin() const {
#if eLibrarySystem(Windows)
            while (!isFinished()) {
                if (isInterrupted()) throw InterruptedException(u"Thread::doJoin() isInterrupted"_S);
                doYieldCpu();
            }
            WaitForSingleObject((HANDLE) ThreadHandle, INFINITE);
#else
            while (!isFinished()) {
                if (isInterrupted()) throw InterruptedException(String(u"Thread::doJoin() isInterrupted"));
                doYieldCpu();
            }
            pthread_join(ThreadHandle, nullptr);
#endif
        }

        void doStart() {
            if (ThreadHandle) throw Exception(u"Thread::doStart() ThreadHandle"_S);
            doStartCore();
        }

        static void doYield() noexcept {
#if eLibrarySystem(Windows)
            SwitchToThread();
#else
            sched_yield();
#endif
        }

        bool isFinished() const noexcept {
            return ThreadState.isFinished();
        }

        bool isInterrupted() const noexcept {
            return ThreadState.isInterrupted();
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
            Functions::doInvokeTuple(ThreadFunction, ThreadParameter);
        }
    };

    template <typename T>
    class ConcurrentQueue final : public Object {
    private:
        ::std::queue<T> QueueObject;
        Mutex QueueMutex;
    public:
        bool isEmpty() {
            MutexLocker QueueLock(QueueMutex);
            return QueueObject.empty();
        }

        bool doDequeue(T &QueueSource) {
            MutexLocker QueueLock(QueueMutex);
            if (QueueObject.empty()) return false;
            QueueSource = Objects::doMove(QueueObject.front());
            QueueObject.pop();
            return true;
        }

        void doEnqueue(T &QueueSource) {
            MutexLocker QueueLock(QueueMutex);
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
        Array<ThreadExecutorCore*> ExecutorThread;
        ::std::condition_variable ExecutorVariable;
    public:
        ThreadExecutor(uintmax_t ExecutorThreadCount) : ExecutorThread(ExecutorThreadCount) {
            for (uintmax_t ExecutorThreadIndex = 0;ExecutorThreadIndex < ExecutorThreadCount;++ExecutorThreadIndex) {
                ExecutorThread.getElement(ExecutorThreadIndex) = new ThreadExecutorCore(this);
                ExecutorThread.getElement(ExecutorThreadIndex)->doStart();
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
        }

        template<typename F, typename ...Ps>
        auto doSubmit(F &&ExecutorFunction, Ps &&...ExecutorFunctionParameter) -> ::std::future<decltype(ExecutorFunction(ExecutorFunctionParameter...))> {
            if (ExecutorShutdown) throw Exception(u"ThreadExecutor::doSubmit<F, Ps...>(F&&, Ps&&...) ExecutorShutdown"_S);
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
}

#endif
