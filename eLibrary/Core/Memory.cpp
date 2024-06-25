#include <Core/Exception.hpp>
#include <new>

namespace eLibrary::Core {
    void *HeapMemoryResource::doAcquire(size_t MemorySize, size_t MemoryAlignment) noexcept {
        auto *MemoryResult(::operator new(MemorySize, (std::align_val_t) MemoryAlignment, ::std::nothrow));
        if (!MemoryResult) doThrowUnchecked(RuntimeException(u"HeapMemoryResource::doAcquire(size_t, size_t) ::operator new"_S));
        return MemoryResult;
    }

    void *NullMemoryResource::doAcquire(size_t MemorySize, size_t MemoryAlignment) noexcept {
        doThrowUnchecked(RuntimeException(u"NullMemoryResource::doAcquire(size_t, size_t)"_S));
    }
}