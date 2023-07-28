#pragma once

#include <Core/Memory.hpp>

#include <array>
#include <forward_list>
#include <list>
#include <optional>
#include <vector>

namespace eLibrary::Core {
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
            if (&typeid(T) != AnyType) throw Exception(String(u"Any::getValue<T>()"));
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

    template<typename E>
    class ArrayIterator final : public Object {
    private:
        E *ElementCurrent;
    public:
        typedef intmax_t difference_type;

        constexpr ArrayIterator() noexcept : ElementCurrent(nullptr) {}

        constexpr ArrayIterator(E *ElementSource) noexcept : ElementCurrent(ElementSource) {}

        ArrayIterator<E> operator+(intmax_t ElementIndex) noexcept {
            return {ElementCurrent + ElementIndex};
        }

        ArrayIterator<E> &operator++() noexcept {
            ++ElementCurrent;
            return *this;
        }

        ArrayIterator<E> operator++(int) noexcept {
            return {ElementCurrent++};
        }

        ArrayIterator<E> &operator-(intmax_t ElementIndex) noexcept {
            return {ElementCurrent - ElementIndex};
        }

        ArrayIterator<E> &operator--() noexcept {
            --ElementCurrent;
            return *this;
        }

        ArrayIterator<E> operator--(int) noexcept {
            return {ElementCurrent--};
        }

        E operator*() const noexcept {
            return *ElementCurrent;
        }

        bool operator==(const ArrayIterator<E> &IteratorSource) const noexcept {
            return ElementCurrent == IteratorSource.ElementCurrent;
        }

        bool operator!=(const ArrayIterator<E> &IteratorSource) const noexcept {
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
    public:
        doEnableCopyAssignConstruct(Array)

        constexpr Array() noexcept = default;

        Array(intmax_t ElementSizeSource) : ElementSize(ElementSizeSource) {
            ElementContainer = MemoryAllocator::newArray<E>(ElementSize);
        }

        ~Array() noexcept {
            ElementSize = 0;
            delete[] ElementContainer;
            ElementContainer = nullptr;
        }

        void doAssign(const Array<E> &ElementSource) {
            if (Objects::getAddress(ElementSource) == this) return;
            delete[] ElementContainer;
            ElementContainer = MemoryAllocator::newArray<E>(ElementSize = ElementSource.ElementSize);
            std::copy_n(ElementSource.ElementContainer, ElementSize, ElementContainer);
        }

        E getElement(intmax_t ElementIndex) const {
            if (ElementIndex < 0) ElementIndex += ElementSize;
            if (ElementIndex < 0 || ElementIndex >= ElementSize) throw Exception(String(u"Array::getElement(intmax_t) ElementIndex"));
            return ElementContainer[ElementIndex];
        }

        intmax_t getElementSize() const noexcept {
            return ElementSize;
        }

        bool isEmpty() const noexcept {
            return !ElementSize;
        }
    };

    /**
     * Support for operating dynamic arrays
     */
    template<typename E>
    class ArrayList final : public Object {
    private:
        intmax_t ElementCapacity = 0, ElementSize = 0;
        E *ElementContainer = nullptr;

        ArrayList(E *ElementSource, intmax_t ElementSourceSize) : ElementCapacity(1), ElementSize(ElementSourceSize) {
            while (ElementCapacity < ElementSourceSize) ElementCapacity <<= 1;
            ElementContainer = MemoryAllocator::newArray<E>(ElementCapacity);
            std::copy(ElementSource, ElementSource + ElementSourceSize, ElementContainer);
        }

        void doInitialize() noexcept {}

        template<typename ...ElementListType>
        void doInitialize(E ElementCurrent, ElementListType ...ElementList) noexcept {
            ElementContainer[ElementSize++] = ElementCurrent;
            doInitialize(ElementList...);
        }
    public:
        doEnableCopyAssignConstruct(ArrayList)

        constexpr ArrayList() noexcept = default;

        template<size_t ElementSourceSize>
        ArrayList(const std::array<E, ElementSourceSize> &ElementSource) : ElementCapacity(1), ElementSize(ElementSourceSize) {
            while (ElementCapacity < ElementSourceSize) ElementCapacity <<= 1;
            ElementContainer = MemoryAllocator::newArray<E>(ElementCapacity);
            std::copy(ElementSource.begin(), ElementSource.end(), ElementContainer);
        }

