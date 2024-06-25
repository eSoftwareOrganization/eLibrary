#pragma once

#ifndef eLibraryHeaderCoreContainer
#define eLibraryHeaderCoreContainer

#include <Core/Exception.hpp>
#include <Core/Memory.hpp>
#include <iterator>
#include <typeinfo>
#include <utility>

namespace eLibrary::Core {
    namespace {
        template<typename E>
        struct DoubleLinkedNode final {
            E NodeValue;
            DoubleLinkedNode *NodeNext = nullptr, *NodePrevious = nullptr;

            explicit DoubleLinkedNode(const E &NodeValueSource) : NodeValue(NodeValueSource) {}
        };

        template<typename E>
        struct SingleLinkedNode final {
            E NodeValue;
            SingleLinkedNode *NodeNext = nullptr;

            explicit SingleLinkedNode(const E &NodeValueSource) : NodeValue(NodeValueSource) {}
        };
    }

    class Any final : public Object {
    private:
        enum class AnyOperation {
            OperationCopy, OperationDestroy, OperationMove
        };

        void *AnyValue = nullptr;
        const ::std::type_info *AnyType = &typeid(void);

        void (*AnyFunction)(AnyOperation, Any *, void*) = nullptr;

        template<typename T>
        static void doExecute(AnyOperation ManagerOperation, Any *ManagerObject, void *ManagerSource) {
            switch (ManagerOperation) {
                case AnyOperation::OperationCopy:
                    ManagerObject->AnyValue = new T(*(const T *) ManagerSource);
                    break;
                case AnyOperation::OperationDestroy:
                    MemoryAllocator<T>::doDestroy((T *) ManagerObject->AnyValue);
                    break;
                case AnyOperation::OperationMove:
                    ManagerObject->AnyValue = new T(Objects::doMove(*(T *) ManagerSource));
                    break;
            }
        }

    public:
        doEnableCopyAssignConstruct(Any)

        doEnableMoveAssignConstruct(Any)

        constexpr Any() = default;

        template<typename T>
        Any(const T &AnySource) {
            doAssign(AnySource);
        }

        template<typename T>
        Any(T &&AnySource) {
            doAssign(Objects::doMove(AnySource));
        }

        ~Any() {
            doReset();
        }

        void doAssign(const Any &AnySource) {
            doReset();
            if (AnySource.hasValue() && Objects::getAddress(AnySource) != this) {
                AnyFunction = AnySource.AnyFunction;
                AnyFunction(AnyOperation::OperationCopy, this, AnySource.AnyValue);
                AnyType = AnySource.AnyType;
            }
        }

        void doAssign(Any &&AnySource) noexcept {
            doReset();
            if (AnySource.hasValue() && Objects::getAddress(AnySource) != this) {
                AnyFunction = AnySource.AnyFunction;
                AnyFunction(AnyOperation::OperationMove, this, AnySource.AnyValue);
                AnyType = AnySource.AnyType;
                AnySource.AnyFunction = nullptr;
                AnySource.AnyType = &typeid(void);
                AnySource.AnyValue = nullptr;
            }
        }

        template<typename T>
        void doAssign(const T &AnySource) {
            doReset();
            AnyFunction = &doExecute<T>;
            AnyType = &typeid(T);
            AnyValue = new T(AnySource);
        }

        template<typename T>
        void doAssign(T &&AnySource) {
            doReset();
            AnyFunction = &doExecute<T>;
            AnyType = &typeid(T);
            AnyValue = new T(Objects::doMove(AnySource));
        }

        void doReset() {
            if (AnyFunction && AnyValue) AnyFunction(AnyOperation::OperationDestroy, this, nullptr);
            AnyFunction = nullptr;
            AnyType = &typeid(void);
            AnyValue = nullptr;
        }

        template<typename T>
        auto getValue() const {
            if (&typeid(T) != AnyType) [[unlikely]]
                doThrowChecked(TypeException, u"Any::getValue<T>()"_S);
            if (!hasValue()) [[unlikely]]
                doThrowChecked(Exception, u"Any::getValue<T>() hasValue"_S);
            return *((T *) AnyValue);
        }

        auto getValueType() noexcept {
            return AnyType;
        }

        bool hasValue() const noexcept {
            return AnyValue;
        }

        template<typename T>
        static Any valueOf(T &&AnySource) {
            return {Objects::doForward<T>(AnySource)};
        }

        template<typename T, typename ...Ts>
        static Any valueOf(Ts &&...AnySource) {
            return {T(Objects::doForward<Ts>(AnySource)...)};
        }
    };

    class Collections final : public NonConstructable {
    public:
        template<typename T1, typename T2>
        static void doCheckG(T1 IndexSource, T2 IndexStart) {
            if (IndexSource <= IndexStart) [[unlikely]]
                doThrowChecked(IndexException, u"Collections::doCheckG(T1, T2) IndexSource"_S);
        }

        template<typename T1, typename T2>
        static void doCheckGE(T1 IndexSource, T2 IndexStart) {
            if (IndexSource < IndexStart) [[unlikely]]
                doThrowChecked(IndexException, u"Collections::doCheckStartGE(T1, T2) IndexSource"_S);
        }

        template<typename T1, typename T2>
        static void doCheckL(T1 IndexSource, T2 IndexStop) {
            if (IndexSource >= IndexStop) [[unlikely]]
                doThrowChecked(IndexException, u"Collections::doCheckL(T1, T2) IndexSource"_S);
        }

        template<typename T1, typename T2>
        static void doCheckLE(T1 IndexSource, T2 IndexStop) {
            if (IndexSource > IndexStop) [[unlikely]]
                doThrowChecked(IndexException, u"Collections::doCheckLE(T1, T2) IndexSource"_S);
        }

        template<typename II>
        static intmax_t
        doCompare(II IteratorInput1Start, II IteratorInput1Stop, II IteratorInput2Start, II IteratorInput2Stop) {
            auto IteratorDistance1(getDistance(IteratorInput1Start, IteratorInput1Stop));
            auto IteratorDistance2(getDistance(IteratorInput2Start, IteratorInput2Stop));
            if (IteratorDistance1 != IteratorDistance2) return IteratorDistance1 > IteratorDistance2 ? 1 : -1;
            while (IteratorInput1Start != IteratorInput1Stop && IteratorInput2Start != IteratorInput2Stop) {
                auto IteratorResult(Objects::doCompare(*IteratorInput1Start, *IteratorInput2Start));
                if (IteratorResult) return IteratorResult;
                ++IteratorInput1Start;
                ++IteratorInput2Start;
            }
            return 0;
        }

        template<typename II>
        static intmax_t
        doCompare(II IteratorInput1, uintmax_t IteratorSize1, II IteratorInput2, uintmax_t IteratorSize2) {
            if (IteratorSize1 != IteratorSize2) return IteratorSize1 > IteratorSize2 ? 1 : -1;
            for (uintmax_t IteratorSize = 0; IteratorSize < IteratorSize1; ++IteratorInput1, ++IteratorInput2) {
                auto IteratorResult(Objects::doCompare(*IteratorInput1, *IteratorInput2));
                if (IteratorResult) return IteratorResult;
            }
            return 0;
        }

        template<typename II, typename OI>
        static constexpr void doCopy(II IteratorInputStart, II IteratorInputStop, OI IteratorOutput) {
            for (; IteratorInputStart != IteratorInputStop; ++IteratorInputStart, ++IteratorOutput)
                *IteratorOutput = *IteratorInputStart;
        }

        template<typename II, typename OI>
        static void doCopy(II IteratorInput, uintmax_t IteratorInputSize, OI IteratorOutput) {
            while (IteratorInputSize) {
                *IteratorOutput = *IteratorInput;
                ++IteratorOutput;
                ++IteratorInput;
                --IteratorInputSize;
            }
        }

        template<typename II, typename OI>
        static void doCopyBackward(II IteratorInputStart, II IteratorInputStop, OI IteratorOutput) {
            while (IteratorInputStart != IteratorInputStop)
                *(--IteratorOutput) = *(--IteratorInputStop);
        }

        template<typename II, typename OI>
        static void doCopyConstruct(II IteratorInputStart, II IteratorInputStop, OI IteratorOutput) {
            using T = typename ::std::iterator_traits<OI>::value_type;
            for (; IteratorInputStart != IteratorInputStop; ++IteratorInputStart, ++IteratorOutput)
                MemoryAllocator<T>::doConstruct(Objects::getAddress(*IteratorOutput), *IteratorInputStart);
        }

        template<typename II, typename OI>
        static void doCopyConstruct(II IteratorInput, uintmax_t IteratorInputSize, OI IteratorOutput) {
            using T = typename ::std::iterator_traits<OI>::value_type;
            while (IteratorInputSize) {
                MemoryAllocator<T>::doConstruct(Objects::getAddress(*IteratorOutput), *IteratorInput);
                ++IteratorOutput;
                ++IteratorInput;
                --IteratorInputSize;
            }
        }

        template<typename II, typename OI>
        static void doCopyReverse(II IteratorInputStart, II IteratorInputStop, OI IteratorOutput) {
            while (IteratorInputStart != IteratorInputStop) {
                --IteratorInputStop;
                *IteratorOutput = *IteratorInputStop;
                ++IteratorOutput;
            }
        }

        template<typename II, typename T>
        static void doFill(II IteratorInputStart, II IteratorInputStop, const T &IteratorValue) {
            for (; IteratorInputStart != IteratorInputStop; ++IteratorInputStart)
                *IteratorInputStart = IteratorValue;
        }

        template<typename II, typename OI>
        static void doMove(II IteratorInputStart, II IteratorInputStop, OI IteratorOutput) {
            for (; IteratorInputStart != IteratorInputStop; ++IteratorInputStart, ++IteratorOutput)
                *IteratorOutput = Objects::doMove(*IteratorInputStart);
        }

        template<typename II, typename OI>
        static void doMove(II IteratorInput, uintmax_t IteratorInputSize, OI IteratorOutput) {
            while (IteratorInputSize) {
                *IteratorOutput = Objects::doMove(*IteratorInput);
                ++IteratorOutput;
                ++IteratorInput;
                --IteratorInputSize;
            }
        }

        template<typename II, typename OI>
        static void doMoveBackward(II IteratorInputStart, II IteratorInputStop, OI IteratorOutput) {
            while (IteratorInputStart != IteratorInputStop)
                *(--IteratorOutput) = Objects::doMove(*(--IteratorInputStop));
        }

        template<typename II, typename OI>
        static void doMoveConstruct(II IteratorInputStart, II IteratorInputStop, OI IteratorOutput) {
            using T = typename ::std::iterator_traits<OI>::value_type;
            for (; IteratorInputStart != IteratorInputStop; ++IteratorInputStart, ++IteratorOutput)
                MemoryAllocator<T>::doConstruct(Objects::getAddress(*IteratorOutput), Objects::doMove(*IteratorInputStart));
        }

        template<typename II, typename OI>
        static void doMoveConstruct(II IteratorInput, uintmax_t IteratorInputSize, OI IteratorOutput) {
            using T = typename ::std::iterator_traits<OI>::value_type;
            while (IteratorInputSize) {
                MemoryAllocator<T>::doConstruct(Objects::getAddress(*IteratorOutput), Objects::doMove(*IteratorInput));
                ++IteratorOutput;
                ++IteratorInput;
                --IteratorInputSize;
            }
        }

        template<typename II, typename OI>
        static void doMoveReverse(II IteratorInputStart, II IteratorInputStop, OI IteratorOutput) {
            for (; IteratorInputStart != IteratorInputStop; --IteratorInputStop, ++IteratorOutput)
                *IteratorOutput = Objects::doMove(*IteratorInputStop);
        }

        template<typename II, typename F>
        static void doTraverse(II IteratorInputStart, II IteratorInputStop, F IteratorFunction) {
            for (; IteratorInputStart != IteratorInputStop; ++IteratorInputStart)
                IteratorFunction(*IteratorInputStart);
        }

        template<typename II>
        static auto getDistance(II IteratorInputStart, II IteratorInputStop) {
            uintmax_t IteratorDistance = 0;
            while (IteratorInputStart != IteratorInputStop) {
                ++IteratorInputStart;
                ++IteratorDistance;
            }
            return IteratorDistance;
        }
    };

    template<typename E>
    class ArrayIterator final {
    private:
        E *ElementCurrent;
    public:
        using difference_type = intmax_t;
        using iterator_category = ::std::contiguous_iterator_tag;
        using value_type = E;

        constexpr ArrayIterator() noexcept: ElementCurrent(nullptr) {}

        constexpr ArrayIterator(E *ElementSource) noexcept: ElementCurrent(ElementSource) {}

        ArrayIterator operator+(intmax_t ElementIndex) const noexcept {
            return {ElementCurrent + ElementIndex};
        }

        ArrayIterator &operator++() noexcept {
            ++ElementCurrent;
            return *this;
        }

        ArrayIterator operator++(int) noexcept {
            return {ElementCurrent++};
        }

        auto operator-(const ArrayIterator &ElementIndex) const noexcept {
            return ElementCurrent - ElementIndex.ElementCurrent;
        }

