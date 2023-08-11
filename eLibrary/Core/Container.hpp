#pragma once

#include <Core/Memory.hpp>

#include <array>
#include <forward_list>
#include <list>
#include <optional>
#include <vector>

namespace eLibrary::Core {
    template<typename E>
    class Collection : public Object {
    public:
        virtual bool isEmpty() const noexcept = 0;
    };

    template<typename E>
    class List : public Collection<E> {};

    template<Comparable K, typename V>
    class Map : public Object {
    public:
        virtual bool isContainsKey(const K&) const noexcept = 0;

        virtual bool isContainsValue(const V&) const noexcept = 0;

        virtual bool isEmpty() const noexcept = 0;
    };

    template<typename E>
    class Queue : public Object {
    public:
        virtual bool isEmpty() const noexcept = 0;
    };

    template<typename E>
    class Set : public Collection<E> {};

    template<typename E>
    class Stack : public Object {};

    class Any final : public Object {
    private:
        enum class AnyOperation {
            OperationCopy, OperationDestroy
        };

        void *AnyValue = nullptr;
        const std::type_info *AnyType = nullptr;
        void (*AnyFunction)(AnyOperation, Any*, void*) = nullptr;

        template<typename T>
        static void doExecute(AnyOperation ManagerOperation, Any *ManagerObject, void *ManagerSource) {
            switch (ManagerOperation) {
                case AnyOperation::OperationCopy:
                    ManagerObject->AnyValue = new T(* (const T*) ManagerSource);
                    break;
                case AnyOperation::OperationDestroy:
                    delete (const T*) ManagerObject->AnyValue;
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

        ~Any() noexcept {
            doReset();
        }

        void doAssign(const Any &AnySource) {
            if (!AnySource.hasValue()) doReset();
            else if (Objects::getAddress(AnySource) != this) {
                doReset();
                AnyFunction = AnySource.AnyFunction;
                AnyFunction(AnyOperation::OperationCopy, this, AnySource.AnyValue);
                AnyType = AnySource.AnyType;
            }
        }

        void doAssign(Any &&AnySource) noexcept {
            if (!AnySource.hasValue()) doReset();
            else if (Objects::getAddress(AnySource) != this) {
                doReset();
                AnyFunction = AnySource.AnyFunction;
                AnyType = AnySource.AnyType;
                AnyValue = AnySource.AnyValue;
                AnySource.doReset();
            }
        }

        template<typename T>
        void doAssign(const T &AnySource) noexcept(std::is_nothrow_copy_constructible<T>::value) {
            doReset();
            AnyFunction = &doExecute<T>;
            AnyType = &typeid(T);
            AnyValue = new T(AnySource);
        }

        void doReset() noexcept {
            if (AnyFunction && AnyValue) AnyFunction(AnyOperation::OperationDestroy, this, nullptr);
            AnyFunction = nullptr;
            AnyType = nullptr;
            AnyValue = nullptr;
        }

        template<typename T>
        auto getValue() const {
            if (&typeid(T) != AnyType) throw TypeException(String(u"Any::getValue<T>()"));
            if (!hasValue()) throw Exception(String(u"Any::getValue<T>() hasValue"));
            return *((T*) AnyValue);
        }

        auto getValueType() noexcept {
            return AnyType;
        }

        bool hasValue() const noexcept {
            return AnyValue;
        }
    };

    class Arrays final : public Object {
    public:
        constexpr Arrays() noexcept = delete;

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
    };

    template<typename E>
    class ArrayIterator final : public Object {
    private:
        E *ElementCurrent;
    public:
        typedef intmax_t difference_type;
        typedef std::random_access_iterator_tag iterator_category;

        constexpr ArrayIterator(E *ElementSource) noexcept : ElementCurrent(ElementSource) {}

        ArrayIterator operator+(intmax_t ElementIndex) noexcept {
            return {ElementCurrent + ElementIndex};
        }

        ArrayIterator &operator++() noexcept {
            ++ElementCurrent;
            return *this;
        }

        ArrayIterator &operator-(intmax_t ElementIndex) noexcept {
            return {ElementCurrent - ElementIndex};
        }

        ArrayIterator &operator--() noexcept {
            --ElementCurrent;
            return *this;
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

        constexpr Array() noexcept = default;

        template<size_t ElementSourceSize>
        explicit Array(const std::array<E, ElementSourceSize> &ElementSource) : ElementSize(ElementSourceSize) {
            ElementContainer = MemoryAllocator::newArray<E>(ElementSize);
            Arrays::doCopy(ElementSource.begin(), ElementSource.end(), ElementContainer);
        }

        Array(std::initializer_list<E> ElementSource) : ElementSize(ElementSource.size()) {
            ElementContainer = MemoryAllocator::newArray<E>(ElementSize);
            Arrays::doCopy(ElementSource.begin(), ElementSource.end(), ElementContainer);
        }

        template<typename ...Es>
        explicit Array(E ElementCurrent, Es ...ElementList) {
            ElementContainer = MemoryAllocator::newArray<E>(sizeof...(ElementList) + 1);
            doInitialize(ElementCurrent, ElementList...);
        }

        ~Array() noexcept {
            ElementSize = 0;
            delete[] ElementContainer;
            ElementContainer = nullptr;
        }

        static Array doAllocate(intmax_t ElementSize) {
            return {MemoryAllocator::newArray<E>(ElementSize), ElementSize};
        }

        void doAssign(const Array<E> &ElementSource) {
            if (Objects::getAddress(ElementSource) == this) return;
            delete[] ElementContainer;
            ElementContainer = MemoryAllocator::newArray<E>(ElementSize = ElementSource.ElementSize);
            Arrays::doCopy(ElementSource.ElementContainer, ElementSize, ElementContainer);
        }

        const E &getElement(intmax_t ElementIndex) const {
            if (ElementIndex < 0) ElementIndex += ElementSize;
            if (ElementIndex < 0 || ElementIndex >= ElementSize) throw IndexException(String(u"Array::getElement(intmax_t) ElementIndex"));
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
            if (ElementIndex < 0 || ElementIndex >= ElementSize) throw IndexException(String(u"Array::setElement(intmax_t, const E&) ElementIndex"));
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
    class ArrayList final : public List<E> {
    private:
        intmax_t ElementCapacity = 0, ElementSize = 0;
        E *ElementContainer = nullptr;

        ArrayList(E *ElementSource, intmax_t ElementSourceSize) : ElementCapacity(1), ElementSize(ElementSourceSize) {
            while (ElementCapacity < ElementSourceSize) ElementCapacity <<= 1;
            ElementContainer = MemoryAllocator::newArray<E>(ElementCapacity);
            Arrays::doCopy(ElementSource, ElementSource + ElementSourceSize, ElementContainer);
        }

        template<typename ...Es>
        void doInitialize(E ElementCurrent, Es ...ElementList) noexcept {
            ElementContainer[ElementSize++] = ElementCurrent;
            if constexpr (sizeof...(ElementList)) doInitialize(ElementList...);
        }
    public:
        doEnableCopyAssignConstruct(ArrayList)

        constexpr ArrayList() noexcept = default;

        explicit ArrayList(const Array<E> &ElementSource) : ElementCapacity(1), ElementSize(ElementSource.getElementSize()) {
            while (ElementCapacity < ElementSize) ElementCapacity <<= 1;
            ElementContainer = MemoryAllocator::newArray<E>(ElementCapacity);
            Arrays::doCopy(ElementSource.ElementContainer, ElementSize, ElementContainer);
        }

        template<typename ...Es>
        explicit ArrayList(Es ...ElementList) : ElementCapacity(1) {
            while (ElementCapacity < sizeof...(ElementList)) ElementCapacity <<= 1;
            ElementContainer = MemoryAllocator::newArray<E>(ElementCapacity);
            doInitialize(ElementList...);
        }

        ArrayList(std::initializer_list<E> ElementList) : ElementCapacity(1), ElementSize((intmax_t) ElementList.size()) {
            while (ElementCapacity < ElementSize)
                ElementCapacity <<= 1;
            ElementContainer = MemoryAllocator::newArray<E>(ElementCapacity);
            Arrays::doCopy(ElementList.begin(), ElementList.end(), ElementContainer);
        }

        ~ArrayList() noexcept {
            doClear();
        }

        void addElement(const E &ElementSource) {
            if (!ElementCapacity) ElementContainer = MemoryAllocator::newArray<E>(ElementCapacity = 1);
            if (ElementSize == ElementCapacity) {
                E *ElementBuffer = MemoryAllocator::newArray<E>(ElementSize);
                Arrays::doMove(ElementContainer, ElementSize, ElementBuffer);
                delete[] ElementContainer;
                ElementContainer = MemoryAllocator::newArray<E>(ElementCapacity <<= 1);
                Arrays::doMove(ElementBuffer, ElementSize, ElementContainer);
                delete[] ElementBuffer;
            }
            ElementContainer[ElementSize++] = ElementSource;
        }

        void addElement(intmax_t ElementIndex, const E &ElementSource) {
            if (ElementIndex < 0) ElementIndex += ElementSize + 1;
            if (ElementIndex < 0 || ElementIndex > ElementSize)
                throw IndexException(String(u"ArrayList<E>::addElement(intmax_t, const E&) ElementIndex"));
            if (!ElementCapacity) ElementContainer = MemoryAllocator::newArray<E>(ElementCapacity = 1);
            if (ElementSize == ElementCapacity) {
                E *ElementBuffer = MemoryAllocator::newArray<E>(ElementSize);
                Arrays::doMove(ElementContainer, ElementContainer + ElementSize, ElementBuffer);
                delete[] ElementContainer;
                ElementContainer = MemoryAllocator::newArray<E>(ElementCapacity <<= 1);
                Arrays::doMove(ElementBuffer, ElementBuffer + ElementSize, ElementContainer);
                delete[] ElementBuffer;
            }
            Arrays::doMoveBackward(ElementContainer + ElementIndex, ElementContainer + ElementSize, ElementContainer + ElementSize + 1);
            ElementContainer[ElementIndex] = ElementSource;
            ++ElementSize;
        }

        void doAssign(const ArrayList<E> &ElementSource) {
            if (Objects::getAddress(ElementSource) == this) return;
            delete[] ElementContainer;
            if (ElementCapacity) ElementContainer = MemoryAllocator::newArray<E>(ElementCapacity = ElementSource.ElementCapacity);
            Arrays::doCopy(ElementSource.ElementContainer, (ElementSize = ElementSource.ElementSize), ElementContainer);
        }

        void doClear() noexcept {
            ElementCapacity = 0;
            ElementSize = 0;
            delete[] ElementContainer;
            ElementContainer = nullptr;
        }

        ArrayList<E> doConcat(const ArrayList<E> &ElementSource) const {
            E *ElementBuffer = MemoryAllocator::newArray<E>(ElementSize + ElementSource.ElementSize);
            Arrays::doCopy(ElementContainer, ElementSize, ElementBuffer);
            Arrays::doCopy(ElementSource.ElementContainer, ElementSource.ElementSize, ElementBuffer + ElementSize);
            return {ElementBuffer, ElementSize + ElementSource.ElementSize};
        }

        void doReverse() {
            E *ElementBuffer = MemoryAllocator::newArray<E>(ElementSize);
            Arrays::doMove(ElementContainer, ElementSize, ElementBuffer);
            Arrays::doMoveReverse(ElementBuffer, ElementBuffer + ElementSize, ElementContainer);
            delete[] ElementBuffer;
        }

        const E &getElement(intmax_t ElementIndex) const {
            if (ElementIndex < 0) ElementIndex += ElementSize;
            if (ElementIndex < 0 || ElementIndex >= ElementSize)
                throw IndexException(String(u"ArrayList<E>::getElement(intmax_t) ElementIndex"));
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

        bool isEmpty() const noexcept override {
            return !ElementSize;
        }

        void removeElement(const E &ElementSource) {
            intmax_t ElementIndex = indexOf(ElementSource);
            if (ElementIndex == -1) throw IndexException(String(u"ArrayList<E>::removeElement(const E&) indexOf"));
            removeIndex(ElementIndex);
        }

        void removeIndex(intmax_t ElementIndex) {
            if (ElementIndex < 0) ElementIndex += ElementSize;
            if (ElementIndex < 0 || ElementIndex >= ElementSize)
                throw IndexException(String(u"ArrayList<E>::removeIndex(intmax_t) ElementIndex"));
            Arrays::doMove(ElementContainer + ElementIndex + 1, ElementContainer + ElementSize, ElementContainer + ElementIndex);
            if (ElementCapacity == 1) doClear();
            else if (--ElementSize <= ElementCapacity >> 1) {
                E *ElementBuffer = MemoryAllocator::newArray<E>(ElementSize);
                Arrays::doMove(ElementContainer, ElementSize, ElementBuffer);
                delete[] ElementContainer;
                ElementContainer = MemoryAllocator::newArray<E>(ElementCapacity >>= 1);
                Arrays::doMove(ElementBuffer, ElementSize, ElementContainer);
                delete[] ElementBuffer;
            }
        }

        void setElement(intmax_t ElementIndex, const E &ElementSource) {
            if (ElementIndex < 0) ElementIndex += ElementSize;
            if (ElementIndex < 0 || ElementIndex >= ElementSize)
                throw IndexException(String(u"ArrayList<E>::setElement(intmax_t, const E&) ElementIndex"));
            ElementContainer[ElementIndex] = ElementSource;
        }

        std::vector<E> toSTLVector() const noexcept {
            return std::vector<E>(ElementContainer, ElementContainer + ElementSize);
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
    class ArraySet final : public Set<E> {
    private:
        uintmax_t ElementCapacity = 0, ElementSize = 0;
        E *ElementContainer = nullptr;
    public:
        doEnableCopyAssignConstruct(ArraySet)

        constexpr ArraySet() noexcept = default;

        ArraySet(const ArrayList<E> &ElementSource) noexcept {
            for (intmax_t ElementIndex = 0;ElementIndex < ElementSource.getElementSize();++ElementIndex)
                addElement(ElementSource.getElement(ElementIndex));
        }

        ~ArraySet() noexcept {
            doClear();
        }

        void addElement(const E &ElementSource) noexcept {
            if (isContains(ElementSource)) return;
            if (!ElementCapacity) ElementContainer = MemoryAllocator::newArray<E>(ElementCapacity = 1);
            if (ElementSize == ElementCapacity) {
                E *ElementBuffer = MemoryAllocator::newArray<E>(ElementSize);
                Arrays::doMove(ElementContainer, ElementContainer + ElementSize, ElementBuffer);
                delete[] ElementContainer;
                ElementContainer = MemoryAllocator::newArray<E>(ElementCapacity <<= 1);
                Arrays::doMove(ElementBuffer, ElementBuffer + ElementSize, ElementContainer);
                delete[] ElementBuffer;
            }
            ElementContainer[ElementSize++] = ElementSource;
        }

        void doAssign(const ArraySet &SetSource) {
            if (Objects::getAddress(SetSource) == this) return;
            delete[] ElementContainer;
            ElementCapacity = SetSource.ElementCapacity;
            if (ElementCapacity) ElementContainer = MemoryAllocator::newArray<E>(ElementCapacity);
            Arrays::doCopy(SetSource.ElementContainer, SetSource.ElementContainer + (ElementSize = SetSource.ElementSize), ElementContainer);
        }

        void doClear() noexcept {
            ElementCapacity = 0;
            ElementSize = 0;
            delete[] ElementContainer;
            ElementContainer = nullptr;
        }

        ArraySet doDifference(const ArraySet &SetSource) const noexcept {
            ArraySet SetResult;
            for (uintmax_t ElementIndex = 0; ElementIndex < ElementSize; ++ElementIndex)
                if (!SetSource.isContains(ElementContainer[ElementIndex])) SetResult.addElement(ElementContainer[ElementIndex]);
            return SetResult;
        }

        ArraySet doIntersection(const ArraySet &SetSource) const noexcept {
            ArraySet SetResult;
            for (uintmax_t ElementIndex = 0; ElementIndex < ElementSize; ++ElementIndex)
                if (SetSource.isContains(ElementContainer[ElementIndex])) SetResult.addElement(ElementContainer[ElementIndex]);
            return SetResult;
        }

        ArraySet doUnion(const ArraySet &SetSource) const noexcept {
            ArraySet SetResult(*this);
            for (uintmax_t ElementIndex = 0; ElementIndex < SetSource.ElementSize; ++ElementIndex)
                SetResult.addElement(SetSource.ElementContainer[ElementIndex]);
            return SetResult;
        }

        intmax_t getElementSize() const noexcept {
            return ElementSize;
        }

        bool isContains(const E &ElementSource) const noexcept {
            for (uintmax_t ElementIndex = 0; ElementIndex < ElementSize; ++ElementIndex)
                if (!Objects::doCompare(ElementContainer[ElementIndex], ElementSource)) return true;
            return false;
        }

        bool isEmpty() const noexcept override {
            return !ElementSize;
        }

        void removeElement(const E &ElementSource) {
            uintmax_t ElementIndex = 0;
            for (; ElementIndex < ElementSize; ++ElementIndex)
                if (!Objects::doCompare(ElementContainer[ElementIndex], ElementSource)) break;
            if (ElementIndex == ElementSize) throw IndexException(String(u"ArraySet::removeElement(const E&) ElementSource"));
            Arrays::doMove(ElementContainer + ElementIndex + 1, ElementContainer + ElementSize, ElementContainer + ElementIndex);
            if (ElementCapacity == 1) doClear();
            else if (--ElementSize <= ElementCapacity >> 1) {
                E *ElementBuffer = MemoryAllocator::newArray<E>(ElementSize);
                Arrays::doMove(ElementContainer, ElementContainer + ElementSize, ElementBuffer);
                delete[] ElementContainer;
                ElementContainer = MemoryAllocator::newArray<E>(ElementCapacity >>= 1);
                Arrays::doMove(ElementBuffer, ElementBuffer + ElementSize, ElementContainer);
                delete[] ElementBuffer;
            }
        }

        ArrayList<E> toArrayList() const noexcept {
            ArrayList<E> ListResult;
            for (uintmax_t ElementIndex = 0; ElementIndex < ElementSize;++ElementIndex)
                ListResult.addElement(ElementContainer[ElementIndex]);
            return ListResult;
        }

        String toString() const noexcept override {
            StringStream CharacterStream;
            CharacterStream.addCharacter(u'{');
            for (uintmax_t ElementIndex = 0; ElementIndex + 1 < ElementSize; ++ElementIndex) {
                CharacterStream.addString(String::valueOf(ElementContainer[ElementIndex]));
                CharacterStream.addCharacter(u',');
            }
            if (ElementSize) CharacterStream.addString(String::valueOf(ElementContainer[ElementSize - 1]));
            CharacterStream.addCharacter(u'}');
            return CharacterStream.toString();
        }

        ArrayIterator<E> begin() const noexcept {
            return {ElementContainer};
        }

        ArrayIterator<E> end() const noexcept {
            return {ElementContainer + ElementSize};
        }
    };

    template<typename E>
    class DoubleLinkedIterator final : public Object {
    private:
        struct LinkedNode {
            E NodeValue;
            LinkedNode *NodeNext, *NodePrevious;
        } *NodeCurrent;
    public:
        typedef std::forward_iterator_tag iterator_category;

        constexpr DoubleLinkedIterator(LinkedNode *NodeSource) noexcept : NodeCurrent(NodeSource) {}

        DoubleLinkedIterator &operator++() {
            NodeCurrent = NodeCurrent->NodeNext;
            return *this;
        }

        E &operator*() noexcept {
            return NodeCurrent->NodeValue;
        }

        bool operator==(const DoubleLinkedIterator &IteratorSource) noexcept {
            return NodeCurrent == IteratorSource.NodeCurrent;
        }

        bool operator!=(const DoubleLinkedIterator &IteratorSource) noexcept {
            return NodeCurrent != IteratorSource.NodeCurrent;
        }
    };

    /**
     * Support for discrete storage of objects
     */
    template<typename E>
    class DoubleLinkedList final : public List<E> {
    private:
        struct LinkedNode final {
            E NodeValue;
            LinkedNode *NodeNext, *NodePrevious;

            constexpr explicit LinkedNode(const E &NodeValueSource) noexcept: NodeValue(NodeValueSource), NodeNext(nullptr), NodePrevious(nullptr) {}
        } *NodeHead = nullptr, *NodeTail = nullptr;

        intmax_t NodeSize = 0;
    public:
        doEnableCopyAssignConstruct(DoubleLinkedList)

        constexpr DoubleLinkedList() noexcept = default;

        DoubleLinkedList(std::initializer_list<E> ElementList) noexcept : NodeSize(ElementList.size()) {
            for (const E &ElementCurrent : ElementList) addElement(ElementCurrent);
        }

        ~DoubleLinkedList() noexcept {
            doClear();
        }

        void addElement(const E &ElementSource) {
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
            if (ElementIndex < 0 || ElementIndex > NodeSize)
                throw IndexException(String(u"DoubleLinkedList<E>::addElement(intmax_t, const E&) ElementIndex"));
            if (ElementIndex == NodeSize) {
                addElement(ElementSource);
                return;
            }
            if (ElementIndex == 0) {
                auto *NodeCurrent = MemoryAllocator::newObject<LinkedNode>(ElementSource);
                NodeCurrent->NodeNext = NodeHead;
                NodeHead->NodePrevious = NodeCurrent;
                NodeHead = NodeCurrent;
                ++NodeSize;
                return;
            }
            LinkedNode *NodeCurrent = NodeHead;
            while (--ElementIndex) NodeCurrent = NodeCurrent->NodeNext;
            auto *NodeTarget = MemoryAllocator::newObject<LinkedNode>(ElementSource);
            NodeTarget->NodePrevious = NodeCurrent;
            NodeTarget->NodeNext = NodeCurrent->NodeNext;
            NodeCurrent->NodeNext->NodePrevious = NodeTarget;
            NodeCurrent->NodeNext = NodeTarget;
            ++NodeSize;
        }

        void doAssign(const DoubleLinkedList<E> &ElementSource) {
            if (Objects::getAddress(ElementSource) == this) return;
            if (!isEmpty()) doClear();
            NodeSize = ElementSource.NodeSize;
            LinkedNode *NodeCurrent = ElementSource.NodeHead;
            while (NodeCurrent) {
                addElement(NodeCurrent->NodeValue);
                NodeCurrent = NodeCurrent->NodeNext;
            }
        }

        void doClear() noexcept {
            NodeSize = 0;
            NodeTail = nullptr;
            LinkedNode *NodeCurrent = NodeHead, *NodePrevious;
            NodeHead = nullptr;
            while (NodeCurrent) {
                NodePrevious = NodeCurrent;
                NodeCurrent = NodeCurrent->NodeNext;
                delete NodePrevious;
            }
        }

        void doReverse() {
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

        const E &getElement(intmax_t ElementIndex) const {
            if (ElementIndex < 0) ElementIndex += NodeSize;
            if (ElementIndex < 0 || ElementIndex >= NodeSize)
                throw IndexException(String(u"DoubleLinkedList<E>::getElement(intmax_t) ElementIndex"));
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

        bool isEmpty() const noexcept override {
            return !NodeSize;
        }

        void removeElement(const E &ElementSource) {
            intmax_t ElementIndex = indexOf(ElementSource);
            if (ElementIndex == -1) throw IndexException(String(u"DoubleLinkedList<E>::removeElement(const E&) ElementSource"));
            removeIndex(ElementIndex);
        }

        void removeIndex(intmax_t ElementIndex) {
            if (ElementIndex < 0) ElementIndex += NodeSize;
            if (ElementIndex < 0 || ElementIndex >= NodeSize)
                throw IndexException(String(u"DoubleLinkedList<E>::removeIndex(intmax_t) ElementIndex"));
            LinkedNode *NodeCurrent;
            if (NodeSize == 1) {
                delete NodeHead;
                NodeHead = NodeTail = nullptr;
            } else if (ElementIndex == 0) {
                NodeHead = NodeHead->NodeNext;
                delete NodeHead->NodePrevious;
                NodeHead->NodePrevious = nullptr;
            } else if (ElementIndex == NodeSize - 1) {
                NodeTail = NodeTail->NodePrevious;
                delete NodeTail->NodeNext;
                NodeTail->NodeNext = nullptr;
            } else if (ElementIndex < (NodeSize >> 1)) {
                NodeCurrent = NodeHead;
                while (--ElementIndex) NodeCurrent = NodeCurrent->NodeNext;
                NodeCurrent->NodeNext = NodeCurrent->NodeNext->NodeNext;
                delete NodeCurrent->NodeNext->NodePrevious;
                NodeCurrent->NodeNext->NodePrevious = NodeCurrent;
            } else {
                NodeCurrent = NodeTail;
                ElementIndex = NodeSize - ElementIndex - 1;
                while (--ElementIndex) NodeCurrent = NodeCurrent->NodePrevious;
                NodeCurrent->NodePrevious = NodeCurrent->NodePrevious->NodePrevious;
                delete NodeCurrent->NodePrevious->NodeNext;
                NodeCurrent->NodePrevious->NodeNext = NodeCurrent;
            }
            --NodeSize;
        }

        void setElement(intmax_t ElementIndex, const E &ElementSource) {
            if (ElementIndex < 0) ElementIndex += NodeSize;
            if (ElementIndex < 0 || ElementIndex >= NodeSize)
                throw IndexException(String(u"DoubleLinkedList<E>::setElement(intmax_t, const E&) ElementIndex"));
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

        std::list<E> toSTLList() const noexcept {
            std::list<E> ListResult;
            LinkedNode *NodeCurrent = NodeHead;
            while (NodeCurrent) {
                ListResult.push_back(NodeCurrent->NodeValue);
                NodeCurrent = NodeCurrent->NodeNext;
            }
            return ListResult;
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
    };

    /**
     * Support for discrete storage of unique objects
     */
    template<typename E>
    class DoubleLinkedSet final : public Set<E> {
    private:
        struct LinkedNode final {
            E NodeValue;
            LinkedNode *NodeNext, *NodePrevious;

            explicit LinkedNode(const E &Value) noexcept: NodeValue(Value), NodeNext(nullptr), NodePrevious(nullptr) {}
        } *NodeHead = nullptr, *NodeTail = nullptr;

        intmax_t NodeSize = 0;
    public:
        doEnableCopyAssignConstruct(DoubleLinkedSet)

        constexpr DoubleLinkedSet() noexcept = default;

        ~DoubleLinkedSet() noexcept {
            doClear();
        }

        void addElement(const E &ElementSource) {
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

        void doAssign(const DoubleLinkedSet &ElementSource) {
            if (Objects::getAddress(ElementSource) == this) return;
            if (!isEmpty()) doClear();
            NodeSize = ElementSource.NodeSize;
            LinkedNode *NodeCurrent = ElementSource.NodeHead;
            while (NodeCurrent) {
                addElement(NodeCurrent->NodeValue);
                NodeCurrent = NodeCurrent->NodeNext;
            }
        }

        void doClear() noexcept {
            NodeSize = 0;
            NodeTail = nullptr;
            LinkedNode *NodeCurrent = NodeHead, *NodePrevious;
            NodeHead = nullptr;
            while (NodeCurrent) {
                NodePrevious = NodeCurrent;
                NodeCurrent = NodeCurrent->NodeNext;
                delete NodePrevious;
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

        bool isEmpty() const noexcept override {
            return !NodeSize;
        }

        void removeElement(const E &ElementSource) {
            intmax_t ElementIndex = 0;
            LinkedNode *NodeCurrent = NodeHead;
            while (NodeCurrent && Objects::doCompare(NodeCurrent->NodeValue, ElementSource)) NodeCurrent = NodeCurrent->NodeNext, ++ElementIndex;
            if (!NodeCurrent) throw IndexException(String(u"DoubleLinkedSet<E>::removeElement(const E&) ElementSource"));
            if (ElementIndex == 0) {
                NodeHead = NodeHead->NodeNext;
                if (NodeHead->NodePrevious) delete NodeHead->NodePrevious;
                NodeHead->NodePrevious = nullptr;
            } else if (ElementIndex == NodeSize - 1) {
                NodeTail = NodeTail->NodePrevious;
                if (NodeTail->NodeNext) delete NodeTail->NodeNext;
                NodeTail->NodeNext = nullptr;
            } else if (ElementIndex < (NodeSize >> 1)) {
                NodeCurrent = NodeHead;
                while (--ElementIndex) NodeCurrent = NodeCurrent->NodeNext;
                NodeCurrent->NodeNext = NodeCurrent->NodeNext->NodeNext;
                delete NodeCurrent->NodeNext->NodePrevious;
                NodeCurrent->NodeNext->NodePrevious = NodeCurrent;
            } else {
                NodeCurrent = NodeTail;
                ElementIndex = NodeSize - ElementIndex - 1;
                while (--ElementIndex) NodeCurrent = NodeCurrent->NodePrevious;
                NodeCurrent->NodePrevious = NodeCurrent->NodePrevious->NodePrevious;
                delete NodeCurrent->NodePrevious->NodeNext;
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
    };

    template<Comparable K, typename V>
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
                    delete NodeChildLeft;
                    NodeChildLeft = nullptr;
                }
                if (NodeChildRight) {
                    delete NodeChildRight;
                    NodeChildRight = nullptr;
                }
            }
        } *NodeRoot = nullptr;

        template<typename OperationType>
        static void doOrderCore(RedBlackNode *NodeCurrent, OperationType Operation) noexcept(std::is_nothrow_invocable<OperationType, K, V>::value) {
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

        template<Comparable, typename>
        friend class TreeMap;

        template<Comparable>
        friend class TreeSet;
    public:
        doEnableCopyAssignConstruct(RedBlackTree)

        constexpr RedBlackTree() noexcept = default;

        ~RedBlackTree() noexcept {
            if (NodeRoot) {
                delete NodeRoot;
                NodeRoot = nullptr;
            }
        }

        void doAssign(const RedBlackTree<K, V> &TreeSource) noexcept {
            if (Objects::getAddress(TreeSource) == this) return;
            delete NodeRoot;
            TreeSource.doOrderCore(TreeSource.NodeRoot, [&](const K &NodeKeySource, const V &NodeValueSource){
                doInsert(NodeKeySource, NodeValueSource);
            });
        }

        void doInsert(const K &NodeKey, const V &NodeValue) {
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
        void doOrder(const OperationType &Operation) noexcept(std::is_nothrow_invocable<OperationType, K, V>::value) {
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

        std::optional<V> doSearch(const K &NodeKey) const noexcept {
            RedBlackNode *NodeResult = doSearchCore(NodeRoot, NodeKey);
            if (NodeResult == nullptr) return std::nullopt;
            return NodeResult->NodeValue;
        }

        uintmax_t getHeight() const noexcept {
            return NodeRoot ? getHeightCore(NodeRoot) : 0;
        }

        uintmax_t getSize() const noexcept {
            return NodeRoot ? getSizeCore(NodeRoot) : 0;
        }
    };

    /**
     * Support for interval modification and summation
     */
    template<std::integral T>
    class SegmentTree final : public Object {
    private:
        T ElementCount;
        T *ElementList;
        T *ElementMark;
        T *ElementTree;

        void doBuildCore(T ElementCurrent, T ElementStart, T ElementStop) {
            if (ElementStart == ElementStop) {
                ElementTree[ElementCurrent] = ElementList[ElementStart];
                return;
            }
            doBuildCore(ElementCurrent << 1, ElementStart, (ElementStart + ElementStop) >> 1);
            doBuildCore(ElementCurrent << 1 | 1, ((ElementStart + ElementStop) >> 1) + 1, ElementStop);
            ElementTree[ElementCurrent] = ElementTree[ElementCurrent << 1] + ElementTree[ElementCurrent << 1 | 1];
        }

        void doPushDown(T ElementCurrent, T ElementStart, T ElementStop) noexcept {
            if (!ElementMark[ElementCurrent]) return;
            ElementMark[ElementCurrent << 1] += ElementMark[ElementCurrent];
            ElementTree[ElementCurrent << 1] += ElementMark[ElementCurrent] * (((ElementStart + ElementStop) >> 1) - ElementStart + 1);
            ElementMark[ElementCurrent << 1 | 1] += ElementMark[ElementCurrent];
            ElementTree[ElementCurrent << 1 | 1] += ElementMark[ElementCurrent] * (ElementStop - (((ElementStart + ElementStop) >> 1) + 1) + 1);
            ElementMark[ElementCurrent] = 0;
        }

        T doQueryCore(T ElementCurrent, T ElementStart, T ElementStop, T ElementTargetStart, T ElementTargetStop) noexcept {
            if (ElementTargetStart <= ElementStart && ElementStop <= ElementTargetStop) return ElementTree[ElementCurrent];
            T ElementSum = 0;
            doPushDown(ElementCurrent, ElementStart, ElementStop);
            if (ElementTargetStart <= (ElementStart + ElementStop) >> 1)
                ElementSum = doQueryCore(ElementCurrent << 1, ElementStart, (ElementStart + ElementStop) >> 1, ElementTargetStart, ElementTargetStop);
            if (ElementTargetStop > (ElementStart + ElementStop) >> 1)
                ElementSum += doQueryCore(ElementCurrent << 1 | 1, ((ElementStart + ElementStop) >> 1) + 1, ElementStop, ElementTargetStart, ElementTargetStop);
            return ElementSum;
        }

        void doUpdateCore(T ElementCurrent, T ElementStart, T ElementStop, T ElementTargetStart, T ElementTargetStop, T ElementChange) noexcept {
            if (ElementTargetStart <= ElementStart && ElementStop <= ElementTargetStop) {
                ElementMark[ElementCurrent] += ElementChange;
                ElementTree[ElementCurrent] += ElementChange * (ElementStop - ElementStart + 1);
                return;
            }
            doPushDown(ElementCurrent, ElementStart, ElementStop);
            if (ElementTargetStart <= (ElementStart + ElementStop) >> 1) doUpdateCore(ElementCurrent << 1, ElementStart, (ElementStart + ElementStop) >> 1, ElementTargetStart, ElementTargetStop, ElementChange);
            if (ElementTargetStop > (ElementStart + ElementStop) >> 1) doUpdateCore(ElementCurrent << 1 | 1, ((ElementStart + ElementStop) >> 1) + 1, ElementStop, ElementTargetStart, ElementTargetStop, ElementChange);
            ElementTree[ElementCurrent] = ElementTree[ElementCurrent << 1] + ElementTree[ElementCurrent << 1 | 1];
        }

        doDisableCopyAssignConstruct(SegmentTree)
    public:
        explicit SegmentTree(T ElementCountSource) : ElementCount(ElementCountSource) {
            if (!ElementCount) throw Exception(String(u"SegmentTree<T>::SegmentTree(T) ElementCount"));
            ElementList = MemoryAllocator::newArray<T>(ElementCount + 1);
            ElementMark = MemoryAllocator::newArray<T>(ElementCount << 2 | 1);
            ElementTree = MemoryAllocator::newArray<T>(ElementCount << 2 | 1);
            memset(ElementList + 1, 0, sizeof(T) * ElementCount);
            memset(ElementMark + 1, 0, sizeof(T) * (ElementCount << 2));
            memset(ElementTree + 1, 0, sizeof(T) * (ElementCount << 2));
        }

        ~SegmentTree() {
            delete[] ElementList;
            ElementList = nullptr;
            delete[] ElementMark;
            ElementMark = nullptr;
            delete[] ElementTree;
            ElementTree = nullptr;
        }

        void doBuild() noexcept {
            doBuildCore(1, 1, ElementCount);
        }

        T doQuery(T ElementStart, T ElementStop) noexcept {
            return doQueryCore(1, 1, ElementCount, ElementStart, ElementStop);
        }

        void doUpdate(T ElementStart, T ElementStop, T ElementDelta) noexcept {
            doUpdateCore(1, 1, ElementCount, ElementStart, ElementStop, ElementDelta);
        }

        void setElement(T ElementIndex, T ElementSource) {
            if (ElementIndex <= 0 || ElementIndex > ElementCount) throw IndexException(String(u"SegmentTree<T>::setElement(T, T) ElementIndex"));
            ElementList[ElementIndex] = ElementSource;
        }
    };

    template<typename E>
    class SingleLinkedIterator final : public Object {
    private:
        struct LinkedNode {
            E NodeValue;
            LinkedNode *NodeNext;
        } *NodeCurrent;
    public:
        typedef std::forward_iterator_tag iterator_category;

        constexpr SingleLinkedIterator(LinkedNode *NodeSource) noexcept : NodeCurrent(NodeSource) {}

        SingleLinkedIterator &operator++() {
            NodeCurrent = NodeCurrent->NodeNext;
            return *this;
        }

        E &operator*() noexcept {
            return NodeCurrent->NodeValue;
        }

        bool operator==(const SingleLinkedIterator &IteratorSource) noexcept {
            return NodeCurrent == IteratorSource.NodeCurrent;
        }

        bool operator!=(const SingleLinkedIterator &IteratorSource) noexcept {
            return NodeCurrent != IteratorSource.NodeCurrent;
        }
    };

    /**
     * Support for discrete storage of objects
     */
    template<typename E>
    class SingleLinkedList final : public List<E> {
    protected:
        struct LinkedNode final {
            E NodeValue;
            LinkedNode *NodeNext;

            constexpr explicit LinkedNode(const E &NodeValueSource) noexcept: NodeValue(NodeValueSource), NodeNext(nullptr) {}
        } *NodeHead = nullptr, *NodeTail = nullptr;

        intmax_t NodeSize = 0;
    public:
        doEnableCopyAssignConstruct(SingleLinkedList)

        constexpr SingleLinkedList() noexcept = default;

        SingleLinkedList(std::initializer_list<E> ElementList) noexcept : NodeSize(ElementList.size()) {
            for (const E &ElementCurrent : ElementList) addElement(ElementCurrent);
        }

        ~SingleLinkedList() noexcept {
            doClear();
        }

        void addElement(const E &ElementSource) {
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

        void addElement(intmax_t ElementIndex, const E &ElementSource) {
            if (ElementIndex < 0) ElementIndex += NodeSize + 1;
            if (ElementIndex < 0 || ElementIndex > NodeSize)
                throw IndexException(String(u"SingleLinkedList<E>::addElement(intmax_t, const E&) ElementIndex"));
            if (ElementIndex == NodeSize) {
                addElement(ElementSource);
                return;
            }
            LinkedNode *NodeCurrent = NodeHead;
            if (ElementIndex) while (--ElementIndex) NodeCurrent = NodeCurrent->NodeNext;
            auto *NodeTarget = MemoryAllocator::newObject<LinkedNode>(ElementSource);
            NodeTarget->NodeNext = NodeCurrent->NodeNext;
            NodeCurrent->NodeNext = NodeTarget;
            ++NodeSize;
        }

        void doAssign(const SingleLinkedList<E> &ElementSource) {
            if (Objects::getAddress(ElementSource) == this) return;
            if (!isEmpty()) doClear();
            NodeSize = ElementSource.NodeSize;
            LinkedNode *NodeCurrent = ElementSource.NodeHead;
            while (NodeCurrent) {
                addElement(NodeCurrent->NodeValue);
                NodeCurrent = NodeCurrent->NodeNext;
            }
        }

        void doClear() noexcept {
            NodeSize = 0;
            NodeTail = nullptr;
            LinkedNode *NodeCurrent = NodeHead, *NodePrevious;
            NodeHead = nullptr;
            while (NodeCurrent) {
                NodePrevious = NodeCurrent;
                NodeCurrent = NodeCurrent->NodeNext;
                delete NodePrevious;
            }
        }

        const E &getElement(intmax_t ElementIndex) const {
            if (ElementIndex < 0) ElementIndex += NodeSize;
            if (ElementIndex < 0 || ElementIndex >= NodeSize)
                throw IndexException(String(u"SingleLinkedList<E>::getElement(intmax_t) ElementIndex"));
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

        bool isEmpty() const noexcept override {
            return !NodeSize;
        }

        void removeElement(const E &ElementSource) {
            intmax_t ElementIndex = indexOf(ElementSource);
            if (ElementIndex == -1) throw IndexException(String(u"SingleLinkedList<E>::removeElement(const E&) ElementSource"));
            removeIndex(ElementIndex);
        }

        void removeIndex(intmax_t ElementIndex) {
            if (ElementIndex < 0) ElementIndex += NodeSize;
            if (ElementIndex < 0 || ElementIndex >= NodeSize)
                throw IndexException(String(u"SingleLinkedList<E>::removeIndex(intmax_t) ElementIndex"));
            LinkedNode *NodeCurrent = NodeHead;
            if (ElementIndex == 0) {
                NodeHead = NodeHead->NodeNext;
                delete NodeCurrent;
                --NodeSize;
                return;
            }
            bool NodeLast = false;
            if (ElementIndex == NodeSize - 1) NodeLast = true;
            while (--ElementIndex) NodeCurrent = NodeCurrent->NodeNext;
            LinkedNode *NodeNext = NodeCurrent->NodeNext->NodeNext;
            delete NodeCurrent->NodeNext;
            NodeCurrent->NodeNext = NodeNext;
            if (NodeLast) NodeTail = NodeCurrent;
            --NodeSize;
        }

        void setElement(intmax_t ElementIndex, const E &ElementSource) {
            if (ElementIndex < 0) ElementIndex += NodeSize;
            if (ElementIndex < 0 || ElementIndex >= NodeSize)
                throw IndexException(String(u"SingleLinkedList<E>::setElement(intmax_t, const E&) ElementIndex"));
            LinkedNode *NodeCurrent = NodeHead;
            while (ElementIndex--) NodeCurrent = NodeCurrent->NodeNext;
            NodeCurrent->NodeValue = ElementSource;
        }

        std::forward_list<E> toSTLForwardList() const {
            std::forward_list<E> ListResult;
            LinkedNode *NodeCurrent = NodeHead;
            while (NodeCurrent) {
                ListResult.insert_after(ListResult.end(), NodeCurrent->NodeValue);
                NodeCurrent = NodeCurrent->NodeNext;
            }
            return ListResult;
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
            return {NodeTail};
        }
    };

    template<typename E>
    class SingleLinkedQueue : public Queue<E> {
    protected:
        struct LinkedNode {
            E NodeValue;
            LinkedNode *NodeNext;

            explicit LinkedNode(const E &NodeValueSource) noexcept: NodeValue(NodeValueSource), NodeNext(nullptr) {}
        } *NodeHead = nullptr, *NodeTail = nullptr;

        intmax_t NodeSize = 0;
    public:
        doEnableCopyAssignConstruct(SingleLinkedQueue)

        constexpr SingleLinkedQueue() noexcept = default;

        ~SingleLinkedQueue() noexcept {
            doClear();
        }

        void addBack(const E &ElementSource) {
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

        void addFront(const E &ElementSource) {
            auto *NodeCurrent = MemoryAllocator::newObject<LinkedNode>(ElementSource);
            if (!NodeHead) {
                NodeHead = NodeCurrent;
                NodeTail = NodeCurrent;
            } else {
                NodeCurrent->NodeNext = NodeHead;
                NodeHead = NodeCurrent;
            }
            ++NodeSize;
        }

        void doAssign(const SingleLinkedQueue<E> &ElementSource) {
            if (Objects::getAddress(ElementSource) == this) return;
            if (!isEmpty()) doClear();
            NodeSize = ElementSource.NodeSize;
            LinkedNode *NodeCurrent = ElementSource.NodeHead;
            while (NodeCurrent) {
                addBack(NodeCurrent->NodeValue);
                NodeCurrent = NodeCurrent->NodeNext;
            }
        }

        void doClear() noexcept {
            NodeSize = 0;
            NodeTail = nullptr;
            LinkedNode *NodeCurrent = NodeHead, *NodePrevious;
            NodeHead = nullptr;
            while (NodeCurrent) {
                NodePrevious = NodeCurrent;
                NodeCurrent = NodeCurrent->NodeNext;
                delete NodePrevious;
            }
        }

        const E &getBack() const {
            if (isEmpty()) throw Exception(String(u"SingleLinkedQueue<E>::getBack() isEmpty"));
            return NodeTail->NodeValue;
        }

        const E &getFront() const {
            if (isEmpty()) throw Exception(String(u"SingleLinkedQueue<E>::getFront() isEmpty"));
            return NodeHead->NodeValue;
        }

        bool isEmpty() const noexcept override {
            return !NodeSize;
        }

        void removeBack() {
            if (isEmpty())
                throw Exception(String(u"SingleLinkedQueue<E>::removeBack() isEmpty"));
            LinkedNode *NodeCurrent = NodeHead;
            intmax_t ElementIndex = NodeSize - 1;
            while (--ElementIndex) NodeCurrent = NodeCurrent->NodeNext;
            LinkedNode *NodeNext = NodeCurrent->NodeNext->NodeNext;
            delete NodeCurrent->NodeNext;
            NodeCurrent->NodeNext = NodeNext;
            NodeTail = NodeCurrent;
            --NodeSize;
        }

        void removeFront() {
            if (isEmpty()) throw Exception(String(u"SingleLinkedQueue<E>::removeFront() isEmpty"));
            LinkedNode *NodeCurrent = NodeHead;
            NodeHead = NodeHead->NodeNext;
            delete NodeCurrent;
            --NodeSize;
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
    };

    /**
     * Support for discrete storage of unique objects
     */
    template<typename E>
    class SingleLinkedSet final : public Set<E> {
    private:
        struct LinkedNode final {
            E NodeValue;
            LinkedNode *NodeNext;

            constexpr explicit LinkedNode(const E &NodeValueSource) noexcept: NodeValue(NodeValueSource), NodeNext(nullptr) {}
        } *NodeHead = nullptr, *NodeTail = nullptr;

        intmax_t NodeSize = 0;
    public:
        doEnableCopyAssignConstruct(SingleLinkedSet)

        constexpr SingleLinkedSet() noexcept = default;

        ~SingleLinkedSet() noexcept {
            doClear();
        }

        void addElement(const E &ElementSource) {
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

        void doAssign(const SingleLinkedSet &ElementSource) {
            if (Objects::getAddress(ElementSource) == this) return;
            if (!isEmpty()) doClear();
            NodeSize = ElementSource.NodeSize;
            LinkedNode *NodeCurrent = ElementSource.NodeHead;
            while (NodeCurrent) {
                addElement(NodeCurrent->NodeValue);
                NodeCurrent = NodeCurrent->NodeNext;
            }
        }

        void doClear() noexcept {
            NodeSize = 0;
            NodeTail = nullptr;
            LinkedNode *NodeCurrent = NodeHead, *NodePrevious;
            NodeHead = nullptr;
            while (NodeCurrent) {
                NodePrevious = NodeCurrent;
                NodeCurrent = NodeCurrent->NodeNext;
                delete NodePrevious;
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

        bool isEmpty() const noexcept override {
            return !NodeSize;
        }

        void removeElement(const E &ElementSource) {
            intmax_t ElementIndex = 0;
            LinkedNode *NodeCurrent = NodeHead;
            while (NodeCurrent && Objects::doCompare(NodeCurrent->NodeValue, ElementSource)) NodeCurrent = NodeCurrent->NodeNext, ++ElementIndex;
            if (!NodeCurrent) throw IndexException(String(u"SingleLinkedSet<E>::removeElement(const E&) ElementSource"));
            NodeCurrent = NodeHead;
            if (ElementIndex == 0) {
                NodeHead = NodeHead->NodeNext;
                delete NodeCurrent;
                --NodeSize;
                return;
            }
            bool NodeLast = false;
            if (ElementIndex == NodeSize - 1) NodeLast = true;
            while (--ElementIndex) NodeCurrent = NodeCurrent->NodeNext;
            LinkedNode *NodeNext = NodeCurrent->NodeNext->NodeNext;
            delete NodeCurrent->NodeNext;
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
            return {NodeTail};
        }
    };

    template<Comparable K, typename V>
    class TreeMap final : public Map<K, V>, protected RedBlackTree<K, V> {
    public:
        const V &getElement(const K &MapKey) const {
            auto *MapNode = this->doSearchCore(this->NodeRoot, MapKey);
            if (!MapNode) throw Exception(String(u"TreeMap<K, V>::getElement(const K&) doSearchCore"));
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

        bool isEmpty() const noexcept override {
            return !this->NodeRoot;
        }

        void removeMapping(const K &MapKey) {
            if (!this->doSearchCore(this->NodeRoot, MapKey))
                throw Exception(String(u"TreeMap<K(Comparable),V>::removeMapping(const K&) MapKey"));
            this->doRemove(MapKey);
        }

        void setMapping(const K &MapKey, const V &MapValue) {
            if (this->doSearchCore(this->NodeRoot, MapKey))
                this->doSearchCore(this->NodeRoot, MapKey)->NodeValue = MapValue;
            else this->doInsert(MapKey, MapValue);
        }
    };

    template<Comparable E>
    class TreeSet final : public Set<E>, protected RedBlackTree<E, std::nullptr_t> {
    private:
        static void toString(const typename RedBlackTree<E, std::nullptr_t>::RedBlackNode *NodeCurrent, StringStream &CharacterStream) noexcept {
            if (!NodeCurrent) return;
            CharacterStream.addString(String::valueOf(NodeCurrent->NodeKey));
            CharacterStream.addCharacter(u' ');
            toString(NodeCurrent->NodeChildLeft, CharacterStream);
            toString(NodeCurrent->NodeChildRight, CharacterStream);
        }
    public:
        void addElement(const E &ElementSource) noexcept {
            this->doInsert(ElementSource, std::nullptr_t());
        }

        TreeSet doDifference(const TreeSet &SetSource) const noexcept {
            TreeSet SetResult;
            this->doOrderCore(this->NodeRoot, [&](const E &ElementSource, std::nullptr_t){
                if (!SetSource.isContains(ElementSource)) SetResult.addElement(ElementSource);
            });
            return SetResult;
        }

        TreeSet doIntersection(const TreeSet &SetSource) const noexcept {
            TreeSet SetResult;
            this->doOrderCore(this->NodeRoot, [&](const E &ElementSource, std::nullptr_t){
                if (SetSource.isContains(ElementSource)) SetResult.addElement(ElementSource);
            });
            return SetResult;
        }

        TreeSet doUnion(const TreeSet &SetSource) const noexcept {
            TreeSet SetResult;
            this->doOrderCore(this->NodeRoot, [&](const E &ElementSource, std::nullptr_t){
                SetResult.addElement(ElementSource);
            });
            SetSource.doOrderCore(SetSource.NodeRoot, [&](const E &ElementSource, std::nullptr_t){
                SetResult.addElement(ElementSource);
            });
            return SetResult;
        }

        uintmax_t getElementSize() const noexcept {
            return this->getSize();
        }

        bool isContains(const E &ElementSource) const noexcept {
            return this->doSearchCore(this->NodeRoot, ElementSource);
        }

        bool isEmpty() const noexcept override {
            return !this->NodeRoot;
        }

        void removeElement(const E &ElementSource) {
            if (!isContains(ElementSource)) throw IndexException(String(u"TreeSet<E>::removeElement(const E&) isContains"));
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
}
