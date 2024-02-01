#pragma once

#ifndef eLibraryHeaderCoreMemory
#define eLibraryHeaderCoreMemory

#include <Core/Object.hpp>
#include <cstdio>
#include <new>

namespace eLibrary::Core {
    class MemoryResource : public Object {
    public:
        virtual void *doAcquire(size_t, size_t) = 0;

        virtual void doRelease(void*, size_t, size_t) = 0;
    };

    class HeapMemoryResource final : public MemoryResource {
    public:
        void *doAcquire(size_t MemorySize, size_t MemoryAlignment) override {
            return ::operator new(MemorySize, (std::align_val_t) MemoryAlignment);
        }

        void doRelease(void *MemorySource, size_t MemorySize, size_t MemoryAlignment) override {
            ::operator delete(MemorySource, MemorySize, (std::align_val_t) MemoryAlignment);
        }

        static HeapMemoryResource *getInstance() noexcept {
            static HeapMemoryResource *ResourceInstance = nullptr;
            if (!ResourceInstance) ResourceInstance = new HeapMemoryResource();
            return ResourceInstance;
        }
    };

    template<typename T>
    class MemoryAllocator final : public Object {
    private:
        MemoryResource *AllocatorResource = HeapMemoryResource::getInstance();
    public:
        template<typename ...Ts>
        auto acquireObject(Ts &&...ObjectParameter) {
            auto *ObjectSource = doAllocate(1);
            doConstruct(ObjectSource, Objects::doForward<Ts>(ObjectParameter)...);
            return ObjectSource;
        }

        static void deleteArray(T *ArraySource) noexcept {
            delete[] ArraySource;
        }

        static void deleteObject(T *ObjectSource) {
            doDestroy(ObjectSource);
            delete ObjectSource;
        }

        [[nodiscard]] T *doAllocate(uintmax_t ObjectSize) {
            return (T*) ::operator new(ObjectSize, AllocatorResource->doAcquire(ObjectSize * sizeof(T), alignof(T)));
        }

        template<typename ...Ts>
        static void doConstruct(T *ObjectSource, Ts&& ...ObjectParameter) {
            ::new((void*) ObjectSource) T(Objects::doForward<Ts>(ObjectParameter)...);
        }

        void doDeallocate(T *ObjectSource, uintmax_t ObjectSize) {
            AllocatorResource->doRelease(ObjectSource, ObjectSize * sizeof(T), alignof(T));
        }

        static void doDestroy(T *ObjectSource) {
            ObjectSource->~T();
        }

        const char *getClassName() const noexcept override {
            return "MemoryAllocator";
        }

        static auto newArray(uintmax_t ArraySize) noexcept {
            return new (std::nothrow) T[ArraySize];
        }

        template<typename ...As>
        static auto newObject(As&&... ArgumentList) {
            return new (std::nothrow) T(Objects::doForward<As>(ArgumentList)...);
        }

        void releaseObject(T *ObjectSource) {
            doDestroy(ObjectSource);
            doDeallocate(ObjectSource, 1);
        }
    };
}

#endif