        ArrayIterator operator-(intmax_t ElementIndex) const noexcept {
            return {ElementCurrent - ElementIndex};
        }

        ArrayIterator &operator--() noexcept {
            --ElementCurrent;
            return *this;
        }

        ArrayIterator operator--(int) noexcept {
            return {ElementCurrent--};
        }

        E &operator*() const {
            return *ElementCurrent;
        }

        bool operator==(const ArrayIterator &IteratorSource) const noexcept {
            return ElementCurrent == IteratorSource.ElementCurrent;
        }

        bool operator!=(const ArrayIterator &IteratorSource) const noexcept {
            return ElementCurrent != IteratorSource.ElementCurrent;
        }
    };

    /**
     * Support for operating static arrays
     */
    template<typename E>
    class Array final : public Object {
    private:
        intmax_t ElementSize = 0;
        E *ElementContainer = nullptr;
        mutable MemoryAllocator<E> ElementAllocator;

        constexpr Array(E *ElementContainerSource, intmax_t ElementSizeSource) noexcept: ElementSize(ElementSizeSource),
                                                                                         ElementContainer(
                                                                                                 ElementContainerSource) {}

        template<typename ...Es>
        void doInitialize(E ElementCurrent, Es&& ...ElementList) noexcept {
            ElementAllocator.doConstruct(ElementContainer + (ElementSize++), ElementCurrent);
            if constexpr (sizeof...(ElementList)) doInitialize(Objects::doForward<Es>(ElementList)...);
        }

        template<typename>
        friend
        class ArrayList;

    public:
        doEnableCopyAssignConstruct(Array)

        doEnableMoveAssignConstruct(Array)

        constexpr Array() noexcept = default;

        template<typename II>
        Array(II ElementStart, II ElementStop, const MemoryAllocator<E> &AllocatorSource = MemoryAllocator<E>()) : ElementSize(Collections::getDistance(ElementStart, ElementStop)), ElementAllocator(AllocatorSource) {
            ElementContainer = ElementAllocator.doAllocate(ElementSize);
            Collections::doCopyConstruct(ElementStart, ElementStop, ElementContainer);
        }

        Array(::std::initializer_list<E> ElementSource): ElementSize(ElementSource.size()) {
            ElementContainer = ElementAllocator.doAllocate(ElementSize);
            Collections::doCopyConstruct(ElementSource.begin(), ElementSource.end(), ElementContainer);
        }

        template<typename ...Es>
        explicit Array(E ElementCurrent, Es&& ...ElementList) {
            ElementContainer = ElementAllocator.doAllocate(sizeof...(ElementList) + 1);
            doInitialize(Objects::doForward<E>(ElementCurrent), Objects::doForward<Es>(ElementList)...);
        }

        explicit Array(intmax_t ElementSizeSource) : ElementSize(ElementSizeSource) {
            ElementContainer = ElementAllocator.doAllocate(ElementSizeSource);
            for (intmax_t ElementIndex = 0;ElementIndex < ElementSize;++ElementIndex)
                ElementAllocator.doConstruct(ElementContainer + ElementIndex);
        }

        ~Array() {
            ElementAllocator.doDeallocate(ElementContainer, ElementSize);
            ElementSize = 0;
            ElementContainer = nullptr;
        }

        void doAssign(Array &&ElementSource) noexcept {
            if (Objects::getAddress(ElementSource) == this) return;
            if (ElementContainer) ElementAllocator.doDeallocate(ElementContainer, ElementSize);
            ElementContainer = ElementSource.ElementContainer;
            ElementSource.ElementContainer = nullptr;
            ElementSize = ElementSource.ElementSize;
            ElementSource.ElementSize = 0;
        }

        void doAssign(const Array &ElementSource) noexcept {
            if (Objects::getAddress(ElementSource) == this) return;
            if (ElementContainer) ElementAllocator.doDeallocate(ElementContainer, ElementSize);
            ElementContainer = ElementAllocator.doAllocate(ElementSize = ElementSource.ElementSize);
            Collections::doCopyConstruct(ElementSource.ElementContainer, ElementSize, ElementContainer);
        }

        intmax_t doCompare(const Array &ElementSource) const noexcept {
            return Collections::doCompare(begin(), ElementSize, ElementSource.begin(), ElementSource.ElementSize);
        }

        Array doConcat(const Array &ElementSource) const noexcept {
            E *ElementBuffer = ElementAllocator.doAllocate(ElementSize + ElementSource.ElementSize);
            Collections::doCopyConstruct(ElementContainer, ElementSize, ElementBuffer);
            Collections::doCopyConstruct(ElementSource.ElementContainer, ElementSource.ElementSize, ElementBuffer + ElementSize);
            return {ElementBuffer, ElementSize + ElementSource.ElementSize};
        }

        const E &getElement(intmax_t ElementIndex) const {
            if (ElementIndex < 0) ElementIndex += ElementSize;
            Collections::doCheckGE(ElementIndex, 0);
            Collections::doCheckL(ElementIndex, ElementSize);
            return ElementContainer[ElementIndex];
        }

        E &getElement(intmax_t ElementIndex) {
            if (ElementIndex < 0) ElementIndex += ElementSize;
            Collections::doCheckGE(ElementIndex, 0);
            Collections::doCheckL(ElementIndex, ElementSize);
            return ElementContainer[ElementIndex];
        }

        E *getElementContainer() const noexcept {
            return ElementContainer;
        }

        intmax_t getElementSize() const noexcept {
            return ElementSize;
        }

        bool isEmpty() const noexcept {
            return !ElementSize;
        }

        void setElement(intmax_t ElementIndex, const E &ElementSource) {
            if (ElementIndex < 0) ElementIndex += ElementSize;
            Collections::doCheckGE(ElementIndex, 0);
            Collections::doCheckL(ElementIndex, ElementSize);
            ElementContainer[ElementIndex] = ElementSource;
        }

        String toString() const noexcept override {
            StringBuilder CharacterStream;
            CharacterStream.addCharacter(u'[');
            for (intmax_t ElementIndex = 0; ElementIndex + 1 < ElementSize; ++ElementIndex) {
                CharacterStream.addString(String::valueOf(ElementContainer[ElementIndex]));
                CharacterStream.addCharacter(u',');
            }
            if (ElementSize) CharacterStream.addString(String::valueOf(ElementContainer[ElementSize - 1]));
            CharacterStream.addCharacter(u']');
            return CharacterStream.toString();
        }

        ArrayIterator<E> begin() const noexcept {
            return {ElementContainer};
        }

        ArrayIterator<E> end() const noexcept {
            return {ElementContainer + ElementSize};
        }
    };

    /**
     * Support for operating dynamic arrays
     */
    template<typename E>
    class ArrayList : public Object {
    protected:
        intmax_t ElementCapacity = 0, ElementSize = 0;
        E *ElementContainer = nullptr;
        mutable MemoryAllocator<E> ElementAllocator;

        ArrayList(E *ElementSource, intmax_t ElementSourceSize) noexcept: ElementCapacity(1),
                                                                          ElementSize(ElementSourceSize) {
            while (ElementCapacity < ElementSourceSize) ElementCapacity <<= 1;
            ElementContainer = ElementAllocator.doAllocate(ElementCapacity);
            Collections::doCopyConstruct(ElementSource, ElementSource + ElementSourceSize, ElementContainer);
        }

        template<typename ...Es>
        void doInitialize(E ElementCurrent, Es&& ...ElementList) noexcept {
            ElementAllocator.doConstruct(ElementContainer + (ElementSize++), ElementCurrent);
            if constexpr (sizeof...(ElementList)) doInitialize(Objects::doForward<Es>(ElementList)...);
        }

    public:
        doEnableCopyAssignConstruct(ArrayList)

        doEnableMoveAssignConstruct(ArrayList)

        constexpr ArrayList() noexcept = default;

        template<typename ...Es>
        ArrayList(Es&& ...ElementList) : ElementCapacity(1) {
            while ((uintmax_t) ElementCapacity < sizeof...(ElementList)) ElementCapacity <<= 1;
            ElementContainer = ElementAllocator.doAllocate(ElementCapacity);
            doInitialize(Objects::doForward<Es>(ElementList)...);
        }

        ArrayList(::std::initializer_list<E> ElementList) : ElementCapacity(1), ElementSize((intmax_t) ElementList.size()) {
            while (ElementCapacity < ElementSize)
                ElementCapacity <<= 1;
            ElementContainer = ElementAllocator.doAllocate(ElementCapacity);
            Collections::doCopyConstruct(ElementList.begin(), ElementList.end(), ElementContainer);
        }

        template<typename II>
        ArrayList(II ElementStart, II ElementStop) : ElementCapacity(1), ElementSize(Collections::getDistance(ElementStart, ElementStop)) {
            while (ElementCapacity < ElementSize)
                ElementCapacity <<= 1;
            ElementContainer = ElementAllocator.doAllocate(ElementSize);
            Collections::doCopyConstruct(ElementStart, ElementStop, ElementContainer);
        }

        ~ArrayList() {
            doClear();
        }

        void addElement(const E &ElementSource) noexcept {
            if (ElementSize == ElementCapacity)
                doReserve(ElementCapacity ? ElementCapacity << 1 : 1);
            ElementAllocator.doConstruct(ElementContainer + (ElementSize++), ElementSource);
        }

        void addElement(intmax_t ElementIndex, const E &ElementSource) {
            if (ElementIndex < 0) ElementIndex += ElementSize + 1;
            Collections::doCheckGE(ElementIndex, 0);
            Collections::doCheckLE(ElementIndex, ElementSize);
            if (ElementSize == ElementCapacity)
                doReserve(ElementCapacity ? ElementCapacity << 1 : 1);
            Collections::doMoveBackward(ElementContainer + ElementIndex, ElementContainer + ElementSize,
                                        ElementContainer + ElementSize + 1);
            ElementContainer[ElementIndex] = ElementSource;
            ++ElementSize;
        }

        void doAssign(ArrayList &&ElementSource) noexcept {
            if (Objects::getAddress(ElementSource) == this) return;
            ElementAllocator.doDeallocate(ElementContainer, ElementCapacity);
            ElementCapacity = ElementSource.ElementCapacity;
            ElementContainer = ElementSource.ElementContainer;
            ElementSize = ElementSource.ElementSize;
            ElementSource.ElementCapacity = 0;
            ElementSource.ElementContainer = nullptr;
            ElementSource.ElementSize = 0;
        }

        void doAssign(const ArrayList &ElementSource) noexcept {
            if (Objects::getAddress(ElementSource) == this) return;
            ElementAllocator.doDeallocate(ElementContainer, ElementCapacity);
            ElementCapacity = ElementSource.ElementCapacity;
            if (ElementCapacity)
                ElementContainer = ElementAllocator.doAllocate(ElementCapacity = ElementSource.ElementCapacity);
            Collections::doCopyConstruct(ElementSource.ElementContainer, (ElementSize = ElementSource.ElementSize),
                                ElementContainer);
        }

        void doClear() {
            ElementAllocator.doDeallocate(ElementContainer, ElementCapacity);
            ElementCapacity = 0;
            ElementSize = 0;
            ElementContainer = nullptr;
        }

        intmax_t doCompare(const ArrayList &ElementSource) const noexcept {
            return Collections::doCompare(begin(), ElementSize, ElementSource.begin(), ElementSource.ElementSize);
        }

        ArrayList doConcat(const ArrayList &ElementSource) const noexcept {
            E *ElementBuffer = ElementAllocator.doAllocate(ElementSize + ElementSource.ElementSize);
            Collections::doCopyConstruct(ElementContainer, ElementSize, ElementBuffer);
            Collections::doCopyConstruct(ElementSource.ElementContainer, ElementSource.ElementSize, ElementBuffer + ElementSize);
            return {ElementBuffer, ElementSize + ElementSource.ElementSize};
        }

        void doReserve(intmax_t ElementCapacityNew) noexcept {
            if (!ElementCapacity) [[unlikely]] ElementContainer = ElementAllocator.doAllocate(ElementCapacity = 1);
            if (ElementCapacityNew > ElementCapacity) {
                E *ElementBuffer = ElementAllocator.doAllocate(ElementSize);
                Collections::doMoveConstruct(ElementContainer, ElementContainer + ElementSize, ElementBuffer);
                ElementAllocator.doDeallocate(ElementContainer, ElementCapacity);
                ElementContainer = ElementAllocator.doAllocate(ElementCapacity = ElementCapacityNew);
                Collections::doMoveConstruct(ElementBuffer, ElementBuffer + ElementSize, ElementContainer);
                ElementAllocator.doDeallocate(ElementBuffer, ElementSize);
            }
        }

        void doReverse() noexcept {
            E *ElementBuffer = ElementAllocator.doAllocate(ElementSize);
            Collections::doMoveConstruct(ElementContainer, ElementSize, ElementBuffer);
            Collections::doMoveReverse(ElementBuffer, ElementBuffer + ElementSize, ElementContainer);
            ElementAllocator.doDeallocate(ElementBuffer, ElementSize);
        }

