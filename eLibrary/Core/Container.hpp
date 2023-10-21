#pragma once

#ifndef eLibraryHeaderCoreContainer
#define eLibraryHeaderCoreContainer

#include <Core/Memory.hpp>
#include <iterator>
#include <typeinfo>

namespace eLibrary::Core {
    namespace {
        template<typename E>
        struct DoubleLinkedNode final {
            E NodeValue;
            DoubleLinkedNode *NodeNext = nullptr, *NodePrevious = nullptr;

            constexpr explicit DoubleLinkedNode(const E &NodeValueSource) : NodeValue(NodeValueSource) {}
        };

        template<typename E>
        struct SingleLinkedNode final {
            E NodeValue;
            SingleLinkedNode *NodeNext = nullptr;

            constexpr explicit SingleLinkedNode(const E &NodeValueSource) : NodeValue(NodeValueSource) {}
        };
    }

    class Any final : public Object {
    private:
        enum class AnyOperation {
            OperationCopy, OperationDestroy, OperationMove
        };

        void *AnyValue = nullptr;
        const ::std::type_info *AnyType = nullptr;
        void (*AnyFunction)(AnyOperation, Any*, void*) = nullptr;

        template<typename T>
        static void doExecute(AnyOperation ManagerOperation, Any *ManagerObject, void *ManagerSource) {
            switch (ManagerOperation) {
                case AnyOperation::OperationCopy:
                    ManagerObject->AnyValue = MemoryAllocator::newObject<T>(* (const T*) ManagerSource);
                    break;
                case AnyOperation::OperationDestroy:
                    MemoryAllocator::deleteObject((const T*) ManagerObject->AnyValue);
                    break;
                case AnyOperation::OperationMove:
                    ManagerObject->AnyValue = MemoryAllocator::newObject<T>(Objects::doMove(* (const T*) ManagerSource));
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

        template<typename T, typename ...Ts>
        Any(::std::in_place_t, Ts &&...ParameterList) {
            doAssign(T(Objects::doForward<Ts>(ParameterList)...));
        }

        ~Any() noexcept {
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
                AnySource.doReset();
            }
        }

        template<typename T>
        void doAssign(const T &AnySource) {
            doReset();
            AnyFunction = &doExecute<T>;
            AnyType = &typeid(T);
            AnyValue = MemoryAllocator::newObject<T>(AnySource);
        }

        template<typename T>
        void doAssign(T &&AnySource) {
            doReset();
            AnyFunction = &doExecute<T>;
            AnyType = &typeid(T);
            AnyValue = MemoryAllocator::newObject<T>(Objects::doMove(AnySource));
        }

        void doReset() noexcept {
            if (AnyFunction && AnyValue) AnyFunction(AnyOperation::OperationDestroy, this, nullptr);
            AnyFunction = nullptr;
            AnyType = nullptr;
            AnyValue = nullptr;
        }

        const char *getClassName() const noexcept override {
            return "Any";
        }

        template<typename T>
        auto getValue() const {
            if (&typeid(T) != AnyType) [[unlikely]]
                throw TypeException(String(u"Any::getValue<T>()"));
            if (!hasValue()) [[unlikely]]
                throw Exception(String(u"Any::getValue<T>() hasValue"));
            return *((T*) AnyValue);
        }

        auto getValueType() noexcept {
            return AnyType;
        }

        bool hasValue() const noexcept {
            return AnyValue;
        }
    };

    class Collections final : public Object {
    public:
        constexpr Collections() noexcept = delete;

        static void doCheckG(auto IndexSource, auto IndexStart) {
            if (IndexSource <= IndexStart) [[unlikely]]
                throw IndexException(String(u"Collections::doCheckG(auto, auto) IndexSource"));
        }

        static void doCheckGE(auto IndexSource, auto IndexStart) {
            if (IndexSource < IndexStart) [[unlikely]]
                throw IndexException(String(u"Collections::doCheckStartGE(auto, auto) IndexSource"));
        }

        static void doCheckL(auto IndexSource, auto IndexStop) {
            if (IndexSource >= IndexStop) [[unlikely]]
                throw IndexException(String(u"Collections::doCheckL(auto, auto) IndexSource"));
        }

        static void doCheckLE(auto IndexSource, auto IndexStop) {
            if (IndexSource > IndexStop) [[unlikely]]
                throw IndexException(String(u"Collections::doCheckLE(auto, auto) IndexSource"));
        }