        template<typename ...ElementListType>
        explicit ArrayList(ElementListType ...ElementList) : ElementCapacity(1), ElementSize(0) {
            while (ElementCapacity < sizeof...(ElementList))
                ElementCapacity <<= 1;
            ElementContainer = MemoryAllocator::newArray<E>(ElementCapacity);
            doInitialize(ElementList...);
        }

        ArrayList(std::initializer_list<E> ElementList) : ElementCapacity(1), ElementSize((intmax_t) ElementList.size()) {
            while (ElementCapacity < (intmax_t) ElementList.size())
                ElementCapacity <<= 1;
            ElementContainer = MemoryAllocator::newArray<E>(ElementCapacity);
            std::copy(ElementList.begin(), ElementList.end(), ElementContainer);
        }

        ~ArrayList() noexcept {
            doClear();
        }

        void addElement(const E &ElementSource) {
            if (!ElementCapacity) ElementContainer = MemoryAllocator::newArray<E>(ElementCapacity = 1);
            if (ElementSize == ElementCapacity) {
                auto *ElementBuffer = MemoryAllocator::newArray<E>(ElementSize);
                std::copy_n(ElementContainer, ElementSize, ElementBuffer);
                delete[] ElementContainer;
                ElementContainer = MemoryAllocator::newArray<E>(ElementCapacity <<= 1);
                std::copy_n(ElementBuffer, ElementSize, ElementContainer);
                delete[] ElementBuffer;
            }
            ElementContainer[ElementSize++] = ElementSource;
        }

        void addElement(intmax_t ElementIndex, const E &ElementSource) {
            if (ElementIndex < 0) ElementIndex += ElementSize + 1;
            if (ElementIndex < 0 || ElementIndex > ElementSize)
                throw Exception(String(u"ArrayList<E>::addElement(intmax_t, const E&) ElementIndex"));
            if (!ElementCapacity) ElementContainer = MemoryAllocator::newArray<E>(ElementCapacity = 1);
            if (ElementSize == ElementCapacity) {
                auto *ElementBuffer = MemoryAllocator::newArray<E>(ElementSize);
                std::copy(ElementContainer, ElementContainer + ElementSize, ElementBuffer);
                delete[] ElementContainer;
                ElementContainer = MemoryAllocator::newArray<E>(ElementCapacity <<= 1);
                std::copy(ElementBuffer, ElementBuffer + ElementSize, ElementContainer);
                delete[] ElementBuffer;
            }
            std::copy(ElementContainer + ElementIndex, ElementContainer + ElementSize, ElementContainer + ElementIndex + 1);
            ElementContainer[ElementIndex] = ElementSource;
            ++ElementSize;
        }

        void doAssign(const ArrayList<E> &ElementSource) {
            if (Objects::getAddress(ElementSource) == this) return;
            delete[] ElementContainer;
            ElementContainer = MemoryAllocator::newArray<E>(ElementCapacity = ElementSource.ElementCapacity);
            std::copy_n(ElementSource.ElementContainer, (ElementSize = ElementSource.ElementSize), ElementContainer);
        }

        void doClear() noexcept {
            ElementCapacity = 0;
            ElementSize = 0;
            delete[] ElementContainer;
            ElementContainer = nullptr;
        }

        ArrayList<E> doConcat(const ArrayList<E> &ElementSource) const {
            E *ElementBuffer = MemoryAllocator::newArray<E>(ElementSize + ElementSource.ElementSize);
            std::copy(ElementContainer, ElementContainer + ElementSize, ElementBuffer);
            std::copy(ElementSource.ElementContainer, ElementSource.ElementContainer + ElementSource.ElementSize, ElementBuffer + ElementSize);
            return {ElementBuffer, ElementSize + ElementSource.ElementSize};
        }

        void doReverse() {
            E *ElementBuffer = MemoryAllocator::newArray<E>(ElementSize);
            std::copy_n(ElementContainer, ElementSize, ElementBuffer);
            std::reverse_copy(ElementBuffer, ElementBuffer + ElementSize, ElementContainer);
            delete[] ElementBuffer;
        }