        const E &getElement(intmax_t ElementIndex) const {
            if (ElementIndex < 0) ElementIndex += ElementSize;
            Collections::doCheckGE(ElementIndex, 0);
            Collections::doCheckL(ElementIndex, ElementSize);
            return ElementContainer[ElementIndex];
        }

        E &getElement(intmax_t ElementIndex) {
            if (ElementIndex < 0) ElementIndex += ElementSize;
            Collections::doCheckGE(ElementIndex, 0);
            Collections::doCheckL(ElementIndex, ElementSize);
            return ElementContainer[ElementIndex];
        }

        intmax_t getElementSize() const noexcept {
            return ElementSize;
        }

        intmax_t indexOf(const E &ElementSource) const noexcept {
            for (intmax_t ElementIndex = 0; ElementIndex < ElementSize; ++ElementIndex)
                if (!Objects::doCompare(ElementContainer[ElementIndex], ElementSource)) return ElementIndex;
            return -1;
        }

        bool isContains(const E &ElementSource) const noexcept {
            for (intmax_t ElementIndex = 0; ElementIndex < ElementSize; ++ElementIndex)
                if (!Objects::doCompare(ElementContainer[ElementIndex], ElementSource)) return true;
            return false;
        }

        bool isEmpty() const noexcept {
            return !ElementSize;
        }

        void removeElement(const E &ElementSource) {
            intmax_t ElementIndex = indexOf(ElementSource);
            Collections::doCheckGE(ElementIndex, 0);
            removeIndex(ElementIndex);
        }

        void removeIndex(intmax_t ElementIndex) {
            if (ElementIndex < 0) ElementIndex += ElementSize;
            Collections::doCheckGE(ElementIndex, 0);
            Collections::doCheckL(ElementIndex, ElementSize);
            Collections::doMove(ElementContainer + ElementIndex + 1, ElementContainer + ElementSize,
                                ElementContainer + ElementIndex);
            if (ElementCapacity == 1) doClear();
            else if (--ElementSize <= ElementCapacity >> 1) {
                E *ElementBuffer = ElementAllocator.doAllocate(ElementSize);
                Collections::doMove(ElementContainer, ElementSize, ElementBuffer);
                ElementAllocator.doDeallocate(ElementContainer, ElementCapacity);
                ElementContainer = ElementAllocator.doAllocate(ElementCapacity >>= 1);
                Collections::doMove(ElementBuffer, ElementSize, ElementContainer);
                ElementAllocator.doDeallocate(ElementBuffer, ElementSize);
            }
        }

        void setElement(intmax_t ElementIndex, const E &ElementSource) {
            if (ElementIndex < 0) ElementIndex += ElementSize;
            Collections::doCheckGE(ElementIndex, 0);
            Collections::doCheckL(ElementIndex, ElementSize);
            ElementContainer[ElementIndex] = ElementSource;
        }

        String toString() const noexcept override {
            StringBuilder CharacterStream;
            CharacterStream.addCharacter(u'[');
            for (intmax_t ElementIndex = 0; ElementIndex + 1 < ElementSize; ++ElementIndex) {
                CharacterStream.addString(String::valueOf(ElementContainer[ElementIndex]));
                CharacterStream.addCharacter(u',');
            }
            if (ElementSize) CharacterStream.addString(String::valueOf(ElementContainer[ElementSize - 1]));
            CharacterStream.addCharacter(u']');
            return CharacterStream.toString();
        }

        ArrayIterator<E> begin() const noexcept {
            return {ElementContainer};
        }

        ArrayIterator<E> end() const noexcept {
            return {ElementContainer + ElementSize};
        }
    };

    template<typename K, typename V>
    class [[deprecated]] AVLTree : public Object {
    protected:
        struct AVLNode final {
            K NodeKey;
            V NodeValue;
            AVLNode *NodeChildLeft = nullptr, *NodeChildRight = nullptr, *NodeParent = nullptr;
            int NodeHeight = 0;

            AVLNode(const K &NodeKeySource, const V &NodeValueSource) : NodeKey(NodeKeySource), NodeValue(NodeValueSource) {}
        } *NodeRoot = nullptr;
        mutable MemoryAllocator<AVLNode> NodeAllocator;

        void deleteNode(AVLNode *NodeCurrent) {
            if (!NodeCurrent) return;
            deleteNode(NodeCurrent->NodeChildLeft);
            deleteNode(NodeCurrent->NodeChildRight);
            NodeAllocator.releaseObject(NodeCurrent);
        }

        template<typename F>
        static void doOrderCore(AVLNode *NodeCurrent, F Operation) {
            if (!NodeCurrent) return;
            Operation(NodeCurrent->NodeKey, NodeCurrent->NodeValue);
            doOrderCore(NodeCurrent->NodeChildLeft, Operation);
            doOrderCore(NodeCurrent->NodeChildRight, Operation);
        }
    public:
        doEnableCopyAssignConstruct(AVLTree)

        doEnableMoveAssignConstruct(AVLTree)

        constexpr AVLTree() noexcept = default;

        ~AVLTree() {
            doClear();
        }

        void doAssign(const AVLTree &TreeSource) {
            if (Objects::getAddress(TreeSource) == this) return;
            doClear();
            TreeSource.doOrderCore(TreeSource.NodeRoot, [&](const K &NodeKeySource, const V &NodeValueSource) {
                doInsert(NodeKeySource, NodeValueSource);
            });
        }

        void doAssign(AVLTree &&TreeSource) {
            if (Objects::getAddress(TreeSource) == this) return;
            doClear();
            NodeRoot = TreeSource.NodeRoot;
            TreeSource.NodeRoot = nullptr;
        }

        void doClear() {
            deleteNode(NodeRoot);
            NodeRoot = nullptr;
        }

        void doInsert(const K &NodeKey, const V &NodeValue) noexcept {
            auto *NodeTarget = NodeAllocator.acquireObject(NodeKey, NodeValue);
            if (!NodeRoot) {
                NodeRoot = NodeTarget;
                return;
            }
            auto *NodeParent = NodeRoot;
            while (NodeParent) {
                auto NodeRelation = Objects::doCompare(NodeTarget->NodeKey, NodeParent->NodeKey);
                if (NodeRelation == 0) {
                    NodeParent->NodeValue = NodeTarget->NodeValue;
                    return;
                } else if (NodeRelation > 0) {
                    if (!NodeParent->NodeChildRight) {
                        NodeParent->NodeChildRight = NodeTarget;
                        NodeTarget->NodeParent = NodeParent;
                        break;
                    }
                    NodeParent = NodeParent->NodeChildRight;
                } else {
                    if (!NodeParent->NodeChildLeft) {
                        NodeParent->NodeChildLeft = NodeTarget;
                        NodeTarget->NodeParent = NodeParent;
                        break;
                    }
                    NodeParent = NodeParent->NodeChildLeft;
                }
            }
        }

        template<typename F>
        void doOrder(F Operation) const {
            doOrderCore(NodeRoot, Operation);
        }
    };

    /**
     * Support for continuous storage of unique objects
     */
    template<typename E, typename C>
    class ContainerSet final : public Object {
    private:
        C ElementContainer;
    public:
        doEnableCopyAssignConstruct(ContainerSet)

        doEnableMoveAssignConstruct(ContainerSet)

        constexpr ContainerSet() noexcept = default;

        explicit ContainerSet(const C &ElementSource) noexcept: ElementContainer(ElementSource) {}

        ~ContainerSet() {
            doClear();
        }

        void addElement(const E &ElementSource) noexcept {
            if (!isContains(ElementSource)) ElementContainer.addElement(ElementSource);
        }

        void doAssign(ContainerSet &&SetSource) noexcept {
            if (Objects::getAddress(SetSource) == this) return;
            ElementContainer.doAssign(Objects::doMove(SetSource.ElementContainer));
        }

        void doAssign(const ContainerSet &SetSource) noexcept {
            if (Objects::getAddress(SetSource) == this) return;
            ElementContainer.doAssign(SetSource.ElementContainer);
        }

        void doClear() {
            ElementContainer.doClear();
        }

        intmax_t doCompare(const ContainerSet &ElementSource) const noexcept {
            return Collections::doCompare(begin(), getElementSize(), ElementSource.begin(),
                                          ElementSource.getElementSize());
        }

        ContainerSet doDifference(const ContainerSet &SetSource) const noexcept {
            ContainerSet SetResult;
            Collections::doTraverse(ElementContainer.begin(), ElementContainer.end(), [&](const E &ElementCurrent) {
                if (!SetSource.isContains(ElementCurrent)) SetResult.addElement(ElementCurrent);
            });
            return SetResult;
        }

        ContainerSet doIntersection(const ContainerSet &SetSource) const noexcept {
            ContainerSet SetResult;
            Collections::doTraverse(ElementContainer.begin(), ElementContainer.end(), [&](const E &ElementCurrent) {
                if (SetSource.isContains(ElementCurrent)) SetResult.addElement(ElementCurrent);
            });
            return SetResult;
        }

        ContainerSet doUnion(const ContainerSet &SetSource) const noexcept {
            ContainerSet SetResult(*this);
            Collections::doTraverse(SetSource.begin(), SetSource.end(), [&](const E &ElementCurrent) {
                SetResult.addElement(ElementCurrent);
            });
            return SetResult;
        }

        auto getElementSize() const noexcept {
            return ElementContainer.getElementSize();
        }

        bool isContains(const E &ElementSource) const noexcept {
            return ElementContainer.isContains(ElementSource);
        }

        bool isEmpty() const noexcept {
            return ElementContainer.isEmpty();
        }

        void removeElement(const E &ElementSource) {
            ElementContainer.removeElement(ElementSource);
        }

        String toString() const noexcept override {
            return ElementContainer.toString();
        }

        auto begin() const noexcept {
            return ElementContainer.begin();
        }

        auto end() const noexcept {
            return ElementContainer.end();
        }
    };

    template<typename E, typename C>
    class ContainerQueue final : public Object {
    private:
        C ElementContainer;
    public:
        doEnableCopyAssignConstruct(ContainerQueue)

        doEnableMoveAssignConstruct(ContainerQueue)

        constexpr ContainerQueue() noexcept = default;

        ~ContainerQueue() {
            doClear();
        }

        void addBack(const E &ElementSource) noexcept {
            ElementContainer.addElement(ElementSource);
        }

        void addFront(const E &ElementSource) noexcept {
            ElementContainer.addElement(0, ElementSource);
        }

        void doAssign(const ContainerQueue<E, C> &ElementSource) {
            if (Objects::getAddress(ElementSource) == this) return;
            if (!isEmpty()) doClear();
            ElementContainer.doAssign(ElementSource);
        }

        void doAssign(ContainerQueue<E, C> &&ElementSource) noexcept {
            if (Objects::getAddress(ElementSource) == this) return;
            if (!isEmpty()) doClear();
            ElementContainer.doAssign(Objects::doMove(ElementSource));
        }

        void doClear() {
            ElementContainer.doClear();
        }

        const E &getBack() const {
            return ElementContainer.getElement(-1);
        }

        E &getBack() {
            return ElementContainer.getElement(-1);
        }

        intmax_t getElementSize() const noexcept {
            return ElementContainer.getElementSize();
        }

        const E &getFront() const {
            return ElementContainer.getElement(0);
        }

        E &getFront() {
            return ElementContainer.getElement(0);
        }

        bool isEmpty() const noexcept {
            return ElementContainer.isEmpty();
        }

        void removeBack() {
            ElementContainer.removeIndex(-1);
        }

        void removeFront() {
            ElementContainer.removeIndex(0);
        }

        String toString() const noexcept override {
            return ElementContainer.toString();
        }
    };

    template<typename E, typename C>
    class ContainerStack final : public Object {
    private:
        C ElementContainer;
    public:
        doEnableCopyAssignConstruct(ContainerStack)

        doEnableMoveAssignConstruct(ContainerStack)

        constexpr ContainerStack() noexcept = default;

        ~ContainerStack() {
            doClear();
        }

        void addElement(const E &ElementSource) noexcept {
            ElementContainer.addElement(0, ElementSource);
        }

        void doAssign(const ContainerStack<E, C> &ElementSource) noexcept {
            if (Objects::getAddress(ElementSource) == this) return;
            if (!isEmpty()) doClear();
            ElementContainer.doAssign(ElementSource.ElementContainer);
        }

        void doAssign(ContainerStack<E, C> &&ElementSource) noexcept {
            if (Objects::getAddress(ElementSource) == this) return;
            if (!isEmpty()) doClear();
            ElementContainer.doAssign(Objects::doMove(ElementSource.ElementContainer));
        }

        void doClear() {
            ElementContainer.doClear();
        }

        const E &getElement() const {
            return ElementContainer.getElement(0);
        }

        E &getElement() {
            return ElementContainer.getElement(0);
        }

        intmax_t getElementSize() const noexcept {
            return ElementContainer.getElementSize();
        }

        bool isEmpty() const noexcept {
            return ElementContainer.isEmpty();
        }

        void removeElement() {
            ElementContainer.removeIndex(0);
        }

        String toString() const noexcept override {
            return ElementContainer.toString();
        }
    };