        template<typename II, typename OI>
        static void doCopy(II IteratorInputStart, II IteratorInputStop, OI IteratorOutput) {
            for (;IteratorInputStart != IteratorInputStop;++IteratorInputStart, ++IteratorOutput)
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
        static void doCopyReverse(II IteratorInputStart, II IteratorInputStop, OI IteratorOutput) {
            while (IteratorInputStart != IteratorInputStop) {
                --IteratorInputStop;
                *IteratorOutput = *IteratorInputStop;
                ++IteratorOutput;
            }
        }

        template<typename II, typename T>
        static void doFill(II IteratorInputStart, II IteratorInputStop, const T &IteratorValue) {
            for (;IteratorInputStart != IteratorInputStop;++IteratorInputStart)
                *IteratorInputStart = IteratorValue;
        }

        template<typename II, typename OI>
        static void doMove(II IteratorInputStart, II IteratorInputStop, OI IteratorOutput) {
            for (;IteratorInputStart != IteratorInputStop;++IteratorInputStart, ++IteratorOutput)
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
        static void doMoveReverse(II IteratorInputStart, II IteratorInputStop, OI IteratorOutput) {
            for (;IteratorInputStart != IteratorInputStop;--IteratorInputStop, ++IteratorOutput)
                *IteratorOutput = Objects::doMove(*IteratorInputStop);
        }

        template<typename II, typename F>
        static void doTraverse(II IteratorInputStart, II IteratorInputStop, F IteratorFunction) {
            for (;IteratorInputStart != IteratorInputStop;++IteratorInputStart)
                IteratorFunction(*IteratorInputStart);
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

        constexpr ArrayIterator() noexcept : ElementCurrent(nullptr) {}

        constexpr ArrayIterator(E *ElementSource) noexcept : ElementCurrent(ElementSource) {}

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

        constexpr Array(E *ElementContainerSource, intmax_t ElementSizeSource) noexcept : ElementSize(ElementSizeSource), ElementContainer(ElementContainerSource) {}

        template<typename ...Es>
        void doInitialize(E ElementCurrent, Es ...ElementList) noexcept {
            ElementContainer[ElementSize++] = ElementCurrent;
            if constexpr (sizeof...(ElementList)) doInitialize(ElementList...);
        }

        template<typename>
        friend class ArrayList;
    public:
        doEnableCopyAssignConstruct(Array)
        doEnableMoveAssignConstruct(Array)

        constexpr Array() noexcept = default;

        template<typename II>
        Array(II ElementStart, II ElementStop) noexcept {
            Collections::doTraverse(ElementStart, ElementStop, [&](const E &ElementCurrent){
                addElement(ElementCurrent);
            });
        }

        Array(::std::initializer_list<E> ElementSource) noexcept : ElementSize(ElementSource.size()) {
            ElementContainer = MemoryAllocator::newArray<E>(ElementSize);
            Collections::doCopy(ElementSource.begin(), ElementSource.end(), ElementContainer);
        }

        template<typename ...Es>
        explicit Array(E ElementCurrent, Es ...ElementList) noexcept {
            ElementContainer = MemoryAllocator::newArray<E>(sizeof...(ElementList) + 1);
            doInitialize(ElementCurrent, ElementList...);
        }

        ~Array() noexcept {
            ElementSize = 0;
            MemoryAllocator::deleteArray(ElementContainer);
            ElementContainer = nullptr;
        }

        static Array doAllocate(intmax_t ElementSize) {
            return {MemoryAllocator::newArray<E>(ElementSize), ElementSize};
        }

        void doAssign(Array<E> &&ElementSource) noexcept {
            if (Objects::getAddress(ElementSource) == this) return;
            ElementContainer = ElementSource.ElementContainer;
            ElementSource.ElementContainer = nullptr;
            ElementSize = ElementSource.ElementSize;
        }

        void doAssign(const Array<E> &ElementSource) noexcept {
            if (Objects::getAddress(ElementSource) == this) return;
            MemoryAllocator::deleteArray(ElementContainer);
            ElementContainer = MemoryAllocator::newArray<E>(ElementSize = ElementSource.ElementSize);
            Collections::doCopy(ElementSource.ElementContainer, ElementSize, ElementContainer);
        }

        const char *getClassName() const noexcept override {
            return "Array";
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

        bool isEmpty() const noexcept {
            return !ElementSize;
        }

        void setElement(intmax_t ElementIndex, const E &ElementSource) {
            if (ElementIndex < 0) ElementIndex += ElementSize;
            Collections::doCheckGE(ElementIndex, 0);
            Collections::doCheckL(ElementIndex, ElementSize);
            ElementContainer[ElementIndex] = ElementSource;
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

        ArrayList(E *ElementSource, intmax_t ElementSourceSize) noexcept : ElementCapacity(1), ElementSize(ElementSourceSize) {
            while (ElementCapacity < ElementSourceSize) ElementCapacity <<= 1;
            ElementContainer = MemoryAllocator::newArray<E>(ElementCapacity);
            Collections::doCopy(ElementSource, ElementSource + ElementSourceSize, ElementContainer);
        }

        template<typename ...Es>
        void doInitialize(E ElementCurrent, Es ...ElementList) noexcept {
            ElementContainer[ElementSize++] = ElementCurrent;
            if constexpr (sizeof...(ElementList)) doInitialize(ElementList...);
        }

        template<typename>
        friend class ArraySet;
    public:
        doEnableCopyAssignConstruct(ArrayList)
        doEnableMoveAssignConstruct(ArrayList)

        constexpr ArrayList() noexcept = default;

        explicit ArrayList(const Array<E> &ElementSource) noexcept : ElementCapacity(1), ElementSize(ElementSource.getElementSize()) {
            while (ElementCapacity < ElementSize) ElementCapacity <<= 1;
            ElementContainer = MemoryAllocator::newArray<E>(ElementCapacity);
            Collections::doCopy(ElementSource.ElementContainer, ElementSize, ElementContainer);
        }

        template<typename ...Es>
        explicit ArrayList(Es ...ElementList) noexcept : ElementCapacity(1) {
            while ((uintmax_t) ElementCapacity < sizeof...(ElementList)) ElementCapacity <<= 1;
            ElementContainer = MemoryAllocator::newArray<E>(ElementCapacity);
            doInitialize(ElementList...);
        }

        ArrayList(::std::initializer_list<E> ElementList) noexcept : ElementCapacity(1), ElementSize((intmax_t) ElementList.size()) {
            while (ElementCapacity < ElementSize)
                ElementCapacity <<= 1;
            ElementContainer = MemoryAllocator::newArray<E>(ElementCapacity);
            Collections::doCopy(ElementList.begin(), ElementList.end(), ElementContainer);
        }

        template<typename II>
        ArrayList(II ElementStart, II ElementStop) noexcept {
            Collections::doTraverse(ElementStart, ElementStop, [&](const E &ElementCurrent){
                addElement(ElementCurrent);
            });
        }

        ~ArrayList() noexcept {
            doClear();
        }

        void addElement(const E &ElementSource) noexcept {
            if (!ElementCapacity) ElementContainer = MemoryAllocator::newArray<E>(ElementCapacity = 1);
            if (ElementSize == ElementCapacity) {
                E *ElementBuffer = MemoryAllocator::newArray<E>(ElementSize);
                Collections::doMove(ElementContainer, ElementSize, ElementBuffer);
                MemoryAllocator::deleteArray(ElementContainer);
                ElementContainer = MemoryAllocator::newArray<E>(ElementCapacity <<= 1);
                Collections::doMove(ElementBuffer, ElementSize, ElementContainer);
                MemoryAllocator::deleteArray(ElementBuffer);
            }
            ElementContainer[ElementSize++] = ElementSource;
        }

        void addElement(intmax_t ElementIndex, const E &ElementSource) {
            if (ElementIndex < 0) ElementIndex += ElementSize + 1;
            Collections::doCheckGE(ElementIndex, 0);
            Collections::doCheckLE(ElementIndex, ElementSize);
            if (!ElementCapacity) ElementContainer = MemoryAllocator::newArray<E>(ElementCapacity = 1);
            if (ElementSize == ElementCapacity) {
                E *ElementBuffer = MemoryAllocator::newArray<E>(ElementSize);
                Collections::doMove(ElementContainer, ElementContainer + ElementSize, ElementBuffer);
                MemoryAllocator::deleteArray(ElementContainer);
                ElementContainer = MemoryAllocator::newArray<E>(ElementCapacity <<= 1);
                Collections::doMove(ElementBuffer, ElementBuffer + ElementSize, ElementContainer);
                MemoryAllocator::deleteArray(ElementBuffer);
            }
            Collections::doMoveBackward(ElementContainer + ElementIndex, ElementContainer + ElementSize, ElementContainer + ElementSize + 1);
            ElementContainer[ElementIndex] = ElementSource;
            ++ElementSize;
        }

        void doAssign(ArrayList<E> &&ElementSource) noexcept {
            if (Objects::getAddress(ElementSource) == this) return;
            MemoryAllocator::deleteArray(ElementContainer);
            ElementCapacity = ElementSource.ElementCapacity;
            ElementContainer = ElementSource.ElementContainer;
            ElementSize = ElementSource.ElementSize;
            ElementSource.ElementCapacity = 0;
            ElementSource.ElementContainer = nullptr;
            ElementSource.ElementSize = 0;
        }

        void doAssign(const ArrayList<E> &ElementSource) noexcept {
            if (Objects::getAddress(ElementSource) == this) return;
            MemoryAllocator::deleteArray(ElementContainer);
            if (ElementCapacity) ElementContainer = MemoryAllocator::newArray<E>(ElementCapacity = ElementSource.ElementCapacity);
            Collections::doCopy(ElementSource.ElementContainer, (ElementSize = ElementSource.ElementSize), ElementContainer);
        }

        void doClear() noexcept {
            ElementCapacity = 0;
            ElementSize = 0;
            MemoryAllocator::deleteArray(ElementContainer);
            ElementContainer = nullptr;
        }

        ArrayList<E> doConcat(const ArrayList<E> &ElementSource) const noexcept {
            E *ElementBuffer = MemoryAllocator::newArray<E>(ElementSize + ElementSource.ElementSize);
            Collections::doCopy(ElementContainer, ElementSize, ElementBuffer);
            Collections::doCopy(ElementSource.ElementContainer, ElementSource.ElementSize, ElementBuffer + ElementSize);
            return {ElementBuffer, ElementSize + ElementSource.ElementSize};
        }

        void doReverse() noexcept {
            E *ElementBuffer = MemoryAllocator::newArray<E>(ElementSize);
            Collections::doMove(ElementContainer, ElementSize, ElementBuffer);
            Collections::doMoveReverse(ElementBuffer, ElementBuffer + ElementSize, ElementContainer);
            MemoryAllocator::deleteArray(ElementBuffer);
        }

        const char *getClassName() const noexcept override {
            return "ArrayList";
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
            Collections::doMove(ElementContainer + ElementIndex + 1, ElementContainer + ElementSize, ElementContainer + ElementIndex);
            if (ElementCapacity == 1) doClear();
            else if (--ElementSize <= ElementCapacity >> 1) {
                E *ElementBuffer = MemoryAllocator::newArray<E>(ElementSize);
                Collections::doMove(ElementContainer, ElementSize, ElementBuffer);
                MemoryAllocator::deleteArray(ElementContainer);
                ElementContainer = MemoryAllocator::newArray<E>(ElementCapacity >>= 1);
                Collections::doMove(ElementBuffer, ElementSize, ElementContainer);
                MemoryAllocator::deleteArray(ElementBuffer);
            }
        }

        void setElement(intmax_t ElementIndex, const E &ElementSource) {
            if (ElementIndex < 0) ElementIndex += ElementSize;
            Collections::doCheckGE(ElementIndex, 0);
            Collections::doCheckL(ElementIndex, ElementSize);
            ElementContainer[ElementIndex] = ElementSource;
        }

        String toString() const noexcept override {
            StringStream CharacterStream;
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
     * Support for continuous storage of unique objects
     */
    template<typename E>
    class ArraySet final : protected ArrayList<E> {
    public:
        doEnableCopyAssignConstruct(ArraySet)
        doEnableMoveAssignConstruct(ArraySet)

        constexpr ArraySet() noexcept = default;

        explicit ArraySet(const ArrayList<E> &ElementSource) noexcept {
            for (intmax_t ElementIndex = 0;ElementIndex < ElementSource.getElementSize();++ElementIndex)
                addElement(ElementSource.ElementContainer[ElementIndex]);
        }

        ~ArraySet() noexcept {
            doClear();
        }

        using ArrayList<E>::addElement;

        void doAssign(ArraySet &&SetSource) noexcept {
            if (Objects::getAddress(SetSource) == this) return;
            MemoryAllocator::deleteArray(this->ElementContainer);
            this->ElementCapacity = SetSource.ElementCapacity;
            this->ElementContainer = SetSource.ElementContainer;
            this->ElementSize = SetSource.ElementSize;
            SetSource.ElementCapacity = SetSource.ElementSize = 0;
            SetSource.ElementContainer = nullptr;
        }

        void doAssign(const ArraySet &SetSource) noexcept {
            if (Objects::getAddress(SetSource) == this) return;
            MemoryAllocator::deleteArray(this->ElementContainer);
            this->ElementCapacity = SetSource.ElementCapacity;
            if (this->ElementCapacity) this->ElementContainer = MemoryAllocator::newArray<E>(this->ElementCapacity);
            Collections::doCopy(SetSource.ElementContainer, SetSource.ElementContainer + (this->ElementSize = SetSource.ElementSize), this->ElementContainer);
        }

        using ArrayList<E>::doClear;

        ArraySet doDifference(const ArraySet &SetSource) const noexcept {
            ArraySet SetResult;
            for (uintmax_t ElementIndex = 0; ElementIndex < this->ElementSize; ++ElementIndex)
                if (!SetSource.isContains(this->ElementContainer[ElementIndex])) SetResult.addElement(this->ElementContainer[ElementIndex]);
            return SetResult;
        }

        ArraySet doIntersection(const ArraySet &SetSource) const noexcept {
            ArraySet SetResult;
            for (uintmax_t ElementIndex = 0; ElementIndex < this->ElementSize; ++ElementIndex)
                if (SetSource.isContains(this->ElementContainer[ElementIndex])) SetResult.addElement(this->ElementContainer[ElementIndex]);
            return SetResult;
        }

        ArraySet doUnion(const ArraySet &SetSource) const noexcept {
            ArraySet SetResult(*this);
            for (uintmax_t ElementIndex = 0; ElementIndex < SetSource.ElementSize; ++ElementIndex)
                SetResult.addElement(SetSource.ElementContainer[ElementIndex]);
            return SetResult;
        }

        const char *getClassName() const noexcept override {
            return "ArraySet";
        }

        using ArrayList<E>::getElementSize;

        using ArrayList<E>::isContains;

        using ArrayList<E>::isEmpty;

        using ArrayList<E>::removeElement;

        ArrayList<E> toArrayList() const noexcept {
            ArrayList<E> ListResult;
            for (uintmax_t ElementIndex = 0; ElementIndex < this->ElementSize;++ElementIndex)
                ListResult.addElement(this->ElementContainer[ElementIndex]);
            return ListResult;
        }

        String toString() const noexcept override {
            StringStream CharacterStream;
            CharacterStream.addCharacter(u'{');
            for (uintmax_t ElementIndex = 0; ElementIndex + 1 < this->ElementSize; ++ElementIndex) {
                CharacterStream.addString(String::valueOf(this->ElementContainer[ElementIndex]));
                CharacterStream.addCharacter(u',');
            }
            if (!isEmpty()) CharacterStream.addString(String::valueOf(this->ElementContainer[this->ElementSize - 1]));
            CharacterStream.addCharacter(u'}');
            return CharacterStream.toString();
        }

        using ArrayList<E>::begin;

        using ArrayList<E>::end;
    };

    template<typename E, typename C>
    class ContainerQueue final : public Object {
    private:
        C ElementContainer;
    public:
        doEnableCopyAssignConstruct(ContainerQueue)
        doEnableMoveAssignConstruct(ContainerQueue)

        constexpr ContainerQueue() noexcept = default;

        ~ContainerQueue() noexcept {
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

        void doClear() noexcept {
            ElementContainer.doClear();
        }

        const E &getBack() const {
            return ElementContainer.getElement(-1);
        }

        E &getBack() {
            return ElementContainer.getElement(-1);
        }

        const char *getClassName() const noexcept override {
            return "ContainerQueue";
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

        ~ContainerStack() noexcept {
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

        void doClear() noexcept {
            ElementContainer.doClear();
        }

        const char *getClassName() const noexcept override {
            return "ContainerStack";
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

        constexpr DoubleLinkedIterator() noexcept : NodeCurrent(nullptr) {}

        constexpr DoubleLinkedIterator(LinkedNode *NodeSource) noexcept : NodeCurrent(NodeSource) {}

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

        template<typename>
        friend class DoubleLinkedSet;

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

        DoubleLinkedList(::std::initializer_list<E> ElementList) noexcept : NodeSize(ElementList.size()) {
            for (const E &ElementCurrent : ElementList) addElement(ElementCurrent);
        }

        template<typename II>
        DoubleLinkedList(II ElementStart, II ElementStop) noexcept {
            Collections::doTraverse(ElementStart, ElementStop, [&](const E &ElementCurrent){
                addElement(ElementCurrent);
            });
        }

        ~DoubleLinkedList() noexcept {
            doClear();
        }

        void addElement(const E &ElementSource) noexcept {
            auto *NodeCurrent = MemoryAllocator::newObject<LinkedNode>(ElementSource);
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
            auto *NodeTarget = MemoryAllocator::newObject<LinkedNode>(ElementSource);
            if (ElementIndex == 0) {
                NodeTarget->NodeNext = NodeHead;
                NodeHead->NodePrevious = NodeTarget;
                NodeHead = NodeTarget;
                ++NodeSize;
                return;
            }
            LinkedNode *NodeCurrent = NodeHead;
            while (--ElementIndex) NodeCurrent = NodeCurrent->NodeNext;
            NodeTarget->NodePrevious = NodeCurrent;
            NodeTarget->NodeNext = NodeCurrent->NodeNext;
            NodeCurrent->NodeNext->NodePrevious = NodeTarget;
            NodeCurrent->NodeNext = NodeTarget;
            ++NodeSize;
        }

        void doAssign(const DoubleLinkedList<E> &ElementSource) noexcept {
            if (Objects::getAddress(ElementSource) == this) return;
            if (!isEmpty()) doClear();
            NodeSize = ElementSource.NodeSize;
            LinkedNode *NodeCurrent = ElementSource.NodeHead;
            while (NodeCurrent) {
                addElement(NodeCurrent->NodeValue);
                NodeCurrent = NodeCurrent->NodeNext;
            }
        }

        void doAssign(DoubleLinkedList<E> &&ElementSource) noexcept {
            if (Objects::getAddress(ElementSource) == this) return;
            if (!isEmpty()) doClear();
            NodeHead = ElementSource.NodeHead;
            NodeSize = ElementSource.NodeSize;
            NodeTail = ElementSource.NodeTail;
            ElementSource.NodeHead = ElementSource.NodeTail = nullptr;
            ElementSource.NodeSize = 0;
        }

        void doClear() noexcept {
            NodeSize = 0;
            NodeTail = nullptr;
            LinkedNode *NodeCurrent = NodeHead, *NodePrevious;
            NodeHead = nullptr;
            while (NodeCurrent) {
                NodePrevious = NodeCurrent;
                NodeCurrent = NodeCurrent->NodeNext;
                MemoryAllocator::deleteObject(NodePrevious);
            }
        }

        void doReverse() noexcept {
            if (NodeHead == NodeTail) return;
            if (NodeHead->NodeNext == NodeTail) {
                Objects::doSwap(NodeHead, NodeTail);
                return;
            }
            LinkedNode *NodeCurrent = NodeHead->NodeNext, *NodeNext, *NodeTemporary = MemoryAllocator::newObject<LinkedNode>();
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

        const char *getClassName() const noexcept override {
            return "DoubleLinkedList";
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
            LinkedNode *NodeCurrent = NodeHead;
            while (NodeCurrent && Objects::doCompare(NodeCurrent->NodeValue, ElementSource)) NodeCurrent = NodeCurrent->NodeNext, ++NodeIndex;
            if (!NodeCurrent) return -1;
            return NodeIndex;
        }

        bool isContains(const E &ElementSource) const noexcept {
            LinkedNode *NodeCurrent = NodeHead;
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
                MemoryAllocator::deleteObject(NodeHead);
                NodeHead = NodeTail = nullptr;
            } else if (ElementIndex == 0) {
                NodeHead = NodeHead->NodeNext;
                MemoryAllocator::deleteObject(NodeHead->NodePrevious);
                NodeHead->NodePrevious = nullptr;
            } else if (ElementIndex == NodeSize - 1) {
                NodeTail = NodeTail->NodePrevious;
                MemoryAllocator::deleteObject(NodeTail->NodeNext);
                NodeTail->NodeNext = nullptr;
            } else if (ElementIndex < (NodeSize >> 1)) {
                NodeCurrent = NodeHead;
                while (--ElementIndex) NodeCurrent = NodeCurrent->NodeNext;
                NodeCurrent->NodeNext = NodeCurrent->NodeNext->NodeNext;
                MemoryAllocator::deleteObject(NodeCurrent->NodeNext->NodePrevious);
                NodeCurrent->NodeNext->NodePrevious = NodeCurrent;
            } else {
                NodeCurrent = NodeTail;
                ElementIndex = NodeSize - ElementIndex - 1;
                while (--ElementIndex) NodeCurrent = NodeCurrent->NodePrevious;
                NodeCurrent->NodePrevious = NodeCurrent->NodePrevious->NodePrevious;
                MemoryAllocator::deleteObject(NodeCurrent->NodePrevious->NodeNext);
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
            StringStream CharacterStream;
            CharacterStream.addCharacter(u'[');
            if (NodeHead) {
                LinkedNode *NodeCurrent = NodeHead;
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

    /**
     * Support for discrete storage of unique objects
     */
    template<typename E>
    class DoubleLinkedSet final : public Object {
    private:
        using LinkedNode = DoubleLinkedNode<E>;
        LinkedNode *NodeHead = nullptr, *NodeTail = nullptr;
        intmax_t NodeSize = 0;
    public:
        doEnableCopyAssignConstruct(DoubleLinkedSet)
        doEnableMoveAssignConstruct(DoubleLinkedSet)

        constexpr DoubleLinkedSet() noexcept = default;

        explicit DoubleLinkedSet(const DoubleLinkedList<E> &ElementSource) noexcept {
            auto *NodeCurrent = ElementSource.NodeHead;
            while (NodeCurrent) {
                addElement(NodeCurrent->NodeValue);
                NodeCurrent = NodeCurrent->NodeNext;
            }
        }

        ~DoubleLinkedSet() noexcept {
            doClear();
        }

        void addElement(const E &ElementSource) noexcept {
            if (isContains(ElementSource)) return;
            auto *NodeCurrent = MemoryAllocator::newObject<LinkedNode>(ElementSource);
            if (!NodeHead) NodeHead = NodeTail = NodeCurrent;
            else {
                NodeTail->NodeNext = NodeCurrent;
                NodeCurrent->NodePrevious = NodeTail;
                NodeTail = NodeCurrent;
            }
            ++NodeSize;
        }

        void doAssign(const DoubleLinkedSet &ElementSource) noexcept {
            if (Objects::getAddress(ElementSource) == this) return;
            if (!isEmpty()) doClear();
            NodeSize = ElementSource.NodeSize;
            LinkedNode *NodeCurrent = ElementSource.NodeHead;
            while (NodeCurrent) {
                addElement(NodeCurrent->NodeValue);
                NodeCurrent = NodeCurrent->NodeNext;
            }
        }

        void doAssign(DoubleLinkedSet &&ElementSource) noexcept {
            if (Objects::getAddress(ElementSource) == this) return;
            if (!isEmpty()) doClear();
            NodeHead = ElementSource.NodeHead;
            NodeSize = ElementSource.NodeSize;
            NodeTail = ElementSource.NodeTail;
            ElementSource.NodeHead = ElementSource.NodeTail = nullptr;
            ElementSource.NodeSize = 0;
        }

        void doClear() noexcept {
            NodeSize = 0;
            NodeTail = nullptr;
            LinkedNode *NodeCurrent = NodeHead, *NodePrevious;
            NodeHead = nullptr;
            while (NodeCurrent) {
                NodePrevious = NodeCurrent;
                NodeCurrent = NodeCurrent->NodeNext;
                MemoryAllocator::deleteObject(NodePrevious);
            }
        }

        DoubleLinkedSet doDifference(const DoubleLinkedSet &SetSource) const noexcept {
            DoubleLinkedSet SetResult;
            LinkedNode *NodeCurrent = NodeHead;
            while (NodeCurrent) {
                if (!SetSource.isContains(NodeCurrent->NodeValue))
                    SetResult.addElement(NodeCurrent->NodeValue);
                NodeCurrent = NodeCurrent->NodeNext;
            }
            return SetResult;
        }

        DoubleLinkedSet doIntersection(const DoubleLinkedSet &SetSource) const noexcept {
            DoubleLinkedSet SetResult;
            LinkedNode *NodeCurrent = NodeHead;
            while (NodeCurrent) {
                if (SetSource.isContains(NodeCurrent->NodeValue))
                    SetResult.addElement(NodeCurrent->NodeValue);
                NodeCurrent = NodeCurrent->NodeNext;
            }
            return SetResult;
        }

        DoubleLinkedSet doUnion(const DoubleLinkedSet &SetSource) const noexcept {
            DoubleLinkedSet SetResult(*this);
            LinkedNode *NodeCurrent = SetSource.NodeHead;
            while (NodeCurrent) {
                SetResult.addElement(NodeCurrent->NodeValue);
                NodeCurrent = NodeCurrent->NodeNext;
            }
            return SetResult;
        }

        const char *getClassName() const noexcept override {
            return "DoubleLinkedSet";
        }

        intmax_t getElementSize() const noexcept {
            return NodeSize;
        }

        bool isContains(const E &ElementSource) const noexcept {
            LinkedNode *NodeCurrent = NodeHead;
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
            intmax_t ElementIndex = 0;
            LinkedNode *NodeCurrent = NodeHead;
            while (NodeCurrent && Objects::doCompare(NodeCurrent->NodeValue, ElementSource)) NodeCurrent = NodeCurrent->NodeNext, ++ElementIndex;
            Collections::doCheckL(ElementIndex, NodeSize);
            if (ElementIndex == 0) {
                NodeHead = NodeHead->NodeNext;
                if (NodeHead->NodePrevious) MemoryAllocator::deleteObject(NodeHead->NodePrevious);
                NodeHead->NodePrevious = nullptr;
            } else if (ElementIndex == NodeSize - 1) {
                NodeTail = NodeTail->NodePrevious;
                if (NodeTail->NodeNext) MemoryAllocator::deleteObject(NodeTail->NodeNext);
                NodeTail->NodeNext = nullptr;
            } else if (ElementIndex < (NodeSize >> 1)) {
                NodeCurrent = NodeHead;
                while (--ElementIndex) NodeCurrent = NodeCurrent->NodeNext;
                NodeCurrent->NodeNext = NodeCurrent->NodeNext->NodeNext;
                MemoryAllocator::deleteObject(NodeCurrent->NodeNext->NodePrevious);
                NodeCurrent->NodeNext->NodePrevious = NodeCurrent;
            } else {
                NodeCurrent = NodeTail;
                ElementIndex = NodeSize - ElementIndex - 1;
                while (--ElementIndex) NodeCurrent = NodeCurrent->NodePrevious;
                NodeCurrent->NodePrevious = NodeCurrent->NodePrevious->NodePrevious;
                MemoryAllocator::deleteObject(NodeCurrent->NodePrevious->NodeNext);
                NodeCurrent->NodePrevious->NodeNext = NodeCurrent;
            }
            --NodeSize;
        }

        DoubleLinkedList<E> toDoubleLinkedList() const noexcept {
            DoubleLinkedList<E> ListResult;
            LinkedNode *NodeCurrent = NodeHead;
            while (NodeCurrent) {
                ListResult.addElement(NodeCurrent->NodeValue);
                NodeCurrent = NodeCurrent->NodeNext;
            }
            return ListResult;
        }

        String toString() const noexcept override {
            StringStream CharacterStream;
            CharacterStream.addCharacter(u'{');
            if (NodeHead) {
                LinkedNode *NodeCurrent = NodeHead;
                while (NodeCurrent->NodeNext) {
                    CharacterStream.addString(String::valueOf(NodeCurrent->NodeValue));
                    CharacterStream.addCharacter(u',');
                    NodeCurrent = NodeCurrent->NodeNext;
                }
                CharacterStream.addString(String::valueOf(NodeCurrent->NodeValue));
            }
            CharacterStream.addCharacter(u'}');
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
            virtual ~FunctionDescriptorBase() noexcept = default;

            virtual Tr doCall(Ts...) = 0;

            virtual FunctionDescriptorBase *doClone() const = 0;
        } *DescriptorHandle = nullptr;

        template<typename F>
        struct FunctionDescriptor : public FunctionDescriptorBase {
        private:
            F DescriptorHandle;
        public:
            constexpr FunctionDescriptor(F DescriptorHandleSource) noexcept : DescriptorHandle(DescriptorHandleSource) {}

            Tr doCall(Ts ...ParameterList) override {
                return DescriptorHandle(Objects::doForward<Ts>(ParameterList)...);
            }

            FunctionDescriptorBase *doClone() const override {
                return MemoryAllocator::newObject<FunctionDescriptor<F>>(DescriptorHandle);
            }
        };
    public:
        doEnableCopyAssignConstruct(Function)
        doEnableMoveAssignConstruct(Function)

        template<typename F> requires std::is_invocable_r_v<Tr, F, Ts...>
        Function(F FunctionObject) noexcept : DescriptorHandle(MemoryAllocator::newObject<FunctionDescriptor<F>>(FunctionObject)) {}

        ~Function() noexcept {
            if (DescriptorHandle) {
                MemoryAllocator::deleteObject(DescriptorHandle);
                DescriptorHandle = nullptr;
            }
        }

        void doAssign(const Function<Tr(Ts...)> &FunctionSource) {
            if (Objects::getAddress(FunctionSource) == this) return;
            MemoryAllocator::deleteObject(DescriptorHandle);
            if (FunctionSource.DescriptorHandle) DescriptorHandle = FunctionSource.DescriptorHandle->doClone();
            else DescriptorHandle = nullptr;
        }

        void doAssign(Function<Tr(Ts...)> &&FunctionSource) noexcept {
            if (Objects::getAddress(FunctionSource) == this) return;
            DescriptorHandle = FunctionSource.DescriptorHandle;
            FunctionSource.DescriptorHandle = nullptr;
        }

        Tr doCall(Ts ...ParameterList) {
            if (!DescriptorHandle) [[unlikely]]
                throw Exception(String(u"Function<Tr(Ts...)>::doCall(Ts...) DescriptorHandle"));
            return DescriptorHandle->doCall(Objects::doForward<Ts>(ParameterList)...);
        }

        const char *getClassName() const noexcept override {
            return "Function";
        }

        Tr operator()(Ts &&...FunctionParameter) {
            doCall(Objects::doForward<Ts>(FunctionParameter)...);
        }
    };

    class Functions final : public Object {
    public:
        constexpr Functions() noexcept = delete;

        template<typename F, typename ...Ts>
        static auto doBind(F &&FunctionSource, Ts &&...FunctionParameter) {
            return std::bind(FunctionSource, Objects::doForward<Ts>(FunctionParameter)...);
        }
    };

    class IteratorAdapter {};

    template<typename T>
    class Optional final : public Object {
    private:
        alignas(alignof(T)) uint8_t OptionalData[sizeof(T)]{};
        bool OptionalValue = false;

        template<typename ...Ts>
        void doCreate(Ts &&...ParameterList) {
            new (OptionalData) T(Objects::doForward<Ts>(ParameterList)...);
            OptionalValue = true;
        }
    public:
        doEnableCopyAssignConstruct(Optional)
        doEnableCopyAssignParameterConstruct(Optional, T)
        doEnableMoveAssignConstruct(Optional)
        doEnableMoveAssignParameterConstruct(Optional, T)

        constexpr Optional() noexcept = default;

        template<typename ...Ts>
        Optional(::std::in_place_t, Ts &&...ParameterList) {
            doAssign(T(Objects::doForward<Ts>(ParameterList)...));
        }

        ~Optional() noexcept {
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
            if (OptionalSource.hasValue() && Objects::getAddress(OptionalSource) != this) doCreate(OptionalSource.getValue());
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
                ((T*) OptionalData)->~T();
            }
        }

        const char *getClassName() const noexcept override {
            return "Optional";
        }

        T getValue() const {
            if (!hasValue()) [[unlikely]]
                throw Exception(String(u"Optional<T>::getValue() hasValue"));
            return *((T*) OptionalData);
        }

        bool hasValue() const noexcept {
            return OptionalValue;
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

            ~RedBlackNode() noexcept {
                NodeParent = nullptr;
                if (NodeChildLeft) {
                    MemoryAllocator::deleteObject(NodeChildLeft);
                    NodeChildLeft = nullptr;
                }
                if (NodeChildRight) {
                    MemoryAllocator::deleteObject(NodeChildRight);
                    NodeChildRight = nullptr;
                }
            }
        } *NodeRoot = nullptr;

        template<typename OperationType>
        static void doOrderCore(RedBlackNode *NodeCurrent, OperationType Operation) {
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
            if (NodeRelation == 0) return NodeCurrent;
            else if (NodeRelation < 0) return doSearchCore(NodeCurrent->NodeChildRight, NodeKey);
            else return doSearchCore(NodeCurrent->NodeChildLeft, NodeKey);
        }

        static uintmax_t getHeightCore(RedBlackNode *NodeCurrent) noexcept {
            if (NodeCurrent->NodeChildLeft && NodeCurrent->NodeChildRight)
                return Objects::getMaximum(getHeightCore(NodeCurrent->NodeChildLeft), getHeightCore(NodeCurrent->NodeChildRight)) + 1;
            if (NodeCurrent->NodeChildLeft) return getHeightCore(NodeCurrent->NodeChildLeft) + 1;
            if (NodeCurrent->NodeChildRight) return getHeightCore(NodeCurrent->NodeChildRight) + 1;
            return 1;
        }

        static uintmax_t getSizeCore(RedBlackNode *NodeCurrent) noexcept {
            if (!NodeCurrent) return 0;
            return getSizeCore(NodeCurrent->NodeChildLeft) + getSizeCore(NodeCurrent->NodeChildRight) + 1;
        }

        template<typename, typename>
        friend class TreeMap;

        template<typename>
        friend class TreeSet;
    public:
        doEnableCopyAssignConstruct(RedBlackTree)

        constexpr RedBlackTree() noexcept = default;

        ~RedBlackTree() noexcept {
            if (NodeRoot) {
                MemoryAllocator::deleteObject(NodeRoot);
                NodeRoot = nullptr;
            }
        }

        void doAssign(const RedBlackTree<K, V> &TreeSource) noexcept {
            if (Objects::getAddress(TreeSource) == this) return;
            MemoryAllocator::deleteObject(NodeRoot);
            TreeSource.doOrderCore(TreeSource.NodeRoot, [&](const K &NodeKeySource, const V &NodeValueSource){
                doInsert(NodeKeySource, NodeValueSource);
            });
        }

        void doInsert(const K &NodeKey, const V &NodeValue) noexcept {
            auto *NodeTarget = MemoryAllocator::newObject<RedBlackNode>(NodeKey, NodeValue);
            if (!NodeRoot) {
                NodeTarget->NodeColor = NodeColorEnumeration::ColorBlack;
                NodeRoot = NodeTarget;
                return;
            }
            auto *NodeParent = NodeRoot;
            while (NodeParent) {
                auto NodeRelation = Objects::doCompare(NodeTarget->NodeKey, NodeParent->NodeKey);
                if (NodeRelation == 0) return;
                else if (NodeRelation > 0) {
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

        template<typename OperationType>
        void doOrder(const OperationType &Operation) {
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
                    if ((!NodeUncle->NodeChildLeft || NodeUncle->NodeChildLeft->NodeColor == NodeColorEnumeration::ColorBlack) &&
                        (!NodeUncle->NodeChildRight || NodeUncle->NodeChildRight->NodeColor == NodeColorEnumeration::ColorBlack)) {
                        NodeUncle->NodeColor = NodeColorEnumeration::ColorRed;
                        NodeChild = NodeParent;
                        NodeParent = NodeChild->NodeParent;
                    } else {
                        if (!NodeUncle->NodeChildRight || NodeUncle->NodeChildRight->NodeColor == NodeColorEnumeration::ColorBlack) {
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
                    if ((!NodeUncle->NodeChildLeft || NodeUncle->NodeChildLeft->NodeColor == NodeColorEnumeration::ColorBlack) &&
                        (!NodeUncle->NodeChildRight || NodeUncle->NodeChildRight->NodeColor == NodeColorEnumeration::ColorBlack)) {
                        NodeUncle->NodeColor = NodeColorEnumeration::ColorRed;
                        NodeChild = NodeParent;
                        NodeParent = NodeChild->NodeParent;
                    } else {
                        if (!NodeUncle->NodeChildLeft || NodeUncle->NodeChildLeft->NodeColor == NodeColorEnumeration::ColorBlack) {
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

        const char *getClassName() const noexcept override {
            return "RedBlackTree";
        }

        uintmax_t getHeight() const noexcept {
            return NodeRoot ? getHeightCore(NodeRoot) : 0;
        }

        uintmax_t getSize() const noexcept {
            return NodeRoot ? getSizeCore(NodeRoot) : 0;
        }
    };

    template<typename E>
    class SingleLinkedIterator final {
    private:
        using LinkedNode = SingleLinkedNode<E>;
        LinkedNode *NodeCurrent;
    public:
        using difference_type = ptrdiff_t;
        using iterator_category = ::std::forward_iterator_tag;
        using value_type = E;

        constexpr SingleLinkedIterator() noexcept : NodeCurrent(nullptr) {}

        constexpr SingleLinkedIterator(SingleLinkedNode<E> *NodeSource) noexcept : NodeCurrent(NodeSource) {}

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

        template<typename>
        friend class SingleLinkedSet;

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

        SingleLinkedList(::std::initializer_list<E> ElementList) noexcept : NodeSize(ElementList.size()) {
            for (const E &ElementCurrent : ElementList) addElement(ElementCurrent);
        }

        template<typename II>
        SingleLinkedList(II ElementStart, II ElementStop) noexcept {
            Collections::doTraverse(ElementStart, ElementStop, [&](const E &ElementCurrent){
                addElement(ElementCurrent);
            });
        }

        ~SingleLinkedList() noexcept {
            doClear();
        }

        void addElement(const E &ElementSource) noexcept {
            auto *NodeCurrent = MemoryAllocator::newObject<LinkedNode>(ElementSource);
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
            auto *NodeTarget = MemoryAllocator::newObject<LinkedNode>(ElementSource);
            if (ElementIndex == 0) {
                NodeTarget->NodeNext = NodeHead;
                NodeHead = NodeTarget;
                ++NodeSize;
                return;
            }
            LinkedNode *NodeCurrent = NodeHead;
            while (--ElementIndex) NodeCurrent = NodeCurrent->NodeNext;
            NodeTarget->NodeNext = NodeCurrent->NodeNext;
            NodeCurrent->NodeNext = NodeTarget;
            ++NodeSize;
        }

        void doAssign(const SingleLinkedList<E> &ElementSource) noexcept {
            if (Objects::getAddress(ElementSource) == this) return;
            if (!isEmpty()) doClear();
            NodeSize = ElementSource.NodeSize;
            LinkedNode *NodeCurrent = ElementSource.NodeHead;
            while (NodeCurrent) {
                addElement(NodeCurrent->NodeValue);
                NodeCurrent = NodeCurrent->NodeNext;
            }
        }

        void doAssign(SingleLinkedList<E> &&ElementSource) noexcept {
            if (Objects::getAddress(ElementSource) == this) return;
            if (!isEmpty()) doClear();
            NodeHead = ElementSource.NodeHead;
            NodeSize = ElementSource.NodeSize;
            NodeTail = ElementSource.NodeTail;
            ElementSource.NodeHead = ElementSource.NodeTail = nullptr;
            ElementSource.NodeSize = 0;
        }

        void doClear() noexcept {
            NodeSize = 0;
            NodeTail = nullptr;
            LinkedNode *NodeCurrent = NodeHead, *NodePrevious;
            NodeHead = nullptr;
            while (NodeCurrent) {
                NodePrevious = NodeCurrent;
                NodeCurrent = NodeCurrent->NodeNext;
                MemoryAllocator::deleteObject(NodePrevious);
            }
        }

        const char *getClassName() const noexcept override {
            return "SingleLinkedList";
        }

        const E &getElement(intmax_t ElementIndex) const {
            if (ElementIndex < 0) ElementIndex += NodeSize;
            Collections::doCheckGE(ElementIndex, 0);
            Collections::doCheckL(ElementIndex, NodeSize);
            if (ElementIndex == NodeSize - 1) return NodeTail->NodeValue;
            LinkedNode *NodeCurrent = NodeHead;
            while (ElementIndex--) NodeCurrent = NodeCurrent->NodeNext;
            return NodeCurrent->NodeValue;
        }

        E &getElement(intmax_t ElementIndex) {
            if (ElementIndex < 0) ElementIndex += NodeSize;
            Collections::doCheckGE(ElementIndex, 0);
            Collections::doCheckL(ElementIndex, NodeSize);
            if (ElementIndex == NodeSize - 1) return NodeTail->NodeValue;
            LinkedNode *NodeCurrent = NodeHead;
            while (ElementIndex--) NodeCurrent = NodeCurrent->NodeNext;
            return NodeCurrent->NodeValue;
        }

        intmax_t getElementSize() const noexcept {
            return NodeSize;
        }

        intmax_t indexOf(const E &ElementSource) const noexcept {
            intmax_t NodeIndex = 0;
            LinkedNode *NodeCurrent = NodeHead;
            while (NodeCurrent && Objects::doCompare(NodeCurrent->NodeValue, ElementSource)) NodeCurrent = NodeCurrent->NodeNext, ++NodeIndex;
            if (!NodeCurrent) return -1;
            return NodeIndex;
        }

        bool isContains(const E &ElementSource) const noexcept {
            LinkedNode *NodeCurrent = NodeHead;
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
            LinkedNode *NodeCurrent = NodeHead;
            if (ElementIndex == 0) {
                NodeHead = NodeHead->NodeNext;
                MemoryAllocator::deleteObject(NodeCurrent);
                --NodeSize;
                return;
            }
            bool NodeLast = false;
            if (ElementIndex == NodeSize - 1) NodeLast = true;
            while (--ElementIndex) NodeCurrent = NodeCurrent->NodeNext;
            LinkedNode *NodeNext = NodeCurrent->NodeNext->NodeNext;
            MemoryAllocator::deleteObject(NodeCurrent->NodeNext);
            NodeCurrent->NodeNext = NodeNext;
            if (NodeLast) NodeTail = NodeCurrent;
            --NodeSize;
        }

        void setElement(intmax_t ElementIndex, const E &ElementSource) {
            if (ElementIndex < 0) ElementIndex += NodeSize;
            Collections::doCheckGE(ElementIndex, 0);
            Collections::doCheckL(ElementIndex, NodeSize);
            LinkedNode *NodeCurrent = NodeHead;
            while (ElementIndex--) NodeCurrent = NodeCurrent->NodeNext;
            NodeCurrent->NodeValue = ElementSource;
        }

        String toString() const noexcept override {
            StringStream CharacterStream;
            CharacterStream.addCharacter(u'[');
            if (NodeHead) {
                LinkedNode *NodeCurrent = NodeHead;
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

    /**
     * Support for discrete storage of unique objects
     */
    template<typename E>
    class SingleLinkedSet final : public Object {
    private:
        using LinkedNode = SingleLinkedNode<E>;
        LinkedNode *NodeHead = nullptr, *NodeTail = nullptr;
        intmax_t NodeSize = 0;
    public:
        doEnableCopyAssignConstruct(SingleLinkedSet)
        doEnableMoveAssignConstruct(SingleLinkedSet)

        constexpr SingleLinkedSet() noexcept = default;

        explicit SingleLinkedSet(const SingleLinkedList<E> &ElementSource) noexcept {
            auto *NodeCurrent = ElementSource.NodeHead;
            while (NodeCurrent) {
                addElement(NodeCurrent->NodeValue);
                NodeCurrent = NodeCurrent->NodeNext;
            }
        }

        ~SingleLinkedSet() noexcept {
            doClear();
        }

        void addElement(const E &ElementSource) noexcept {
            if (isContains(ElementSource)) return;
            auto *NodeCurrent = MemoryAllocator::newObject<LinkedNode>(ElementSource);
            if (!NodeHead) {
                NodeHead = NodeCurrent;
                NodeTail = NodeCurrent;
            } else {
                NodeTail->NodeNext = NodeCurrent;
                NodeTail = NodeCurrent;
            }
            ++NodeSize;
        }

        void doAssign(const SingleLinkedSet &ElementSource) noexcept {
            if (Objects::getAddress(ElementSource) == this) return;
            if (!isEmpty()) doClear();
            NodeSize = ElementSource.NodeSize;
            LinkedNode *NodeCurrent = ElementSource.NodeHead;
            while (NodeCurrent) {
                addElement(NodeCurrent->NodeValue);
                NodeCurrent = NodeCurrent->NodeNext;
            }
        }

        void doAssign(SingleLinkedSet<E> &&ElementSource) noexcept {
            if (Objects::getAddress(ElementSource) == this) return;
            if (!isEmpty()) doClear();
            NodeHead = ElementSource.NodeHead;
            NodeSize = ElementSource.NodeSize;
            NodeTail = ElementSource.NodeTail;
            ElementSource.NodeHead = ElementSource.NodeTail = nullptr;
            ElementSource.NodeSize = 0;
        }

        void doClear() noexcept {
            NodeSize = 0;
            NodeTail = nullptr;
            LinkedNode *NodeCurrent = NodeHead, *NodePrevious;
            NodeHead = nullptr;
            while (NodeCurrent) {
                NodePrevious = NodeCurrent;
                NodeCurrent = NodeCurrent->NodeNext;
                MemoryAllocator::deleteObject(NodePrevious);
            }
        }

        SingleLinkedSet doDifference(const SingleLinkedSet &SetSource) const noexcept {
            SingleLinkedSet SetResult;
            LinkedNode *NodeCurrent = NodeHead;
            while (NodeCurrent) {
                if (!SetSource.isContains(NodeCurrent->NodeValue))
                    SetResult.addElement(NodeCurrent->NodeValue);
                NodeCurrent = NodeCurrent->NodeNext;
            }
            return SetResult;
        }

        SingleLinkedSet doIntersection(const SingleLinkedSet &SetSource) const noexcept {
            SingleLinkedSet SetResult;
            LinkedNode *NodeCurrent = NodeHead;
            while (NodeCurrent) {
                if (SetSource.isContains(NodeCurrent->NodeValue))
                    SetResult.addElement(NodeCurrent->NodeValue);
                NodeCurrent = NodeCurrent->NodeNext;
            }
            return SetResult;
        }

        SingleLinkedSet doUnion(const SingleLinkedSet &SetSource) const noexcept {
            SingleLinkedSet SetResult(*this);
            LinkedNode *NodeCurrent = SetSource.NodeHead;
            while (NodeCurrent) {
                SetResult.addElement(NodeCurrent->NodeValue);
                NodeCurrent = NodeCurrent->NodeNext;
            }
            return SetResult;
        }

        const char *getClassName() const noexcept override {
            return "SingleLinkedSet";
        }

        intmax_t getElementSize() const noexcept {
            return NodeSize;
        }

        bool isContains(const E &ElementSource) const noexcept {
            LinkedNode *NodeCurrent = NodeHead;
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
            intmax_t ElementIndex = 0;
            LinkedNode *NodeCurrent = NodeHead;
            while (NodeCurrent && Objects::doCompare(NodeCurrent->NodeValue, ElementSource)) NodeCurrent = NodeCurrent->NodeNext, ++ElementIndex;
            Collections::doCheckL(ElementIndex, NodeSize);
            NodeCurrent = NodeHead;
            if (ElementIndex == 0) {
                NodeHead = NodeHead->NodeNext;
                MemoryAllocator::deleteObject(NodeCurrent);
                --NodeSize;
                return;
            }
            bool NodeLast = false;
            if (ElementIndex == NodeSize - 1) NodeLast = true;
            while (--ElementIndex) NodeCurrent = NodeCurrent->NodeNext;
            LinkedNode *NodeNext = NodeCurrent->NodeNext->NodeNext;
            MemoryAllocator::deleteObject(NodeCurrent->NodeNext);
            NodeCurrent->NodeNext = NodeNext;
            if (NodeLast) NodeTail = NodeCurrent;
            --NodeSize;
        }

        SingleLinkedList<E> toSingleLinkedList() const noexcept {
            SingleLinkedList<E> ListResult;
            LinkedNode *NodeCurrent = NodeHead;
            while (NodeCurrent) {
                ListResult.addElement(NodeCurrent->NodeValue);
                NodeCurrent = NodeCurrent->NodeNext;
            }
            return ListResult;
        }

        String toString() const noexcept override {
            StringStream CharacterStream;
            CharacterStream.addCharacter(u'{');
            if (NodeHead) {
                LinkedNode *NodeCurrent = NodeHead;
                while (NodeCurrent->NodeNext) {
                    CharacterStream.addString(String::valueOf(NodeCurrent->NodeValue));
                    CharacterStream.addCharacter(u',');
                    NodeCurrent = NodeCurrent->NodeNext;
                }
                CharacterStream.addString(String::valueOf(NodeCurrent->NodeValue));
            }
            CharacterStream.addCharacter(u'}');
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
                throw IndexException(String(u"TreeMap<K, V>::getElement(const K&) doSearchCore"));
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
            this->doOrderCore(this->NodeRoot, [&](const K&, const V &MapValueCurrent){
                if (!Objects::doCompare(MapValue, MapValueCurrent)) ValueResult = true;
            });
            return ValueResult;
        }

        bool isEmpty() const noexcept {
            return !this->NodeRoot;
        }

        void removeMapping(const K &MapKey) {
            if (!this->doSearchCore(this->NodeRoot, MapKey)) [[unlikely]]
                throw IndexException(String(u"TreeMap<K(Comparable),V>::removeMapping(const K&) MapKey"));
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
        static void toString(const typename RedBlackTree<E, ::std::nullptr_t>::RedBlackNode *NodeCurrent, StringStream &CharacterStream) noexcept {
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
            this->doOrderCore(this->NodeRoot, [&](const E &ElementSource, ::std::nullptr_t){
                if (!SetSource.isContains(ElementSource)) SetResult.addElement(ElementSource);
            });
            return SetResult;
        }

        TreeSet doIntersection(const TreeSet &SetSource) const noexcept {
            TreeSet SetResult;
            this->doOrderCore(this->NodeRoot, [&](const E &ElementSource, ::std::nullptr_t){
                if (SetSource.isContains(ElementSource)) SetResult.addElement(ElementSource);
            });
            return SetResult;
        }

        TreeSet doUnion(const TreeSet &SetSource) const noexcept {
            TreeSet SetResult;
            this->doOrderCore(this->NodeRoot, [&](const E &ElementSource, ::std::nullptr_t){
                SetResult.addElement(ElementSource);
            });
            SetSource.doOrderCore(SetSource.NodeRoot, [&](const E &ElementSource, ::std::nullptr_t){
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
                throw IndexException(String(u"TreeObject::removeElement(const E&) isContains"));
            this->doRemove(ElementSource);
        }

        String toString() const noexcept override {
            StringStream CharacterStream;
            CharacterStream.addCharacter(u'{');
            toString(this->NodeRoot, CharacterStream);
            CharacterStream.addCharacter(u'}');
            return CharacterStream.toString();
        }
    };

    namespace {
        template<size_t V, size_t ...Vs>
        constexpr size_t getMaximumInteger = V > getMaximumInteger<Vs...> ? V : getMaximumInteger<Vs...>;

        template<size_t V>
        constexpr size_t getMaximumInteger<V> = V;

        template<typename ...Ts>
        struct VariantUtility;

        template<typename T, typename ...Ts>
        struct VariantUtility<T, Ts...> {
            static void doCopy(const ::std::type_info *VariantType, void *VariantDataSource, void *VariantDataDestination) {
                if (VariantType == &typeid(T)) new (VariantDataDestination) T(*((T*) VariantDataSource));
                else VariantUtility<Ts...>::doCopy(VariantType, VariantDataSource, VariantDataDestination);
            }

            static void doDestroy(const ::std::type_info *VariantType, void *VariantData) {
                if (VariantType == &typeid(T)) ((T*) VariantData)->~T();
                else VariantUtility<Ts...>::doDestroy(VariantType, VariantData);
            }

            static void doMove(const ::std::type_info *VariantType, void *VariantDataSource, void *VariantDataDestination) {
                if (VariantType == &typeid(T)) new (VariantDataDestination) T(Objects::doMove(*((T*) VariantDataSource)));
                else VariantUtility<Ts...>::doMove(VariantType, VariantDataSource, VariantDataDestination);
            }
        };

        template<>
        struct VariantUtility<> {
            static void doCopy(const ::std::type_info*, void*, void*) {}

            static void doDestroy(const ::std::type_info*, void*) {}

            static void doMove(const ::std::type_info*, void*, void*) {}
        };

        template<typename T, typename ...Ts>
        constexpr bool isContainsType = true;

        template<typename T, typename T1, typename ...T2>
        constexpr bool isContainsType<T, T1, T2...> = ::std::is_same_v<T, T1> || isContainsType<T, T2...>;

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
        bool VariantValue = false;
    public:
        doEnableCopyAssignConstruct(Variant)
        doEnableMoveAssignConstruct(Variant)

        constexpr Variant() noexcept = default;

        template<typename T> requires ContainsType<T, Ts...>
        Variant(const T &VariantSource) {
            new (VariantData) T(VariantSource);
            VariantType = &typeid(T);
        }

        template<typename T> requires ContainsType<T, Ts...>
        Variant(T &&VariantSource) {
            new (VariantData) T(Objects::doMove(VariantSource));
            VariantType = &typeid(T);
        }

        ~Variant() {
            VariantUtility<Ts...>::doDestroy(VariantType, VariantData);
        }

        void doAssign(const Variant<Ts...> &VariantSource) {
            if (Objects::getAddress(VariantSource) == this) return;
            VariantUtility<Ts...>::doDestroy(VariantType, VariantData);
            VariantUtility<Ts...>::doCopy(VariantSource.VariantType, VariantSource.VariantData, VariantData);
            VariantType = VariantSource.VariantType;
        }

        void doAssign(Variant<Ts...> &&VariantSource) {
            if (Objects::getAddress(VariantSource) == this) return;
            VariantUtility<Ts...>::doDestroy(VariantType, VariantData);
            VariantUtility<Ts...>::doMove(VariantSource.VariantType, VariantSource.VariantData, VariantData);
            VariantType = VariantSource.VariantType;
            VariantSource.VariantType = nullptr;
        }

        const char *getClassName() const noexcept override {
            return "Variant";
        }

        template<typename T>
        auto getValue() {
            if (&typeid(T) != VariantType) [[unlikely]]
                throw TypeException(String(u"Variant::getValue<T>() T"));
            return *((T*) VariantData);
        }
    };

    template<typename E>
    using ArrayQueue = ContainerQueue<E, ArrayList<E>>;

    template<typename E>
    using ArrayStack = ContainerStack<E, ArrayList<E>>;

    template<typename E>
    using DoubleLinkedQueue = ContainerQueue<E, DoubleLinkedList<E>>;

    template<typename E>
    using DoubleLinkedStack = ContainerStack<E, DoubleLinkedList<E>>;

    template<typename E>
    using SingleLinkedQueue = ContainerQueue<E, SingleLinkedList<E>>;

    template<typename E>
    using SingleLinkedStack = ContainerStack<E, SingleLinkedList<E>>;
}

#endif