        const E &getElement(intmax_t ElementIndex) const {
            if (ElementIndex < 0) ElementIndex += ElementSize;
            if (ElementIndex < 0 || ElementIndex >= ElementSize)
                throw Exception(String(u"ArrayList<E>::getElement(intmax_t) ElementIndex"));
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
            if (ElementIndex == -1) throw Exception(String(u"ArrayList<E>::removeElement(const E&) indexOf"));
            removeIndex(ElementIndex);
        }

        void removeIndex(intmax_t ElementIndex) {
            if (ElementIndex < 0) ElementIndex += ElementSize;
            if (ElementIndex < 0 || ElementIndex >= ElementSize)
                throw Exception(String(u"ArrayList<E>::removeIndex(intmax_t) ElementIndex"));
            std::copy(ElementContainer + ElementIndex + 1, ElementContainer + ElementSize, ElementContainer + ElementIndex);
            if (ElementCapacity == 1) doClear();
            else if (--ElementSize <= ElementCapacity >> 1) {
                E *ElementBuffer = MemoryAllocator::newArray<E>(ElementSize);
                std::copy_n(ElementContainer, ElementSize, ElementBuffer);
                delete[] ElementContainer;
                ElementContainer = MemoryAllocator::newArray<E>(ElementCapacity >>= 1);
                std::copy_n(ElementBuffer, ElementSize, ElementContainer);
                delete[] ElementBuffer;
            }
        }

        void setElement(intmax_t ElementIndex, const E &ElementSource) {
            if (ElementIndex < 0) ElementIndex += ElementSize;
            if (ElementIndex < 0 || ElementIndex >= ElementSize)
                throw Exception(String(u"ArrayList<E>::setElement(intmax_t, const E&) ElementIndex"));
            ElementContainer[ElementIndex] = ElementSource;
        }

        auto toSTLArray() const noexcept {
            std::array<E, this->ElementSize> ArraySource;
            for (intmax_t ElementIndex = 0;ElementIndex < ElementSize;++ElementIndex)
                ArraySource[ElementIndex] = ElementContainer[ElementIndex];
            return ArraySource;
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
    class ArraySet final : public Object {
    private:
        intmax_t ElementCapacity = 0, ElementSize = 0;
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
                auto *ElementBuffer = MemoryAllocator::newArray<E>(ElementSize);
                std::copy(ElementContainer, ElementContainer + ElementSize, ElementBuffer);
                delete[] ElementContainer;
                ElementContainer = MemoryAllocator::newArray<E>(ElementCapacity <<= 1);
                std::copy(ElementBuffer, ElementBuffer + ElementSize, ElementContainer);
                delete[] ElementBuffer;
            }
            ElementContainer[ElementSize++] = ElementSource;
        }

        void doAssign(const ArraySet<E> &SetSource) {
            if (Objects::getAddress(SetSource) == this) return;
            delete[] ElementContainer;
            ElementCapacity = SetSource.ElementCapacity;
            if (ElementCapacity) ElementContainer = MemoryAllocator::newArray<E>(ElementCapacity);
            ElementSize = SetSource.ElementSize;
            std::copy(SetSource.ElementContainer, SetSource.ElementContainer + ElementSize, ElementContainer);
        }

        void doClear() noexcept {
            ElementCapacity = 0;
            ElementSize = 0;
            delete[] ElementContainer;
            ElementContainer = nullptr;
        }

        ArraySet<E> doDifference(const ArraySet<E> &SetSource) const noexcept {
            ArraySet<E> SetResult;
            for (intmax_t ElementIndex = 0; ElementIndex < ElementSize; ++ElementIndex)
                if (!SetSource.isContains(ElementContainer[ElementIndex])) SetResult.addElement(ElementContainer[ElementIndex]);
            return SetResult;
        }

        ArraySet<E> doIntersection(const ArraySet<E> &SetSource) const noexcept {
            ArraySet<E> SetResult;
            for (intmax_t ElementIndex = 0; ElementIndex < ElementSize; ++ElementIndex)
                if (SetSource.isContains(ElementContainer[ElementIndex])) SetResult.addElement(ElementContainer[ElementIndex]);
            return SetResult;
        }