    template<typename E>
    class DoubleLinkedIterator final {
    private:
        using LinkedNode = DoubleLinkedNode<E>;
        LinkedNode *NodeCurrent;
    public:
        using difference_type = ptrdiff_t;
        using iterator_category = ::std::bidirectional_iterator_tag;
        using value_type = E;

        constexpr DoubleLinkedIterator() noexcept: NodeCurrent(nullptr) {}

        constexpr DoubleLinkedIterator(LinkedNode *NodeSource) noexcept: NodeCurrent(NodeSource) {}

        DoubleLinkedIterator &operator++() {
            NodeCurrent = NodeCurrent->NodeNext;
            return *this;
        }

        DoubleLinkedIterator operator++(int) {
            LinkedNode *NodeBuffer = NodeCurrent;
            NodeCurrent = NodeCurrent->NodeNext;
            return {NodeBuffer};
        }

        DoubleLinkedIterator &operator--() {
            NodeCurrent = NodeCurrent->NodePrevious;
            return *this;
        }

        DoubleLinkedIterator operator--(int) {
            LinkedNode *NodeBuffer = NodeCurrent;
            NodeCurrent = NodeCurrent->NodePrevious;
            return {NodeBuffer};
        }

        E &operator*() const {
            return NodeCurrent->NodeValue;
        }

        bool operator==(const DoubleLinkedIterator &IteratorSource) const noexcept {
            return NodeCurrent == IteratorSource.NodeCurrent;
        }

        bool operator!=(const DoubleLinkedIterator &IteratorSource) const noexcept {
            return NodeCurrent != IteratorSource.NodeCurrent;
        }
    };

    /**
     * Support for discrete storage of objects
     */
    template<typename E>
    class DoubleLinkedList final : public Object {
    private:
        using LinkedNode = DoubleLinkedNode<E>;
        LinkedNode *NodeHead = nullptr, *NodeTail = nullptr;
        intmax_t NodeSize = 0;
        mutable MemoryAllocator<LinkedNode> NodeAllocator;

        template<typename ...Es>
        void doInitialize(E ElementCurrent, Es ...ElementList) noexcept {
            addElement(ElementCurrent);
            if constexpr (sizeof...(ElementList)) doInitialize(ElementList...);
        }
    public:
        doEnableCopyAssignConstruct(DoubleLinkedList)

        doEnableMoveAssignConstruct(DoubleLinkedList)

        constexpr DoubleLinkedList() noexcept = default;

        template<typename ...Es>
        explicit DoubleLinkedList(Es ...ElementList) noexcept {
            doInitialize(ElementList...);
        }

        DoubleLinkedList(::std::initializer_list<E> ElementList) noexcept: NodeSize(ElementList.size()) {
            for (const E &ElementCurrent: ElementList) addElement(ElementCurrent);
        }

        template<typename II>
        DoubleLinkedList(II ElementStart, II ElementStop) noexcept {
            Collections::doTraverse(ElementStart, ElementStop, [&](const E &ElementCurrent) {
                addElement(ElementCurrent);
            });
        }

        ~DoubleLinkedList() {
            doClear();
        }

        void addElement(const E &ElementSource) noexcept {
            auto *NodeCurrent = NodeAllocator.acquireObject(ElementSource);
            if (!NodeHead) NodeHead = NodeTail = NodeCurrent;
            else {
                NodeTail->NodeNext = NodeCurrent;
                NodeCurrent->NodePrevious = NodeTail;
                NodeTail = NodeCurrent;
            }
            ++NodeSize;
        }

        void addElement(intmax_t ElementIndex, const E &ElementSource) {
            if (ElementIndex < 0) ElementIndex += NodeSize + 1;
            Collections::doCheckGE(ElementIndex, 0);
            Collections::doCheckLE(ElementIndex, NodeSize);
            if (ElementIndex == NodeSize) {
                addElement(ElementSource);
                return;
            }
            auto *NodeTarget = NodeAllocator.acquireObject(ElementSource);
            if (ElementIndex == 0) {
                NodeTarget->NodeNext = NodeHead;
                NodeHead->NodePrevious = NodeTarget;
                NodeHead = NodeTarget;
                ++NodeSize;
                return;
            }
            auto *NodeCurrent(NodeHead);
            while (--ElementIndex) NodeCurrent = NodeCurrent->NodeNext;
            NodeTarget->NodePrevious = NodeCurrent;
            NodeTarget->NodeNext = NodeCurrent->NodeNext;
            NodeCurrent->NodeNext->NodePrevious = NodeTarget;
            NodeCurrent->NodeNext = NodeTarget;
            ++NodeSize;
        }

        void doAssign(const DoubleLinkedList &ElementSource) noexcept {
            if (Objects::getAddress(ElementSource) == this) return;
            if (!isEmpty()) doClear();
            NodeSize = ElementSource.NodeSize;
            LinkedNode *NodeCurrent = ElementSource.NodeHead;
            while (NodeCurrent) {
                addElement(NodeCurrent->NodeValue);
                NodeCurrent = NodeCurrent->NodeNext;
            }
        }

        void doAssign(DoubleLinkedList &&ElementSource) noexcept {
            if (Objects::getAddress(ElementSource) == this) return;
            if (!isEmpty()) doClear();
            NodeHead = ElementSource.NodeHead;
            NodeSize = ElementSource.NodeSize;
            NodeTail = ElementSource.NodeTail;
            ElementSource.NodeHead = ElementSource.NodeTail = nullptr;
            ElementSource.NodeSize = 0;
        }

        void doClear() {
            NodeSize = 0;
            NodeTail = nullptr;
            auto *NodeCurrent(NodeHead);
            NodeHead = nullptr;
            if (!NodeCurrent) return;
            while (NodeCurrent->NodeNext) {
                NodeCurrent = NodeCurrent->NodeNext;
                NodeAllocator.releaseObject(NodeCurrent->NodePrevious);
            }
            NodeAllocator.releaseObject(NodeCurrent);
        }

        intmax_t doCompare(const DoubleLinkedList &ElementSource) const noexcept {
            return Collections::doCompare(begin(), NodeSize, ElementSource.begin(), ElementSource.NodeSize);
        }

        void doReverse() noexcept {
            if (NodeHead == NodeTail) return;
            if (NodeHead->NodeNext == NodeTail) {
                Objects::doSwap(NodeHead, NodeTail);
                return;
            }
            LinkedNode *NodeCurrent = NodeHead->NodeNext, *NodeNext, *NodeTemporary = NodeAllocator.acquireObject();
            NodeTemporary->NodeNext = NodeTail;
            while (NodeCurrent != NodeTail) {
                NodeNext = NodeCurrent->NodeNext;
                NodeCurrent->NodeNext = NodeTemporary->NodeNext;
                NodeTemporary->NodeNext->NodePrevious = NodeCurrent;
                NodeCurrent->NodePrevious = NodeTemporary;
                NodeTemporary->NodeNext = NodeCurrent;
                NodeCurrent = NodeNext;
            }
            NodeTemporary->NodeNext->NodePrevious = NodeHead;
            NodeHead->NodeNext = NodeTemporary->NodeNext;
        }

        const E &getElement(intmax_t ElementIndex) const {
            if (ElementIndex < 0) ElementIndex += NodeSize;
            Collections::doCheckGE(ElementIndex, 0);
            Collections::doCheckL(ElementIndex, NodeSize);
            LinkedNode *NodeCurrent;
            if (ElementIndex < (NodeSize >> 1)) {
                NodeCurrent = NodeHead;
                while (ElementIndex--) NodeCurrent = NodeCurrent->NodeNext;
            } else {
                ElementIndex = NodeSize - ElementIndex - 1;
                NodeCurrent = NodeTail;
                while (ElementIndex--) NodeCurrent = NodeCurrent->NodePrevious;
            }
            return NodeCurrent->NodeValue;
        }

        E &getElement(intmax_t ElementIndex) {
            if (ElementIndex < 0) ElementIndex += NodeSize;
            Collections::doCheckGE(ElementIndex, 0);
            Collections::doCheckL(ElementIndex, NodeSize);
            LinkedNode *NodeCurrent;
            if (ElementIndex < (NodeSize >> 1)) {
                NodeCurrent = NodeHead;
                while (ElementIndex--) NodeCurrent = NodeCurrent->NodeNext;
            } else {
                ElementIndex = NodeSize - ElementIndex - 1;
                NodeCurrent = NodeTail;
                while (ElementIndex--) NodeCurrent = NodeCurrent->NodePrevious;
            }
            return NodeCurrent->NodeValue;
        }

        intmax_t getElementSize() const noexcept {
            return NodeSize;
        }

        intmax_t indexOf(const E &ElementSource) const noexcept {
            intmax_t NodeIndex = 0;
            auto *NodeCurrent(NodeHead);
            while (NodeCurrent && Objects::doCompare(NodeCurrent->NodeValue, ElementSource))
                NodeCurrent = NodeCurrent->NodeNext, ++NodeIndex;
            if (!NodeCurrent) return -1;
            return NodeIndex;
        }

        bool isContains(const E &ElementSource) const noexcept {
            auto *NodeCurrent(NodeHead);
            while (NodeCurrent) {
                if (Objects::doCompare(NodeCurrent->NodeValue, ElementSource) == 0) return true;
                NodeCurrent = NodeCurrent->NodeNext;
            }
            return false;
        }

        bool isEmpty() const noexcept {
            return !NodeSize;
        }

        void removeElement(const E &ElementSource) {
            intmax_t ElementIndex = indexOf(ElementSource);
            Collections::doCheckGE(ElementIndex, 0);
            removeIndex(ElementIndex);
        }

        void removeIndex(intmax_t ElementIndex) {
            if (ElementIndex < 0) ElementIndex += NodeSize;
            Collections::doCheckGE(ElementIndex, 0);
            Collections::doCheckL(ElementIndex, NodeSize);
            LinkedNode *NodeCurrent;
            if (NodeSize == 1) {
                NodeAllocator.releaseObject(NodeHead);
                NodeHead = NodeTail = nullptr;
            } else if (ElementIndex == 0) {
                NodeHead = NodeHead->NodeNext;
                NodeAllocator.releaseObject(NodeHead->NodePrevious);
                NodeHead->NodePrevious = nullptr;
            } else if (ElementIndex == NodeSize - 1) {
                NodeTail = NodeTail->NodePrevious;
                NodeAllocator.releaseObject(NodeTail->NodeNext);
                NodeTail->NodeNext = nullptr;
            } else if (ElementIndex < (NodeSize >> 1)) {
                NodeCurrent = NodeHead;
                while (--ElementIndex) NodeCurrent = NodeCurrent->NodeNext;
                NodeCurrent->NodeNext = NodeCurrent->NodeNext->NodeNext;
                NodeAllocator.releaseObject(NodeCurrent->NodeNext->NodePrevious);
                NodeCurrent->NodeNext->NodePrevious = NodeCurrent;
            } else {
                NodeCurrent = NodeTail;
                ElementIndex = NodeSize - ElementIndex - 1;
                while (--ElementIndex) NodeCurrent = NodeCurrent->NodePrevious;
                NodeCurrent->NodePrevious = NodeCurrent->NodePrevious->NodePrevious;
                NodeAllocator.releaseObject(NodeCurrent->NodePrevious->NodeNext);
                NodeCurrent->NodePrevious->NodeNext = NodeCurrent;
            }
            --NodeSize;
        }

        void setElement(intmax_t ElementIndex, const E &ElementSource) {
            if (ElementIndex < 0) ElementIndex += NodeSize;
            Collections::doCheckGE(ElementIndex, 0);
            Collections::doCheckL(ElementIndex, NodeSize);
            LinkedNode *NodeCurrent;
            if (ElementIndex < (NodeSize >> 1)) {
                NodeCurrent = NodeHead;
                while (ElementIndex--) NodeCurrent = NodeCurrent->NodeNext;
            } else {
                NodeCurrent = NodeTail;
                ElementIndex = NodeSize - ElementIndex - 1;
                while (ElementIndex--) NodeCurrent = NodeCurrent->NodePrevious;
            }
            NodeCurrent->NodeValue = ElementSource;
        }

        String toString() const noexcept override {
            StringBuilder CharacterStream;
            CharacterStream.addCharacter(u'[');
            if (NodeHead) {
                auto *NodeCurrent(NodeHead);
                while (NodeCurrent->NodeNext) {
                    CharacterStream.addString(String::valueOf(NodeCurrent->NodeValue));
                    CharacterStream.addCharacter(u',');
                    NodeCurrent = NodeCurrent->NodeNext;
                }
                CharacterStream.addString(String::valueOf(NodeCurrent->NodeValue));
            }
            CharacterStream.addCharacter(u']');
            return CharacterStream.toString();
        }

        DoubleLinkedIterator<E> begin() const noexcept {
            return NodeHead;
        }

        DoubleLinkedIterator<E> end() const noexcept {
            return {};
        }
    };

    template<typename>
    class Function final : public Object {};

    template<typename Tr, typename ...Ts>
    class Function<Tr(Ts...)> final : public Object {
    private:
        struct FunctionDescriptorBase {
        public:
            virtual ~FunctionDescriptorBase() = default;

            virtual Tr doCall(Ts...) = 0;

            virtual FunctionDescriptorBase *doClone() const = 0;
        } *DescriptorHandle = nullptr;

        template<typename F>
        struct FunctionDescriptor : public FunctionDescriptorBase {
        private:
            F DescriptorHandle;
        public:
            constexpr FunctionDescriptor(F DescriptorHandleSource) noexcept: DescriptorHandle(DescriptorHandleSource) {}

            Tr doCall(Ts ...ParameterList) override {
                return DescriptorHandle(Objects::doForward<Ts>(ParameterList)...);
            }

            FunctionDescriptorBase *doClone() const override {
                return new FunctionDescriptor<F>(DescriptorHandle);
            }
        };

    public:
        doEnableCopyAssignConstruct(Function)

        doEnableMoveAssignConstruct(Function)

        template<typename F>
        requires ::std::is_invocable_r_v<Tr, F, Ts...>
        Function(F FunctionObject) noexcept : DescriptorHandle(new FunctionDescriptor<F>(Objects::doMove(FunctionObject))) {}

        ~Function() {
            if (DescriptorHandle) {
                delete DescriptorHandle;
                DescriptorHandle = nullptr;
            }
        }

        void doAssign(const Function<Tr(Ts...)> &FunctionSource) {
            if (Objects::getAddress(FunctionSource) == this) return;
            delete DescriptorHandle;
            if (FunctionSource.DescriptorHandle) DescriptorHandle = FunctionSource.DescriptorHandle->doClone();
            else DescriptorHandle = nullptr;
        }

        void doAssign(Function<Tr(Ts...)> &&FunctionSource) noexcept {
            if (Objects::getAddress(FunctionSource) == this) return;
            DescriptorHandle = FunctionSource.DescriptorHandle;
            FunctionSource.DescriptorHandle = nullptr;
        }

        Tr doCall(Ts ...FunctionParameter) {
            if (!DescriptorHandle) [[unlikely]]
                doThrowChecked(Exception, u"Function<Tr(Ts...)>::doCall(Ts...) DescriptorHandle"_S);
            return DescriptorHandle->doCall(Objects::doForward<Ts>(FunctionParameter)...);
        }

        Tr operator()(Ts &&...FunctionParameter) {
            if (!DescriptorHandle) [[unlikely]]
                doThrowChecked(Exception, u"Function<Tr(Ts...)>::operator()(Ts...) DescriptorHandle"_S);
            return DescriptorHandle->doCall(Objects::doForward<Ts>(FunctionParameter)...);
        }
    };

    class Functions final : public NonConstructable {
    public:
        template<int> struct PlaceHolder {};
    private:
        template<int...>
        struct IndexTuple {};

        template<int N, int ...I>
        struct MakeIndexTuple : MakeIndexTuple<N-1, N-1, I...> {};

        template<int ...I>
        struct MakeIndexTuple<0, I...> {
            typedef IndexTuple<I...> Type;
        };

        template<typename>
        class FunctionCore {};

        template<typename Tf, typename ...Tp>
        class FunctionCore<Tf(Tp...)> {
        private:
            ::std::decay_t<Tf> FunctionObject;
            ::std::tuple<::std::decay_t<Tp>...> FunctionParameter;

            template<typename Tv, typename Tt>
            auto doSelect(Tv &&TupleValue, Tt&) {
                return Objects::doForward<Tv>(TupleValue);
            }

            template<int I, typename Tt>
            auto doSelect(PlaceHolder<I>&, Tt &TupleObject) {
                return ::std::get<I-1>(TupleObject);
            }

            template<typename Tt, int ...I>
            auto doCall(Tt &&FunctionTuple, IndexTuple<I...>) {
                return Functions::doInvoke(FunctionObject, doSelect(::std::get<I>(FunctionParameter), FunctionTuple)...);
            }
        public:
            template<typename TfS, typename ...TpS>
            constexpr explicit FunctionCore(TfS &&FunctionSource, TpS &&...FunctionParameterSource) : FunctionObject(Objects::doForward<TfS>(FunctionSource)), FunctionParameter(Objects::doForward<TpS>(FunctionParameterSource)...) {}

            template<typename ...TpS>
            auto doCall(TpS &&...FunctionParameterList) {
                return doCall(::std::forward_as_tuple(Objects::doForward<TpS>(FunctionParameterList)...), typename MakeIndexTuple<sizeof...(Tp)>::Type());
            }

            template<typename ...TpS>
            auto operator()(TpS &&...FunctionParameterList) {
                return doCall(Objects::doForward<TpS>(FunctionParameterList)...);
            }
        };

        template<typename Tf, typename Tt, size_t ...I>
        static auto doInvokeTuple(Tf &&FunctionSource, Tt &&FunctionParameter, IndexTuple<I...>) {
            return doInvoke(Objects::doForward<Tf>(FunctionSource), ::std::get<I>(Objects::doForward<Tt>(FunctionParameter))...);
        }
    public:
        template<typename Tf, typename ...Tp>
        static auto doBind(Tf &&FunctionSource, Tp &&...FunctionParameter) {
            return FunctionCore<Tf(Tp...)>(Objects::doForward<Tf>(FunctionSource),
                                           Objects::doForward<Tp>(FunctionParameter)...);
        }

        template<typename Tf, typename Tc, typename ...Tp> requires (::std::is_member_function_pointer_v<Type::noReference<Tf>>) && (Type::isPointer<Type::noReference<Tc>>)
        static auto doInvoke(Tf &&FunctionSource, Tc &&FunctionClass, Tp &&...FunctionParameter) {
            return (Objects::doForward<Tc>(FunctionClass)->*FunctionSource)(Objects::doForward<Tp>(FunctionParameter)...);
        }

        template<typename Tf, typename Tc, typename ...Tp> requires (::std::is_member_function_pointer_v<Type::noReference<Tf>>) && (!Type::isPointer<Type::noReference<Tc>>)
        static auto doInvoke(Tf &&FunctionSource, Tc &&FunctionClass, Tp &&...FunctionParameter) {
            return (Objects::doForward<Tc>(FunctionClass).*FunctionSource)(Objects::doForward<Tp>(FunctionParameter)...);
        }

        template<typename Tf, typename Tc> requires (::std::is_member_object_pointer_v<Type::noReference<Tf>>) && (Type::isPointer<Type::noReference<Tc>>)
        static auto doInvoke(Tf &&FunctionSource, Tc &&FunctionClass) {
            return Objects::doForward<Tc>(FunctionClass)->*FunctionSource;
        }

        template<typename Tf, typename Tc> requires (::std::is_member_object_pointer_v<Type::noReference<Tf>>) && (!Type::isPointer<Type::noReference<Tc>>)
        static auto doInvoke(Tf &&FunctionSource, Tc &&FunctionClass) {
            return Objects::doForward<Tc>(FunctionClass).*FunctionSource;
        }

        template<typename Tf, typename ...Tp> requires (!::std::is_member_pointer_v<Type::noReference<Tf>>) && (!Type::isPointer<Type::noReference<Tf>>)
        static auto doInvoke(Tf &&FunctionSource, Tp &&...FunctionParameter) {
            return FunctionSource(Objects::doForward<Tp>(FunctionParameter)...);
        }

        template<typename Tf, typename ...Tp> requires Type::isPointer<Type::noReference<Tf>>
        static auto doInvoke(Tf &&FunctionSource, Tp &&...FunctionParameter) {
            return (*FunctionSource)(Objects::doForward<Tp>(FunctionParameter)...);
        }

        template<typename Tf, typename Tt>
        static auto doInvokeTuple(Tf &&FunctionSource, Tt &&FunctionParameter) {
            return doInvokeTuple(Objects::doForward<Tf>(FunctionSource), Objects::doForward<Tt>(FunctionParameter), typename MakeIndexTuple<::std::tuple_size_v<Type::noReference<Tt>>>::Type());
        }
    };

    template<typename T>
    class Optional final : public Object {
    private:
        alignas(alignof(T)) uint8_t OptionalData[sizeof(T)]{};
        bool OptionalValue = false;

        template<typename ...Ts>
        void doCreate(Ts &&...ParameterList) {
            new(OptionalData) T(Objects::doForward<Ts>(ParameterList)...);
            OptionalValue = true;
        }

    public:
        doEnableCopyAssignConstruct(Optional)

        doEnableCopyAssignParameterConstruct(Optional, T)

        doEnableMoveAssignConstruct(Optional)

        doEnableMoveAssignParameterConstruct(Optional, T)

        constexpr Optional() noexcept = default;

        ~Optional() {
            doReset();
        }

        void doAssign(const T &OptionalSource) {
            doReset();
            doCreate(OptionalSource);
        }

        void doAssign(T &&OptionalSource) {
            doReset();
            doCreate(Objects::doMove(OptionalSource));
        }

        void doAssign(const Optional<T> &OptionalSource) {
            doReset();
            if (OptionalSource.hasValue() && Objects::getAddress(OptionalSource) != this)
                doCreate(OptionalSource.getValue());
        }

        void doAssign(Optional<T> &&OptionalSource) {
            doReset();
            if (OptionalSource.hasValue() && Objects::getAddress(OptionalSource) != this) {
                doCreate(Objects::doMove(OptionalSource.getValue()));
                OptionalSource.doReset();
            }
        }

        void doReset() {
            if (hasValue()) {
                OptionalValue = false;
                ((T *) OptionalData)->~T();
            }
        }

        T getValue() const {
            if (!hasValue()) [[unlikely]]
                doThrowChecked(Exception, u"Optional<T>::getValue() hasValue"_S);
            return *((T *) OptionalData);
        }

        bool hasValue() const noexcept {
            return OptionalValue;
        }

        static Optional valueOf(T &&OptionalSource) {
            return {Objects::doForward<T>(OptionalSource)};
        }

        template<typename ...Ts>
        static Optional valueOf(Ts&& ...OptionalSource) {
            return {T(Objects::doForward<Ts>(OptionalSource)...)};
        }
    };

    template<typename T>
    class PointerShared final {
    private:
        T *PointerObject = nullptr;
        uintmax_t *PointerReference = nullptr;

        void doRelease() {
            if (PointerReference && --(*PointerReference) == 0) {
                delete PointerObject;
                delete PointerReference;
            }
        }

        PointerShared(T *PointerObjectSource, uintmax_t *PointerReferenceSource) : PointerReference(PointerReferenceSource) {
            PointerObject = PointerObjectSource;
        }
    public:
        doEnableCopyAssignConstruct(PointerShared)

        doEnableCopyAssignParameterConstruct(PointerShared, ::std::nullptr_t)

        doEnableMoveAssignConstruct(PointerShared)

        constexpr PointerShared() noexcept = default;

        template<typename Tp> requires Type::isConvertible<Tp, T>
        explicit PointerShared(Tp *PointerSource) : PointerObject(PointerSource), PointerReference(new uintmax_t(1)) {}

        ~PointerShared() {
            doRelease();
        }

        void doAssign(const PointerShared &PointerSource) {
            if (Objects::getAddress(PointerSource) == this) return;
            doRelease();
            PointerObject = PointerSource.PointerObject;
            PointerReference = PointerSource.PointerReference;
            ++(*PointerReference);
        }

        void doAssign(::std::nullptr_t) {
            doReset();
        }

        void doAssign(PointerShared &&PointerSource) {
            PointerShared(Objects::doMove(PointerSource)).doSwap(*this);
        }

        void doReset() noexcept {
            PointerShared().doSwap(*this);
        }

        template<typename Tp>
        void doReset(Tp *PointerSource) noexcept {
            PointerShared(PointerSource).doSwap(*this);
        }

        void doSwap(PointerShared &PointerSource) noexcept {
            Objects::doSwap(PointerObject, PointerSource.PointerObject);
            Objects::doSwap(PointerReference, PointerSource.PointerReference);
        }

        uintmax_t getReferenceCount() const noexcept {
            return PointerReference ? *PointerReference : 0;
        }

        T *getValue() const noexcept {
            return PointerObject;
        }

        bool hasValue() const noexcept {
            return PointerObject;
        }

        T &operator*() const {
            if (!hasValue()) [[unlikely]]
                doThrowChecked(Exception, u"PointerShared::operator*() hasValue"_S);
            return *PointerObject;
        }

        T *operator->() const noexcept {
            return PointerObject;
        }

        template<typename ...Ts>
        static PointerShared valueOf(Ts &&...PointerParameter) {
            return PointerShared(new T(Objects::doForward<Ts>(PointerParameter)...));
        }
    };

    template<typename T>
    class PointerWeak final {
    private:
        T *PointerObject = nullptr;
        uintmax_t *PointerReference = nullptr;

        template<typename>
        friend class PointerSharedThis;
    public:
        doEnableCopyAssignConstruct(PointerWeak)

        doEnableCopyAssignParameterConstruct(PointerWeak, PointerShared<T>)

        doEnableCopyAssignParameterConstruct(PointerWeak, ::std::nullptr_t)

        doEnableMoveAssignConstruct(PointerWeak)

        constexpr PointerWeak() noexcept = default;

        void doAssign(const PointerShared<T> &PointerSource) noexcept {
            if (Objects::getAddress(PointerSource) == this) return;
            PointerObject = PointerSource.PointerObject;
            PointerReference = PointerSource.PointerReference;
        }