        ArraySet<E> doUnion(const ArraySet<E> &SetSource) const noexcept {
            ArraySet<E> SetResult(*this);
            for (intmax_t ElementIndex = 0; ElementIndex < SetSource.ElementSize; ++ElementIndex)
                SetResult.addElement(SetSource.ElementContainer[ElementIndex]);
            return SetResult;
        }

        intmax_t getElementSize() const noexcept {
            return ElementSize;
        }

        bool isContains(const E &ElementSource) const noexcept {
            for (intmax_t ElementIndex = 0; ElementIndex < ElementSize; ++ElementIndex)
                if (!Objects::doCompare(ElementContainer[ElementIndex], ElementSource)) return true;
            return false;
        }

        bool isEmpty() const noexcept {
            return ElementSize == 0;
        }

        void removeElement(const E &ElementSource) {
            intmax_t ElementIndex = 0;
            for (; ElementIndex < ElementSize; ++ElementIndex)
                if (!Objects::doCompare(ElementContainer[ElementIndex], ElementSource)) break;
            if (ElementIndex == ElementSize) throw Exception(String(u"ArraySet::removeElement(const E&) ElementSource"));
            std::copy(ElementContainer + ElementIndex + 1, ElementContainer + ElementSize, ElementContainer + ElementIndex);
            if (ElementCapacity == 1) doClear();
            else if (--ElementSize <= ElementCapacity >> 1) {
                auto *ElementBuffer = MemoryAllocator::newArray<E>(ElementSize);
                std::copy(ElementContainer, ElementContainer + ElementSize, ElementBuffer);
                delete[] ElementContainer;
                ElementContainer = MemoryAllocator::newArray<E>(ElementCapacity >>= 1);
                std::copy(ElementBuffer, ElementBuffer + ElementSize, ElementContainer);
                delete[] ElementBuffer;
            }
        }

        ArrayList<E> toArrayList() const noexcept {
            ArrayList<E> ListResult;
            for (intmax_t ElementIndex = 0; ElementIndex < ElementSize;++ElementIndex)
                ListResult.addElement(ElementContainer[ElementIndex]);
            return ListResult;
        }