        void doAssign(const PointerWeak &PointerSource) noexcept {
            if (Objects::getAddress(PointerSource) == this) return;
            PointerObject = PointerSource.doLock().getValue();
            PointerReference = PointerSource.PointerReference;
        }

        void doAssign(PointerWeak &&PointerSource) noexcept {
            PointerWeak(Objects::doMove(PointerSource)).doSwap(*this);
        }

        void doAssign(::std::nullptr_t) noexcept {
            doReset();
        }

        PointerShared<T> doLock() const {
            return isExpired() ? PointerShared<T>() : PointerShared<T>(PointerObject, PointerReference);
        }

        void doReset() noexcept {
            PointerWeak().doSwap(*this);
        }

        void doSwap(PointerWeak &PointerSource) noexcept {
            Objects::doSwap(PointerObject, PointerSource.PointerObject);
            Objects::doSwap(PointerReference, PointerSource.PointerReference);
        }

        uintmax_t getReferenceCount() const noexcept {
            return PointerReference ? *PointerReference : 0;
        }

        T *getValue() const noexcept {
            return PointerObject;
        }

        bool hasValue() const noexcept {
            return PointerObject;
        }

        bool isExpired() const noexcept {
            return getReferenceCount() == 0;
        }

        T &operator*() const {
            if (!hasValue()) [[unlikely]]
                doThrowChecked(Exception, u"PointerWeak::operator*() hasValue"_S);
            return *PointerObject;
        }

        T *operator->() const noexcept {
            return PointerObject;
        }
    };

    template<typename T>
    class PointerUnique final : public NonCopyable {
    private:
        T *PointerObject = nullptr;
    public:
        doEnableCopyAssignParameterConstruct(PointerUnique, ::std::nullptr_t)

        doEnableMoveAssignConstruct(PointerUnique)

        constexpr PointerUnique() noexcept = default;

        template<typename Tp> requires Type::isConvertible<Tp, T>
        explicit PointerUnique(Tp *PointerSource) : PointerObject(PointerSource) {}

        void doAssign(::std::nullptr_t) {
            doReset();
        }

        void doAssign(PointerUnique &&PointerSource) {
            doReset(PointerSource.doRelease());
        }

        T *doRelease() noexcept {
            T *PointerBackup(PointerObject);
            PointerObject = nullptr;
            return PointerBackup;
        }

        void doReset() {
            delete PointerObject;
            PointerObject = nullptr;
        }

        template<typename Tp> requires Type::isConvertible<Tp, T>
        void doReset(Tp *PointerSource) {
            delete PointerObject;
            PointerObject = Objects::doMove(PointerSource);
        }

        T *getValue() const noexcept {
            return PointerObject;
        }

        bool hasValue() const noexcept {
            return PointerObject;
        }

        T &operator*() const {
            if (!hasValue()) [[unlikely]]
                doThrowChecked(Exception, u"PointerUnique::operator*() hasValue"_S);
            return *PointerObject;
        }

        T *operator->() const noexcept {
            return PointerObject;
        }

        template<typename ...Ts>
        static PointerUnique valueOf(Ts &&...PointerParameter) {
            return PointerUnique(new T(Objects::doForward<Ts>(PointerParameter)...));
        }
    };

    template<typename K, typename V>
    class RedBlackTree : public Object {
    protected:
        enum class NodeColorEnumeration {
            ColorBlack,
            ColorRed
        };

        struct RedBlackNode final {
            K NodeKey;
            V NodeValue;
            NodeColorEnumeration NodeColor;
            RedBlackNode *NodeChildLeft = nullptr, *NodeChildRight = nullptr, *NodeParent = nullptr;

            constexpr RedBlackNode(const K &NodeKeySource, const V &NodeValueSource) noexcept: NodeKey(NodeKeySource), NodeValue(NodeValueSource), NodeColor(NodeColorEnumeration::ColorRed) {}
        } *NodeRoot = nullptr;
        mutable MemoryAllocator<RedBlackNode> NodeAllocator;

        void deleteNode(RedBlackNode *NodeCurrent) {
            if (!NodeCurrent) return;
            deleteNode(NodeCurrent->NodeChildLeft);
            deleteNode(NodeCurrent->NodeChildRight);
            NodeAllocator.releaseObject(NodeCurrent);
        }

        template<typename F>
        static void doOrderCore(RedBlackNode *NodeCurrent, F Operation) {
            if (!NodeCurrent) return;
            Operation(NodeCurrent->NodeKey, NodeCurrent->NodeValue);
            doOrderCore(NodeCurrent->NodeChildLeft, Operation);
            doOrderCore(NodeCurrent->NodeChildRight, Operation);
        }

        void doRotateLeft(RedBlackNode *NodeTarget) noexcept {
            auto *NodeChildRight = NodeTarget->NodeChildRight;

            NodeTarget->NodeChildRight = NodeChildRight->NodeChildLeft;
            if (NodeChildRight->NodeChildLeft) NodeChildRight->NodeChildLeft->NodeParent = NodeTarget;

            NodeChildRight->NodeParent = NodeTarget->NodeParent;

            if (!NodeTarget->NodeParent) NodeRoot = NodeChildRight;
            else if (NodeTarget->NodeParent->NodeChildLeft == NodeTarget)
                NodeTarget->NodeParent->NodeChildLeft = NodeChildRight;
            else NodeTarget->NodeParent->NodeChildRight = NodeChildRight;

            NodeChildRight->NodeChildLeft = NodeTarget;
            NodeTarget->NodeParent = NodeChildRight;
        }

        void doRotateRight(RedBlackNode *NodeTarget) noexcept {
            auto *NodeChildLeft = NodeTarget->NodeChildLeft;

            NodeTarget->NodeChildLeft = NodeChildLeft->NodeChildRight;
            if (NodeChildLeft->NodeChildRight) NodeChildLeft->NodeChildRight->NodeParent = NodeTarget;

            NodeChildLeft->NodeParent = NodeTarget->NodeParent;

            if (!NodeTarget->NodeParent) NodeRoot = NodeChildLeft;
            else if (NodeTarget->NodeParent->NodeChildLeft == NodeTarget)
                NodeTarget->NodeParent->NodeChildLeft = NodeChildLeft;
            else NodeTarget->NodeParent->NodeChildRight = NodeChildLeft;

            NodeChildLeft->NodeChildRight = NodeTarget;
            NodeTarget->NodeParent = NodeChildLeft;
        }

        static RedBlackNode *doSearchCore(RedBlackNode *NodeCurrent, const K &NodeKey) noexcept {
            if (!NodeCurrent) return nullptr;
            auto NodeRelation = Objects::doCompare(NodeCurrent->NodeKey, NodeKey);
            if (NodeRelation) return doSearchCore(NodeRelation < 0 ? NodeCurrent->NodeChildRight : NodeCurrent->NodeChildLeft, NodeKey);
            else return NodeCurrent;
        }

        static uintmax_t getHeightCore(RedBlackNode *NodeCurrent) noexcept {
            if (NodeCurrent->NodeChildLeft && NodeCurrent->NodeChildRight)
                return Objects::getMaximum(getHeightCore(NodeCurrent->NodeChildLeft),
                                           getHeightCore(NodeCurrent->NodeChildRight)) + 1;
            if (NodeCurrent->NodeChildLeft) return getHeightCore(NodeCurrent->NodeChildLeft) + 1;
            if (NodeCurrent->NodeChildRight) return getHeightCore(NodeCurrent->NodeChildRight) + 1;
            return 1;
        }

        static uintmax_t getSizeCore(RedBlackNode *NodeCurrent) noexcept {
            if (!NodeCurrent) return 0;
            return getSizeCore(NodeCurrent->NodeChildLeft) + getSizeCore(NodeCurrent->NodeChildRight) + 1;
        }

        template<typename, typename>
        friend
        class TreeMap;

        template<typename>
        friend
        class TreeSet;
    public:
        doEnableCopyAssignConstruct(RedBlackTree)

        doEnableMoveAssignConstruct(RedBlackTree)

        constexpr RedBlackTree() noexcept = default;

        ~RedBlackTree() {
            doClear();
        }

        void doAssign(const RedBlackTree &TreeSource) {
            if (Objects::getAddress(TreeSource) == this) return;
            doClear();
            TreeSource.doOrderCore(TreeSource.NodeRoot, [&](const K &NodeKeySource, const V &NodeValueSource) {
                doInsert(NodeKeySource, NodeValueSource);
            });
        }

        void doAssign(RedBlackTree &&TreeSource) {
            if (Objects::getAddress(TreeSource) == this) return;
            doClear();
            NodeRoot = TreeSource.NodeRoot;
            TreeSource.NodeRoot = nullptr;
        }

        void doClear() {
            deleteNode(NodeRoot);
            NodeRoot = nullptr;
        }

        void doInsert(const K &NodeKey, const V &NodeValue) noexcept {
            auto *NodeTarget = NodeAllocator.acquireObject(NodeKey, NodeValue);
            if (!NodeRoot) {
                NodeTarget->NodeColor = NodeColorEnumeration::ColorBlack;
                NodeRoot = NodeTarget;
                return;
            }
            auto *NodeParent = NodeRoot;
            while (NodeParent) {
                auto NodeRelation = Objects::doCompare(NodeTarget->NodeKey, NodeParent->NodeKey);
                if (NodeRelation == 0) {
                    NodeParent->NodeValue = NodeTarget->NodeValue;
                    return;
                } else if (NodeRelation > 0) {
                    if (!NodeParent->NodeChildRight) {
                        NodeParent->NodeChildRight = NodeTarget;
                        NodeTarget->NodeParent = NodeParent;
                        break;
                    }
                    NodeParent = NodeParent->NodeChildRight;
                } else {
                    if (!NodeParent->NodeChildLeft) {
                        NodeParent->NodeChildLeft = NodeTarget;
                        NodeTarget->NodeParent = NodeParent;
                        break;
                    }
                    NodeParent = NodeParent->NodeChildLeft;
                }
            }
            while ((NodeParent = NodeTarget->NodeParent) && NodeParent->NodeColor == NodeColorEnumeration::ColorRed)
                if (NodeParent == NodeParent->NodeParent->NodeChildLeft) {
                    RedBlackNode *NodeUncle = NodeParent->NodeParent->NodeChildRight;
                    if (NodeUncle && NodeUncle->NodeColor == NodeColorEnumeration::ColorRed) {
                        NodeParent->NodeColor = NodeColorEnumeration::ColorBlack;
                        NodeParent->NodeParent->NodeColor = NodeColorEnumeration::ColorRed;
                        NodeUncle->NodeColor = NodeColorEnumeration::ColorBlack;
                        NodeTarget = NodeParent->NodeParent;
                        continue;
                    }
                    if (NodeParent->NodeChildRight == NodeTarget) {
                        doRotateLeft(NodeParent);
                        Objects::doSwap(NodeTarget, NodeParent);
                    }
                    NodeParent->NodeColor = NodeColorEnumeration::ColorBlack;
                    NodeParent->NodeParent->NodeColor = NodeColorEnumeration::ColorRed;
                    doRotateRight(NodeParent->NodeParent);
                } else {
                    RedBlackNode *NodeUncle = NodeParent->NodeParent->NodeChildLeft;
                    if (NodeUncle && NodeUncle->NodeColor == NodeColorEnumeration::ColorRed) {
                        NodeParent->NodeColor = NodeColorEnumeration::ColorBlack;
                        NodeParent->NodeParent->NodeColor = NodeColorEnumeration::ColorRed;
                        NodeUncle->NodeColor = NodeColorEnumeration::ColorBlack;
                        NodeTarget = NodeParent->NodeParent;
                        continue;
                    }
                    if (NodeParent->NodeChildLeft == NodeTarget) {
                        doRotateRight(NodeParent);
                        Objects::doSwap(NodeTarget, NodeParent);
                    }
                    NodeParent->NodeColor = NodeColorEnumeration::ColorBlack;
                    NodeParent->NodeParent->NodeColor = NodeColorEnumeration::ColorRed;
                    doRotateLeft(NodeParent->NodeParent);
                }
            NodeRoot->NodeColor = NodeColorEnumeration::ColorBlack;
        }

        template<typename F>
        void doOrder(F Operation) const {
            doOrderCore(NodeRoot, Operation);
        }

        void doRemove(const K &NodeKey) noexcept {
            RedBlackNode *NodeTarget = doSearchCore(NodeRoot, NodeKey);
            if (!NodeTarget) return;
            RedBlackNode *NodeChild, *NodeParent;
            NodeColorEnumeration NodeColor;
            if (NodeTarget->NodeChildLeft && NodeTarget->NodeChildRight) {
                RedBlackNode *NodeReplace = NodeTarget->NodeChildRight;
                while (NodeReplace->NodeChildLeft) NodeReplace = NodeReplace->NodeChildLeft;
                if (!NodeTarget->NodeParent) NodeRoot = NodeReplace;
                else if (NodeTarget->NodeParent->NodeChildLeft == NodeTarget)
                    NodeTarget->NodeParent->NodeChildLeft = NodeReplace;
                else NodeTarget->NodeParent->NodeChildRight = NodeReplace;
                NodeChild = NodeReplace->NodeChildRight;
                NodeParent = NodeReplace->NodeParent;
                NodeColor = NodeReplace->NodeColor;
                if (NodeParent == NodeTarget) NodeParent = NodeReplace;
                else {
                    if (NodeChild) NodeChild->NodeParent = NodeParent;
                    NodeParent->NodeChildLeft = NodeChild;
                    NodeReplace->NodeChildRight = NodeTarget->NodeChildRight;
                    NodeTarget->NodeChildRight->NodeParent = NodeReplace;
                }
                NodeReplace->NodeChildLeft = NodeTarget->NodeChildLeft;
                NodeReplace->NodeColor = NodeTarget->NodeColor;
                NodeReplace->NodeParent = NodeTarget->NodeParent;
                NodeTarget->NodeChildLeft->NodeParent = NodeReplace;
                if (NodeColor == NodeColorEnumeration::ColorBlack) goto doRemoveFixup;
                return;
            }
            NodeChild = NodeTarget->NodeChildLeft ? NodeTarget->NodeChildLeft : NodeTarget->NodeChildRight;
            NodeParent = NodeTarget->NodeParent;
            NodeColor = NodeTarget->NodeColor;
            if (NodeChild) NodeChild->NodeParent = NodeParent;
            if (!NodeParent) NodeRoot = NodeChild;
            else if (NodeParent->NodeChildLeft == NodeTarget)
                NodeParent->NodeChildLeft = NodeChild;
            else NodeParent->NodeChildRight = NodeChild;
            if (NodeColor == NodeColorEnumeration::ColorRed) return;
            doRemoveFixup:
            while ((!NodeChild || NodeChild->NodeColor == NodeColorEnumeration::ColorBlack) && NodeChild != NodeRoot)
                if (NodeParent->NodeChildLeft == NodeChild) {
                    RedBlackNode *NodeUncle = NodeParent->NodeChildRight;
                    if (NodeUncle->NodeColor == NodeColorEnumeration::ColorRed) {
                        NodeParent->NodeColor = NodeColorEnumeration::ColorRed;
                        NodeUncle->NodeColor = NodeColorEnumeration::ColorBlack;
                        doRotateLeft(NodeParent);
                        NodeUncle = NodeParent->NodeChildRight;
                    }
                    if ((!NodeUncle->NodeChildLeft ||
                         NodeUncle->NodeChildLeft->NodeColor == NodeColorEnumeration::ColorBlack) &&
                        (!NodeUncle->NodeChildRight ||
                         NodeUncle->NodeChildRight->NodeColor == NodeColorEnumeration::ColorBlack)) {
                        NodeUncle->NodeColor = NodeColorEnumeration::ColorRed;
                        NodeChild = NodeParent;
                        NodeParent = NodeChild->NodeParent;
                    } else {
                        if (!NodeUncle->NodeChildRight ||
                            NodeUncle->NodeChildRight->NodeColor == NodeColorEnumeration::ColorBlack) {
                            NodeUncle->NodeChildLeft->NodeColor = NodeColorEnumeration::ColorBlack;
                            NodeUncle->NodeColor = NodeColorEnumeration::ColorRed;
                            doRotateRight(NodeUncle);
                            NodeUncle = NodeParent->NodeChildRight;
                        }
                        NodeUncle->NodeColor = NodeParent->NodeColor;
                        NodeParent->NodeColor = NodeColorEnumeration::ColorBlack;
                        NodeUncle->NodeChildRight->NodeColor = NodeColorEnumeration::ColorBlack;
                        doRotateLeft(NodeParent);
                        NodeChild = NodeRoot;
                        break;
                    }
                } else {
                    RedBlackNode *NodeUncle = NodeParent->NodeChildLeft;
                    if (NodeUncle->NodeColor == NodeColorEnumeration::ColorRed) {
                        NodeParent->NodeColor = NodeColorEnumeration::ColorRed;
                        NodeUncle->NodeColor = NodeColorEnumeration::ColorBlack;
                        doRotateRight(NodeParent);
                        NodeUncle = NodeParent->NodeChildLeft;
                    }
                    if ((!NodeUncle->NodeChildLeft ||
                         NodeUncle->NodeChildLeft->NodeColor == NodeColorEnumeration::ColorBlack) &&
                        (!NodeUncle->NodeChildRight ||
                         NodeUncle->NodeChildRight->NodeColor == NodeColorEnumeration::ColorBlack)) {
                        NodeUncle->NodeColor = NodeColorEnumeration::ColorRed;
                        NodeChild = NodeParent;
                        NodeParent = NodeChild->NodeParent;
                    } else {
                        if (!NodeUncle->NodeChildLeft ||
                            NodeUncle->NodeChildLeft->NodeColor == NodeColorEnumeration::ColorBlack) {
                            NodeUncle->NodeChildRight->NodeColor = NodeColorEnumeration::ColorBlack;
                            NodeUncle->NodeColor = NodeColorEnumeration::ColorRed;
                            doRotateLeft(NodeUncle);
                            NodeUncle = NodeParent->NodeChildLeft;
                        }
                        NodeUncle->NodeColor = NodeParent->NodeColor;
                        NodeParent->NodeColor = NodeColorEnumeration::ColorBlack;
                        NodeUncle->NodeChildLeft->NodeColor = NodeColorEnumeration::ColorBlack;
                        doRotateRight(NodeParent);
                        NodeChild = NodeRoot;
                        break;
                    }
                }
            if (NodeChild) NodeChild->NodeColor = NodeColorEnumeration::ColorBlack;
        }

        Optional<V> doSearch(const K &NodeKey) const noexcept {
            RedBlackNode *NodeResult = doSearchCore(NodeRoot, NodeKey);
            if (NodeResult == nullptr) return {};
            return {NodeResult->NodeValue};
        }

        uintmax_t getHeight() const noexcept {
            return NodeRoot ? getHeightCore(NodeRoot) : 0;
        }

        uintmax_t getSize() const noexcept {
            return NodeRoot ? getSizeCore(NodeRoot) : 0;
        }
    };

    template<typename T>
    class Reference final {
    private:
        T *ReferenceData;

        template<typename TI>
        static TI *getAddress(TI &ReferenceSource) noexcept {
            return Objects::getAddress(ReferenceSource);
        }

        template<typename TI>
        static TI *getAddress(TI &&) noexcept = delete;

    public:
        template<typename TI>
        Reference(TI &&ReferenceSource) noexcept: ReferenceData(getAddress(Objects::doForward<TI>(ReferenceSource))) {}

        T &getValue() const noexcept {
            return *ReferenceData;
        }

        operator T&() const noexcept {
            return getValue();
        }
    };

    template<class T>
    Reference(T&) -> Reference<T>;

    template<typename E>
    class SingleLinkedIterator final {
    private:
        using LinkedNode = SingleLinkedNode<E>;
        LinkedNode *NodeCurrent;
    public:
        using difference_type = ptrdiff_t;
        using iterator_category = ::std::forward_iterator_tag;
        using value_type = E;

        constexpr SingleLinkedIterator() noexcept: NodeCurrent(nullptr) {}

        constexpr SingleLinkedIterator(SingleLinkedNode<E> *NodeSource) noexcept: NodeCurrent(NodeSource) {}

        SingleLinkedIterator &operator++() {
            NodeCurrent = NodeCurrent->NodeNext;
            return *this;
        }

        SingleLinkedIterator operator++(int) {
            LinkedNode *NodeBuffer = NodeCurrent;
            NodeCurrent = NodeCurrent->NodeNext;
            return {NodeBuffer};
        }

        E &operator*() const {
            return NodeCurrent->NodeValue;
        }

        bool operator==(const SingleLinkedIterator &IteratorSource) const noexcept {
            return NodeCurrent == IteratorSource.NodeCurrent;
        }

        bool operator!=(const SingleLinkedIterator &IteratorSource) const noexcept {
            return NodeCurrent != IteratorSource.NodeCurrent;
        }
    };

    /**
     * Support for discrete storage of objects
     */
    template<typename E>
    class SingleLinkedList final : public Object {
    protected:
        using LinkedNode = SingleLinkedNode<E>;
        LinkedNode *NodeHead = nullptr, *NodeTail = nullptr;
        intmax_t NodeSize = 0;
        mutable MemoryAllocator<LinkedNode> NodeAllocator;

        template<typename ...Es>
        void doInitialize(E ElementCurrent, Es ...ElementList) noexcept {
            addElement(ElementCurrent);
            if constexpr (sizeof...(ElementList)) doInitialize(ElementList...);
        }
    public:
        doEnableCopyAssignConstruct(SingleLinkedList)

        doEnableMoveAssignConstruct(SingleLinkedList)

        constexpr SingleLinkedList() noexcept = default;

        template<typename ...Es>
        explicit SingleLinkedList(Es ...ElementList) noexcept {
            doInitialize(ElementList...);
        }

        SingleLinkedList(::std::initializer_list<E> ElementList) noexcept: NodeSize(ElementList.size()) {
            for (const E &ElementCurrent: ElementList) addElement(ElementCurrent);
        }

        template<typename II>
        SingleLinkedList(II ElementStart, II ElementStop) noexcept {
            Collections::doTraverse(ElementStart, ElementStop, [&](const E &ElementCurrent) {
                addElement(ElementCurrent);
            });
        }

        ~SingleLinkedList() {
            doClear();
        }

        void addElement(const E &ElementSource) noexcept {
            auto *NodeCurrent = NodeAllocator.acquireObject(ElementSource);
            if (!NodeHead) NodeHead = NodeTail = NodeCurrent;
            else {
                NodeTail->NodeNext = NodeCurrent;
                NodeTail = NodeCurrent;
            }
            ++NodeSize;
        }

        void addElement(intmax_t ElementIndex, const E &ElementSource) {
            if (ElementIndex < 0) ElementIndex += NodeSize + 1;
            Collections::doCheckGE(ElementIndex, 0);
            Collections::doCheckLE(ElementIndex, NodeSize);
            if (ElementIndex == NodeSize) {
                addElement(ElementSource);
                return;
            }
            auto *NodeTarget = NodeAllocator.acquireObject(ElementSource);
            if (ElementIndex == 0) {
                NodeTarget->NodeNext = NodeHead;
                NodeHead = NodeTarget;
                ++NodeSize;
                return;
            }
            auto *NodeCurrent(NodeHead);
            while (--ElementIndex) NodeCurrent = NodeCurrent->NodeNext;
            NodeTarget->NodeNext = NodeCurrent->NodeNext;
            NodeCurrent->NodeNext = NodeTarget;
            ++NodeSize;
        }

        void doAssign(const SingleLinkedList &ElementSource) noexcept {
            if (Objects::getAddress(ElementSource) == this) return;
            if (!isEmpty()) doClear();
            NodeSize = ElementSource.NodeSize;
            LinkedNode *NodeCurrent = ElementSource.NodeHead;
            while (NodeCurrent) {
                addElement(NodeCurrent->NodeValue);
                NodeCurrent = NodeCurrent->NodeNext;
            }
        }

        void doAssign(SingleLinkedList &&ElementSource) noexcept {
            if (Objects::getAddress(ElementSource) == this) return;
            if (!isEmpty()) doClear();
            NodeHead = ElementSource.NodeHead;
            NodeSize = ElementSource.NodeSize;
            NodeTail = ElementSource.NodeTail;
            ElementSource.NodeHead = ElementSource.NodeTail = nullptr;
            ElementSource.NodeSize = 0;
        }

        void doClear() {
            NodeSize = 0;
            NodeTail = nullptr;
            LinkedNode *NodeCurrent = NodeHead, *NodePrevious;
            NodeHead = nullptr;
            while (NodeCurrent) {
                NodePrevious = NodeCurrent;
                NodeCurrent = NodeCurrent->NodeNext;
                NodeAllocator.releaseObject(NodePrevious);
            }
        }

        intmax_t doCompare(const SingleLinkedList &ElementSource) const noexcept {
            return Collections::doCompare(begin(), NodeSize, ElementSource.begin(), ElementSource.NodeSize);
        }

        const E &getElement(intmax_t ElementIndex) const {
            if (ElementIndex < 0) ElementIndex += NodeSize;
            Collections::doCheckGE(ElementIndex, 0);
            Collections::doCheckL(ElementIndex, NodeSize);
            if (ElementIndex == NodeSize - 1) return NodeTail->NodeValue;
            auto *NodeCurrent(NodeHead);
            while (ElementIndex--) NodeCurrent = NodeCurrent->NodeNext;
            return NodeCurrent->NodeValue;
        }

        E &getElement(intmax_t ElementIndex) {
            if (ElementIndex < 0) ElementIndex += NodeSize;
            Collections::doCheckGE(ElementIndex, 0);
            Collections::doCheckL(ElementIndex, NodeSize);
            if (ElementIndex == NodeSize - 1) return NodeTail->NodeValue;
            auto *NodeCurrent(NodeHead);
            while (ElementIndex--) NodeCurrent = NodeCurrent->NodeNext;
            return NodeCurrent->NodeValue;
        }