        String toString() const noexcept override {
            StringStream CharacterStream;
            CharacterStream.addCharacter(u'{');
            for (intmax_t ElementIndex = 0; ElementIndex + 1 < ElementSize; ++ElementIndex) {
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

    /**
     * Support for discrete storage of objects
     */
    template<typename E>
    class DoubleLinkedList final : public Object {
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
                throw Exception(String(u"DoubleLinkedList<E>::addElement(intmax_t, const E&) ElementIndex"));
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
                std::swap(NodeHead, NodeTail);
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
                throw Exception(String(u"DoubleLinkedList<E>::getElement(intmax_t) ElementIndex"));
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
            if (ElementIndex == -1) throw Exception(String(u"DoubleLinkedList<E>::removeElement(const E&) ElementSource"));
            removeIndex(ElementIndex);
        }

        void removeIndex(intmax_t ElementIndex) {
            if (ElementIndex < 0) ElementIndex += NodeSize;
            if (ElementIndex < 0 || ElementIndex >= NodeSize)
                throw Exception(String(u"DoubleLinkedList<E>::removeElement(intmax_t) ElementIndex"));
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
                throw Exception(String(u"DoubleLinkedList<E>::setElement(intmax_t, const E&) ElementIndex"));
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

        ArrayList<E> toArrayList() const noexcept {
            ArrayList<E> ListResult;
            LinkedNode *NodeCurrent = NodeHead;
            while (NodeCurrent) {
                ListResult.addElement(NodeCurrent->NodeValue);
                NodeCurrent = NodeCurrent->NodeNext;
            }
            return ListResult;
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
    class DoubleLinkedSet final : public Object {
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

        void doAssign(const DoubleLinkedSet<E> &ElementSource) {
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

        DoubleLinkedSet<E> doDifference(const DoubleLinkedSet<E> &SetSource) const noexcept {
            DoubleLinkedSet<E> SetResult;
            LinkedNode *NodeCurrent = NodeHead;
            while (NodeCurrent) {
                if (!SetSource.isContains(NodeCurrent->NodeValue))
                    SetResult.addElement(NodeCurrent->NodeValue);
                NodeCurrent = NodeCurrent->NodeNext;
            }
            return SetResult;
        }

        DoubleLinkedSet<E> doIntersection(const DoubleLinkedSet<E> &SetSource) const noexcept {
            DoubleLinkedSet<E> SetResult;
            LinkedNode *NodeCurrent = NodeHead;
            while (NodeCurrent) {
                if (SetSource.isContains(NodeCurrent->NodeValue))
                    SetResult.addElement(NodeCurrent->NodeValue);
                NodeCurrent = NodeCurrent->NodeNext;
            }
            return SetResult;
        }

        DoubleLinkedSet<E> doUnion(const DoubleLinkedSet<E> &SetSource) const noexcept {
            DoubleLinkedSet<E> SetResult(*this);
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

        bool isEmpty() const noexcept {
            return !NodeSize;
        }

        void removeElement(const E &ElementSource) {
            intmax_t ElementIndex = 0;
            LinkedNode *NodeCurrent = NodeHead;
            while (NodeCurrent && Objects::doCompare(NodeCurrent->NodeValue, ElementSource)) NodeCurrent = NodeCurrent->NodeNext, ++ElementIndex;
            if (!NodeCurrent) throw Exception(String(u"DoubleLinkedSet<E>::removeElement(const E&) ElementSource"));
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
        void doOrderCore(RedBlackNode *NodeCurrent, OperationType Operation) const noexcept(std::is_nothrow_invocable<OperationType, K, V>::value) {
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

        RedBlackNode *doSearchCore(RedBlackNode *NodeCurrent, const K &NodeKey) const noexcept {
            if (!NodeCurrent) return nullptr;
            auto NodeRelation = Objects::doCompare(NodeCurrent->NodeKey, NodeKey);
            if (NodeRelation == 0) return NodeCurrent;
            else if (NodeRelation < 0) return doSearchCore(NodeCurrent->NodeChildRight, NodeKey);
            else return doSearchCore(NodeCurrent->NodeChildLeft, NodeKey);
        }

        uintmax_t getHeightCore(RedBlackNode *NodeCurrent) const noexcept {
            if (NodeCurrent->NodeChildLeft && NodeCurrent->NodeChildRight)
                return Objects::getMaximum(getHeightCore(NodeCurrent->NodeChildLeft), getHeightCore(NodeCurrent->NodeChildRight)) + 1;
            if (NodeCurrent->NodeChildLeft) return getHeightCore(NodeCurrent->NodeChildLeft) + 1;
            if (NodeCurrent->NodeChildRight) return getHeightCore(NodeCurrent->NodeChildRight) + 1;
            return 1;
        }

        uintmax_t getSizeCore(RedBlackNode *NodeCurrent) const noexcept {
            if (!NodeCurrent) return 0;
            return getSizeCore(NodeCurrent->NodeChildLeft) + getSizeCore(NodeCurrent->NodeChildRight) + 1;
        }
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
                        std::swap(NodeTarget, NodeParent);
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
                        std::swap(NodeTarget, NodeParent);
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
            if (!ElementList || !ElementMark || !ElementTree) throw Exception(String(u"SegmentTree<T>::SegmentTree(T) new"));
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
            if (ElementIndex <= 0 || ElementIndex > ElementCount) throw Exception(String(u"SegmentTree<T>::setElement(T, T) ElementIndex"));
            ElementList[ElementIndex] = ElementSource;
        }
    };

    /**
     * Support for discrete storage of objects
     */
    template<typename E>
    class SingleLinkedList final : public Object {
    private:
        struct LinkedNode final {
            E NodeValue;
            LinkedNode *NodeNext;

            explicit LinkedNode(const E &NodeValueSource) noexcept: NodeValue(NodeValueSource), NodeNext(nullptr) {}
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
                throw Exception(String(u"SingleLinkedList<E>::addElement(intmax_t, const E&) ElementIndex"));
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
                throw Exception(String(u"SingleLinkedList<E>::getElement(intmax_t) ElementIndex"));
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
            if (ElementIndex == -1) throw Exception(String(u"SingleLinkedList<E>::removeElement(const E&) ElementSource"));
            removeIndex(ElementIndex);
        }

        void removeIndex(intmax_t ElementIndex) {
            if (ElementIndex < 0) ElementIndex += NodeSize;
            if (ElementIndex < 0 || ElementIndex >= NodeSize)
                throw Exception(String(u"SingleLinkedList<E>::removeElement(intmax_t) ElementIndex"));
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
                throw Exception(String(u"SingleLinkedList<E>::setElement(intmax_t, const E&) ElementIndex"));
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
    };

    /**
     * Support for discrete storage of unique objects
     */
    template<typename E>
    class SingleLinkedSet final : public Object {
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

        void doAssign(const SingleLinkedSet<E> &ElementSource) {
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

        SingleLinkedSet<E> doDifference(const SingleLinkedSet<E> &SetSource) const noexcept {
            SingleLinkedSet<E> SetResult;
            LinkedNode *NodeCurrent = NodeHead;
            while (NodeCurrent) {
                if (!SetSource.isContains(NodeCurrent->NodeValue))
                    SetResult.addElement(NodeCurrent->NodeValue);
                NodeCurrent = NodeCurrent->NodeNext;
            }
            return SetResult;
        }

        SingleLinkedSet<E> doIntersection(const SingleLinkedSet<E> &SetSource) const noexcept {
            SingleLinkedSet<E> SetResult;
            LinkedNode *NodeCurrent = NodeHead;
            while (NodeCurrent) {
                if (SetSource.isContains(NodeCurrent->NodeValue))
                    SetResult.addElement(NodeCurrent->NodeValue);
                NodeCurrent = NodeCurrent->NodeNext;
            }
            return SetResult;
        }

        SingleLinkedSet<E> doUnion(const SingleLinkedSet<E> &SetSource) const noexcept {
            SingleLinkedSet<E> SetResult(*this);
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

        bool isEmpty() const noexcept {
            return !NodeSize;
        }

        void removeElement(const E &ElementSource) {
            intmax_t ElementIndex = 0;
            LinkedNode *NodeCurrent = NodeHead;
            while (NodeCurrent && Objects::doCompare(NodeCurrent->NodeValue, ElementSource)) NodeCurrent = NodeCurrent->NodeNext, ++ElementIndex;
            if (!NodeCurrent) throw Exception(String(u"SingleLinkedSet<E>::removeElement(const E&) ElementSource"));
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
    };

    template<Comparable K, typename V>
    class TreeMap final : protected RedBlackTree<K, V> {
    public:
        const V &getElement(const K &MapKey) const {
            auto *MapNode = this->doSearchCore(this->NodeRoot, MapKey);
            if (!MapNode) throw Exception(String(u"TreeMap<K, V>::getElement(const K&) doSearchCore"));
            return MapNode->NodeValue;
        }

        uintmax_t getElementSize() const noexcept {
            return this->getSize();
        }

        bool isContains(const K &MapKey) const noexcept {
            return this->doSearchCore(this->NodeRoot, MapKey);
        }

        bool isEmpty() const noexcept {
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
    class TreeSet final : protected RedBlackTree<E, std::nullptr_t> {
    private:
        void toString(const RedBlackTree<E, std::nullptr_t>::RedBlackNode *NodeCurrent, StringStream &CharacterStream) const noexcept {
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

        TreeSet<E> doDifference(const TreeSet<E> &SetSource) const noexcept {
            TreeSet<E> SetResult;
            this->doOrderCore(this->NodeRoot, [&](const E &ElementSource, std::nullptr_t){
                if (!SetSource.isContains(ElementSource)) SetResult.addElement(ElementSource);
            });
            return SetResult;
        }

        TreeSet<E> doIntersection(const TreeSet<E> &SetSource) const noexcept {
            TreeSet<E> SetResult;
            this->doOrderCore(this->NodeRoot, [&](const E &ElementSource, std::nullptr_t){
                if (SetSource.isContains(ElementSource)) SetResult.addElement(ElementSource);
            });
            return SetResult;
        }

        TreeSet<E> doUnion(const TreeSet<E> &SetSource) const noexcept {
            TreeSet<E> SetResult;
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

        bool isEmpty() const noexcept {
            return !this->NodeRoot;
        }

        void removeElement(const E &ElementSource) {
            if (!isContains(ElementSource)) throw Exception(String(u"TreeSet<E>::removeElement(const E&) isContains"));
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