        intmax_t getElementSize() const noexcept {
            return NodeSize;
        }

        intmax_t indexOf(const E &ElementSource) const noexcept {
            intmax_t NodeIndex = 0;
            auto *NodeCurrent(NodeHead);
            while (NodeCurrent && Objects::doCompare(NodeCurrent->NodeValue, ElementSource))
                NodeCurrent = NodeCurrent->NodeNext, ++NodeIndex;
            if (!NodeCurrent) return -1;
            return NodeIndex;
        }

        bool isContains(const E &ElementSource) const noexcept {
            auto *NodeCurrent(NodeHead);
            while (NodeCurrent) {
                if (!Objects::doCompare(NodeCurrent->NodeValue, ElementSource)) return true;
                NodeCurrent = NodeCurrent->NodeNext;
            }
            return false;
        }

        bool isEmpty() const noexcept {
            return !NodeSize;
        }

        void removeElement(const E &ElementSource) {
            intmax_t ElementIndex = indexOf(ElementSource);
            Collections::doCheckGE(ElementIndex, 0);
            removeIndex(ElementIndex);
        }

        void removeIndex(intmax_t ElementIndex) {
            if (ElementIndex < 0) ElementIndex += NodeSize;
            Collections::doCheckGE(ElementIndex, 0);
            Collections::doCheckL(ElementIndex, NodeSize);
            auto *NodeCurrent(NodeHead);
            if (ElementIndex == 0) {
                NodeHead = NodeHead->NodeNext;
                NodeAllocator.releaseObject(NodeCurrent);
                --NodeSize;
                return;
            }
            bool NodeLast = false;
            if (ElementIndex == NodeSize - 1) NodeLast = true;
            while (--ElementIndex) NodeCurrent = NodeCurrent->NodeNext;
            LinkedNode *NodeNext = NodeCurrent->NodeNext->NodeNext;
            NodeAllocator.releaseObject(NodeCurrent->NodeNext);
            NodeCurrent->NodeNext = NodeNext;
            if (NodeLast) NodeTail = NodeCurrent;
            --NodeSize;
        }

        void setElement(intmax_t ElementIndex, const E &ElementSource) {
            if (ElementIndex < 0) ElementIndex += NodeSize;
            Collections::doCheckGE(ElementIndex, 0);
            Collections::doCheckL(ElementIndex, NodeSize);
            auto *NodeCurrent(NodeHead);
            while (ElementIndex--) NodeCurrent = NodeCurrent->NodeNext;
            NodeCurrent->NodeValue = ElementSource;
        }

        String toString() const noexcept override {
            StringBuilder CharacterStream;
            CharacterStream.addCharacter(u'[');
            if (NodeHead) {
                auto *NodeCurrent(NodeHead);
                while (NodeCurrent->NodeNext) {
                    CharacterStream.addString(String::valueOf(NodeCurrent->NodeValue));
                    CharacterStream.addCharacter(u',');
                    NodeCurrent = NodeCurrent->NodeNext;
                }
                CharacterStream.addString(String::valueOf(NodeCurrent->NodeValue));
            }
            CharacterStream.addCharacter(u']');
            return CharacterStream.toString();
        }

        SingleLinkedIterator<E> begin() const noexcept {
            return {NodeHead};
        }

        SingleLinkedIterator<E> end() const noexcept {
            return {};
        }
    };

    template<typename K, typename V>
    class TreeMap final : protected RedBlackTree<K, V> {
    public:
        const char *getClassName() const noexcept override {
            return "TreeMap";
        }

        const V &getElement(const K &MapKey) const {
            auto *MapNode = this->doSearchCore(this->NodeRoot, MapKey);
            if (!MapNode) [[unlikely]]
                doThrowChecked(IndexException, u"TreeMap<K, V>::getElement(const K&) doSearchCore"_S);
            return MapNode->NodeValue;
        }

        uintmax_t getElementSize() const noexcept {
            return this->getSize();
        }

        bool isContainsKey(const K &MapKey) const noexcept override {
            return this->doSearchCore(this->NodeRoot, MapKey);
        }

        bool isContainsValue(const V &MapValue) const noexcept override {
            bool ValueResult = false;
            this->doOrderCore(this->NodeRoot, [&](const K &, const V &MapValueCurrent) {
                if (!Objects::doCompare(MapValue, MapValueCurrent)) ValueResult = true;
            });
            return ValueResult;
        }

        bool isEmpty() const noexcept {
            return !this->NodeRoot;
        }

        void removeMapping(const K &MapKey) {
            if (!this->doSearchCore(this->NodeRoot, MapKey)) [[unlikely]]
                doThrowChecked(IndexException, u"TreeMap<K(Comparable),V>::removeMapping(const K&) MapKey"_S);
            this->doRemove(MapKey);
        }

        void setMapping(const K &MapKey, const V &MapValue) {
            if (this->doSearchCore(this->NodeRoot, MapKey))
                this->doSearchCore(this->NodeRoot, MapKey)->NodeValue = MapValue;
            else this->doInsert(MapKey, MapValue);
        }
    };

    template<typename E>
    class TreeSet final : protected RedBlackTree<E, ::std::nullptr_t> {
    private:
        static void toString(const typename RedBlackTree<E, ::std::nullptr_t>::RedBlackNode *NodeCurrent,
                             StringBuilder &CharacterStream) noexcept {
            if (!NodeCurrent) return;
            CharacterStream.addString(String::valueOf(NodeCurrent->NodeKey));
            CharacterStream.addCharacter(u' ');
            toString(NodeCurrent->NodeChildLeft, CharacterStream);
            toString(NodeCurrent->NodeChildRight, CharacterStream);
        }

    public:
        void addElement(const E &ElementSource) noexcept {
            this->doInsert(ElementSource, nullptr);
        }

        TreeSet doDifference(const TreeSet &SetSource) const noexcept {
            TreeSet SetResult;
            this->doOrderCore(this->NodeRoot, [&](const E &ElementSource, ::std::nullptr_t) {
                if (!SetSource.isContains(ElementSource)) SetResult.addElement(ElementSource);
            });
            return SetResult;
        }

        TreeSet doIntersection(const TreeSet &SetSource) const noexcept {
            TreeSet SetResult;
            this->doOrderCore(this->NodeRoot, [&](const E &ElementSource, ::std::nullptr_t) {
                if (SetSource.isContains(ElementSource)) SetResult.addElement(ElementSource);
            });
            return SetResult;
        }

        TreeSet doUnion(const TreeSet &SetSource) const noexcept {
            TreeSet SetResult;
            this->doOrderCore(this->NodeRoot, [&](const E &ElementSource, ::std::nullptr_t) {
                SetResult.addElement(ElementSource);
            });
            SetSource.doOrderCore(SetSource.NodeRoot, [&](const E &ElementSource, ::std::nullptr_t) {
                SetResult.addElement(ElementSource);
            });
            return SetResult;
        }

        const char *getClassName() const noexcept override {
            return "TreeSet";
        }

        uintmax_t getElementSize() const noexcept {
            return this->getSize();
        }

        bool isContains(const E &ElementSource) const noexcept {
            return this->doSearchCore(this->NodeRoot, ElementSource);
        }

        bool isEmpty() const noexcept {
            return !this->NodeRoot;
        }

        void removeElement(const E &ElementSource) {
            if (!isContains(ElementSource)) [[unlikely]]
                doThrowChecked(IndexException, u"TreeObject::removeElement(const E&) isContains"_S);
            this->doRemove(ElementSource);
        }

        String toString() const noexcept override {
            StringBuilder CharacterStream;
            CharacterStream.addCharacter(u'{');
            toString(this->NodeRoot, CharacterStream);
            CharacterStream.addCharacter(u'}');
            return CharacterStream.toString();
        }
    };

    namespace {
        template<size_t V, size_t ...Vs>
        constexpr size_t getMaximumInteger = V > getMaximumInteger < Vs
        ...> ? V :
        getMaximumInteger<Vs...>;

        template<size_t V>
        constexpr size_t getMaximumInteger<V> = V;

        template<typename ...Ts>
        struct VariantUtility;

        template<typename T, typename ...Ts>
        struct VariantUtility<T, Ts...> {
            static void
            doCopy(const ::std::type_info *VariantType, void *VariantDataSource, void *VariantDataDestination) {
                if (VariantType == &typeid(T)) new(VariantDataDestination) T(*((T *) VariantDataSource));
                else VariantUtility<Ts...>::doCopy(VariantType, VariantDataSource, VariantDataDestination);
            }

            static void deleteObject(const ::std::type_info *VariantType, void *VariantData) {
                if (VariantType == &typeid(T)) ((T *) VariantData)->~T();
                else VariantUtility<Ts...>::deleteObject(VariantType, VariantData);
            }

            static void
            doMove(const ::std::type_info *VariantType, void *VariantDataSource, void *VariantDataDestination) {
                if (VariantType == &typeid(T))
                    new(VariantDataDestination) T(Objects::doMove(*((T *) VariantDataSource)));
                else VariantUtility<Ts...>::doMove(VariantType, VariantDataSource, VariantDataDestination);
            }
        };

        template<typename ...T>
        class Tuple {};

        template<>
        struct VariantUtility<> {
            static void doCopy(const ::std::type_info *, void *, void *) {}

            static void deleteObject(const ::std::type_info *, void *) {}

            static void doMove(const ::std::type_info *, void *, void *) {}
        };

        template<typename T, typename ...Ts>
        constexpr bool isContainsType = true;

        template<typename T, typename T1, typename ...T2>
        constexpr bool isContainsType<T, T1, T2...> = Type::isSame<T, T1> || isContainsType<T, T2...>;

        template<typename T>
        constexpr bool isContainsType<T> = false;

        template<typename T, typename ...Ts>
        concept ContainsType = isContainsType<T, Ts...>;
    }

    template<typename ...Ts>
    class Variant final : public Object {
    private:
        alignas(getMaximumInteger<alignof(Ts)...>) uint8_t VariantData[getMaximumInteger<sizeof(Ts)...>]{};
        const ::std::type_info *VariantType = nullptr;
    public:
        doEnableCopyAssignConstruct(Variant)

        doEnableMoveAssignConstruct(Variant)

        constexpr Variant() noexcept = default;

        template<typename T>
        requires ContainsType<T, Ts...>
        Variant(const T &VariantSource) {
            MemoryAllocator<T>::doConstruct(VariantData, VariantSource);
            VariantType = &typeid(T);
        }

        template<typename T>
        requires ContainsType<T, Ts...>
        Variant(T &&VariantSource) {
            MemoryAllocator<T>::doConstruct(VariantData, Objects::doMove(VariantSource));
            VariantType = &typeid(T);
        }

        ~Variant() {
            VariantUtility<Ts...>::deleteObject(VariantType, VariantData);
        }

        void doAssign(const Variant<Ts...> &VariantSource) {
            if (Objects::getAddress(VariantSource) == this) return;
            VariantUtility<Ts...>::deleteObject(VariantType, VariantData);
            VariantUtility<Ts...>::doCopy(VariantSource.VariantType, VariantSource.VariantData, VariantData);
            VariantType = VariantSource.VariantType;
        }

        void doAssign(Variant<Ts...> &&VariantSource) {
            if (Objects::getAddress(VariantSource) == this) return;
            VariantUtility<Ts...>::deleteObject(VariantType, VariantData);
            VariantUtility<Ts...>::doMove(VariantSource.VariantType, VariantSource.VariantData, VariantData);
            VariantType = VariantSource.VariantType;
            VariantSource.VariantType = nullptr;
        }

        template<typename T>
        auto getValue() {
            if (&typeid(T) != VariantType) [[unlikely]]
                doThrowChecked(TypeException, u"Variant::getValue<T>() T"_S);
            return *((T *) VariantData);
        }

        template<typename T> requires ContainsType<T, Ts...>
        static Variant valueOf(T &&VariantSource) {
            return {Objects::doForward<T>(VariantSource)};
        }

        template<typename T, typename ...Tp> requires ContainsType<T, Ts...>
        static Variant valueOf(Tp&& ...VariantSource) {
            return {T(Objects::doForward<Tp>(VariantSource)...)};
        }
    };

    template<typename E>
    using ArrayQueue = ContainerQueue<E, ArrayList<E>>;

    template<typename E>
    using ArraySet = ContainerSet<E, ArrayList<E>>;

    template<typename E>
    using ArrayStack = ContainerStack<E, ArrayList<E>>;

    template<typename E>
    using DoubleLinkedQueue = ContainerQueue<E, DoubleLinkedList<E>>;

    template<typename E>
    using DoubleLinkedSet = ContainerSet<E, DoubleLinkedList<E>>;

    template<typename E>
    using DoubleLinkedStack = ContainerStack<E, DoubleLinkedList<E>>;

    template<typename E>
    using SingleLinkedQueue = ContainerQueue<E, SingleLinkedList<E>>;

    template<typename E>
    using SingleLinkedSet = ContainerSet<E, SingleLinkedList<E>>;

    template<typename E>
    using SingleLinkedStack = ContainerStack<E, SingleLinkedList<E>>;
}

#endif
