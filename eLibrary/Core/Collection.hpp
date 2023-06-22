#pragma once

#include <Core/Exception.hpp>

#include <array>
#include <forward_list>
#include <list>
#include <vector>

namespace eLibrary::Core {
    /**
     * The ArrayList class provides support for continuous storage of objects
     */
    template<typename E>
    class ArrayList final : public Object {
    private:
        intmax_t ElementCapacity, ElementSize;
        E *ElementContainer;

        ArrayList(E *ElementSource, uintmax_t ElementSourceSize) noexcept : ElementCapacity(1), ElementSize(ElementSourceSize) {
            while (ElementCapacity < ElementSourceSize) ElementCapacity <<= 1;
            ElementContainer = new E[ElementCapacity];
            std::copy(ElementSource, ElementSource + ElementSourceSize, ElementContainer);
        }

        void doInitialize() noexcept {}

        template<typename ...ElementListType>
        void doInitialize(E ElementCurrent, ElementListType ...ElementList) noexcept {
            ElementContainer[ElementSize++] = ElementCurrent;
            doInitialize(ElementList...);
        }
    public:
        constexpr ArrayList() noexcept: ElementCapacity(0), ElementSize(0), ElementContainer(nullptr) {}

        template<size_t ElementSourceSize>
        ArrayList(const std::array<E, ElementSourceSize> &ElementSource) noexcept : ElementCapacity(1), ElementSize(ElementSourceSize) {
            if (ElementSourceSize) while (ElementCapacity < ElementSourceSize) ElementCapacity <<= 1;
            ElementContainer = new E[ElementCapacity];
            std::copy(ElementSource.begin(), ElementSource.end(), ElementContainer);
        }

        template<typename ...ElementListType>
        explicit ArrayList(ElementListType ...ElementList) noexcept : ElementCapacity(1), ElementSize(0) {
            while (ElementCapacity < sizeof...(ElementList))
                ElementCapacity <<= 1;
            ElementContainer = new E[ElementCapacity];
            doInitialize(ElementList...);
        }

        ArrayList(std::initializer_list<E> ElementList) noexcept : ElementCapacity(1), ElementSize((intmax_t) ElementList.size()) {
            while (ElementCapacity < (intmax_t) ElementList.size())
                ElementCapacity <<= 1;
            ElementContainer = new E[ElementCapacity];
            std::copy(ElementList.begin(), ElementList.end(), ElementContainer);
        }

        ~ArrayList() noexcept {
            doClear();
        }

        void addElement(const E &ElementSource) noexcept {
            if (ElementCapacity == 0) ElementContainer = new E[ElementCapacity = 1];
            if (ElementSize == ElementCapacity) {
                auto *ElementBuffer = new E[ElementSize];
                std::copy(ElementContainer, ElementContainer + ElementSize, ElementBuffer);
                delete[] ElementContainer;
                ElementContainer = new E[ElementCapacity <<= 1];
                std::copy(ElementBuffer, ElementBuffer + ElementSize, ElementContainer);
                delete[] ElementBuffer;
            }
            ElementContainer[ElementSize++] = ElementSource;
        }

        void addElement(intmax_t ElementIndex, const E &ElementSource) {
            if (ElementIndex < 0) ElementIndex += ElementSize + 1;
            if (ElementIndex < 0 || ElementIndex > ElementSize)
                throw Exception(String(u"ArrayList<E>::addElement(intmax_t, const E&) ElementIndex"));
            if (ElementCapacity == 0) ElementContainer = new E[ElementCapacity = 1];
            if (ElementSize == ElementCapacity) {
                auto *ElementBuffer = new E[ElementSize];
                std::copy(ElementContainer, ElementContainer + ElementSize, ElementBuffer);
                delete[] ElementContainer;
                ElementContainer = new E[ElementCapacity <<= 1];
                std::copy(ElementBuffer, ElementBuffer + ElementSize, ElementContainer);
                delete[] ElementBuffer;
            }
            std::copy(ElementContainer + ElementIndex, ElementContainer + ElementSize, ElementContainer + ElementIndex + 1);
            ElementContainer[ElementIndex] = ElementSource;
            ++ElementSize;
        }

        void doAssign(const ArrayList<E> &ElementSource) noexcept {
            if (&ElementSource == this) return;
            delete[] ElementContainer;
            ElementContainer = new E[ElementCapacity = ElementSource.ElementCapacity];
            ElementSize = ElementSource.ElementSize;
            std::copy(ElementSource.ElementContainer, ElementSource.ElementContainer + ElementSource.ElementSize, ElementContainer);
        }

        void doClear() noexcept {
            if (ElementCapacity && ElementContainer) {
                ElementCapacity = 0;
                ElementSize = 0;
                delete[] ElementContainer;
                ElementContainer = nullptr;
            }
        }

        ArrayList<E> doConcat(const ArrayList<E> &ElementSource) const noexcept {
            E *ElementBuffer = new E[ElementSize + ElementSource.ElementSize];
            std::copy(ElementContainer, ElementContainer + ElementSize, ElementBuffer);
            std::copy(ElementSource.ElementContainer, ElementSource.ElementContainer + ElementSource.ElementSize, ElementBuffer + ElementSize);
            return {ElementBuffer, ElementSize + ElementSource.ElementSize};
        }

        void doReverse() noexcept {
            E *ElementBuffer = new E[ElementSize];
            std::copy(ElementBuffer, ElementBuffer + ElementSize, ElementContainer);
            delete[] ElementBuffer;
        }

        E &getElement(intmax_t ElementIndex) const {
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
                if (ElementContainer[ElementIndex].doCompare(ElementSource) == 0) return ElementIndex;
            return -1;
        }

        bool isContains(const E &ElementSource) const noexcept {
            for (intmax_t ElementIndex = 0; ElementIndex < ElementSize; ++ElementIndex)
                if (ElementContainer[ElementIndex].doCompare(ElementSource) == 0) return true;
            return false;
        }

        bool isEmpty() const noexcept {
            return ElementSize == 0;
        }

        ArrayList<E> &operator=(const ArrayList<E> &ElementSource) noexcept {
            doAssign(ElementSource);
            return *this;
        }

        void removeElement(const E &ElementSource) {
            intmax_t ElementIndex = indexOf(ElementSource);
            if (ElementIndex == -1) throw Exception(String(u"ArrayList<E>::removeElement(const E&) ElementSource"));
            removeIndex(ElementIndex);
        }

        void removeIndex(intmax_t ElementIndex) {
            if (ElementIndex < 0) ElementIndex += ElementSize;
            if (ElementIndex < 0 || ElementIndex >= ElementSize)
                throw Exception(String(u"ArrayList<E>::removeElement(intmax_t) ElementIndex"));
            std::copy(ElementContainer + ElementIndex + 1, ElementContainer + ElementSize, ElementContainer + ElementIndex);
            if (ElementCapacity == 1) doClear();
            else if (--ElementSize <= ElementCapacity >> 1) {
                auto *ElementBuffer = new E[ElementSize];
                std::copy(ElementContainer, ElementContainer + ElementSize, ElementBuffer);
                delete[] ElementContainer;
                ElementContainer = new E[ElementCapacity >>= 1];
                std::copy(ElementBuffer, ElementBuffer + ElementSize, ElementContainer);
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
    };

    /**
     * The ArraySet class provides support for continuous storage of unique objects
     */
    template<typename E>
    class ArraySet final : public Object {
    private:
        intmax_t ElementCapacity, ElementSize;
        E *ElementContainer;
    public:
        constexpr ArraySet() noexcept : ElementCapacity(0), ElementSize(0), ElementContainer(nullptr) {}

        ArraySet(const ArrayList<E> &ElementSource) noexcept : ElementCapacity(0), ElementSize(0), ElementContainer(nullptr) {
            for (intmax_t ElementIndex = 0;ElementIndex < ElementSource.getElementSize();++ElementIndex)
                addElement(ElementSource.getElement(ElementIndex));
        }

        ArraySet(const ArraySet<E> &SetSource) noexcept : ElementCapacity(SetSource.ElementCapacity), ElementSize(SetSource.ElementSize) {
            if (ElementCapacity) ElementContainer = new E[ElementCapacity];
            std::copy(SetSource.ElementContainer, SetSource.ElementContainer + ElementSize, ElementContainer);
        }

        ~ArraySet() noexcept {
            if (ElementCapacity && ElementContainer) {
                ElementCapacity = 0;
                ElementSize = 0;
                delete[] ElementContainer;
                ElementContainer = nullptr;
            }
        }

        void addElement(const E &ElementSource) noexcept {
            if (isContains(ElementSource)) return;
            if (ElementCapacity == 0) ElementContainer = new E[ElementCapacity = 1];
            if (ElementSize == ElementCapacity) {
                auto *ElementBuffer = new E[ElementSize];
                std::copy(ElementContainer, ElementContainer + ElementSize, ElementBuffer);
                delete[] ElementContainer;
                ElementContainer = new E[ElementCapacity <<= 1];
                std::copy(ElementBuffer, ElementBuffer + ElementSize, ElementContainer);
                delete[] ElementBuffer;
            }
            ElementContainer[ElementSize++] = ElementSource;
        }

        void doClear() noexcept {
            if (ElementCapacity && ElementContainer) {
                ElementCapacity = 0;
                ElementSize = 0;
                delete[] ElementContainer;
                ElementContainer = nullptr;
            }
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
                if (ElementContainer[ElementIndex].doCompare(ElementSource) == 0) return true;
            return false;
        }

        bool isEmpty() const noexcept {
            return ElementSize == 0;
        }

        ArraySet<E> &operator=(const ArraySet<E> &SetSource) noexcept = delete;

        void removeElement(const E &ElementSource) {
            intmax_t ElementIndex = 0;
            for (; ElementIndex < ElementSize; ++ElementIndex)
                if (ElementContainer[ElementIndex] == ElementSource) break;
            if (ElementIndex == ElementSize) throw Exception(String(u"ArraySet::removeElement(const E&) ElementSource"));
            std::copy(ElementContainer + ElementIndex + 1, ElementContainer + ElementSize, ElementContainer + ElementIndex);
            if (ElementCapacity == 1) doClear();
            else if (--ElementSize <= ElementCapacity >> 1) {
                auto *ElementBuffer = new E[ElementSize];
                std::copy(ElementContainer, ElementContainer + ElementSize, ElementBuffer);
                delete[] ElementContainer;
                ElementContainer = new E[ElementCapacity >>= 1];
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
    };

    /**
     * The DoubleLinkedList class provides support for discrete storage of objects
     */
    template<typename E>
    class DoubleLinkedList final : public Object {
    private:
        struct LinkedNode final {
            E NodeValue;
            LinkedNode *NodeNext, *NodePrevious;

            explicit LinkedNode(const E &Value) noexcept: NodeValue(Value), NodeNext(nullptr), NodePrevious(nullptr) {}
        } *NodeHead, *NodeTail;

        intmax_t NodeSize;
    public:
        DoubleLinkedList() noexcept: NodeHead(nullptr), NodeTail(nullptr), NodeSize(0) {}

        DoubleLinkedList(std::initializer_list<E> ElementList) noexcept : NodeSize(ElementList.size()) {
            for (const E &ElementCurrent : ElementList) addElement(ElementCurrent);
        }

        ~DoubleLinkedList() noexcept {
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

        void addElement(const E &ElementSource) noexcept {
            auto *NodeCurrent = new LinkedNode(ElementSource);
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
                LinkedNode *NodeCurrent = new LinkedNode(ElementSource);
                NodeCurrent->NodeNext = NodeHead;
                NodeHead->NodePrevious = NodeCurrent;
                NodeHead = NodeCurrent;
                ++NodeSize;
                return;
            }
            LinkedNode *NodeCurrent = NodeHead;
            while (--ElementIndex) NodeCurrent = NodeCurrent->NodeNext;
            auto *NodeTarget = new LinkedNode(ElementSource);
            NodeTarget->NodePrevious = NodeCurrent;
            NodeTarget->NodeNext = NodeCurrent->NodeNext;
            NodeCurrent->NodeNext->NodePrevious = NodeTarget;
            NodeCurrent->NodeNext = NodeTarget;
            ++NodeSize;
        }

        void doReverse() noexcept {
            if (NodeHead == NodeTail) return;
            if (NodeHead->NodeNext == NodeTail) {
                std::swap(NodeHead, NodeTail);
                return;
            }
            LinkedNode *NodeCurrent = NodeHead->NodeNext, *NodeNext, *NodeTemporary = new LinkedNode();
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

        E &getElement(intmax_t ElementIndex) const {
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
            while (NodeCurrent && NodeCurrent->NodeValue.doCompare(ElementSource)) NodeCurrent = NodeCurrent->NodeNext, ++NodeIndex;
            if (!NodeCurrent) return -1;
            return NodeIndex;
        }

        bool isContains(const E &ElementSource) const noexcept {
            LinkedNode *NodeCurrent = NodeHead;
            while (NodeCurrent) {
                if (NodeCurrent->NodeValue.doCompare(ElementSource) == 0) return true;
                NodeCurrent = NodeCurrent->NodeNext;
            }
            return false;
        }

        bool isEmpty() const noexcept {
            return NodeSize == 0;
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
     * The DoubleLinkedSet class provides support for discrete storage of unique objects
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
        constexpr DoubleLinkedSet() noexcept = default;

        DoubleLinkedSet(const DoubleLinkedSet<E> &SetSource) noexcept {
            LinkedNode *NodeCurrent = SetSource.NodeHead;
            while (NodeCurrent) {
                addElement(NodeCurrent->NodeValue);
                NodeCurrent = NodeCurrent->NodeNext;
            }
        }

        ~DoubleLinkedSet() noexcept {
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

        void addElement(const E &ElementSource) noexcept {
            if (isContains(ElementSource)) return;
            auto *NodeCurrent = new LinkedNode(ElementSource);
            if (!NodeHead) NodeHead = NodeTail = NodeCurrent;
            else {
                NodeTail->NodeNext = NodeCurrent;
                NodeCurrent->NodePrevious = NodeTail;
                NodeTail = NodeCurrent;
            }
            ++NodeSize;
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
                if (NodeCurrent->NodeValue.doCompare(ElementSource) == 0) return true;
                NodeCurrent = NodeCurrent->NodeNext;
            }
            return false;
        }

        bool isEmpty() const noexcept {
            return NodeSize == 0;
        }

        DoubleLinkedSet<E> &operator=(const DoubleLinkedSet<E>&) noexcept = delete;

        void removeElement(const E &ElementSource) {
            intmax_t ElementIndex = 0;
            LinkedNode *NodeCurrent = NodeHead;
            while (NodeCurrent && NodeCurrent->NodeValue.doCompare(ElementSource)) NodeCurrent = NodeCurrent->NodeNext, ++ElementIndex;
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

    /**
     * The SingleLinkedList class provides support for discrete storage of objects
     */
    template<typename E>
    class SingleLinkedList final : public Object {
    private:
        struct LinkedNode final {
            E NodeValue;
            LinkedNode *NodeNext;

            explicit LinkedNode(const E &NodeValueSource) noexcept: NodeValue(NodeValueSource), NodeNext(nullptr) {}
        } *NodeHead, *NodeTail;

        intmax_t NodeSize;
    public:
        SingleLinkedList() noexcept: NodeHead(nullptr), NodeTail(nullptr), NodeSize(0) {}

        SingleLinkedList(std::initializer_list<E> ElementList) noexcept : NodeSize(ElementList.size()) {
            for (const E &ElementCurrent : ElementList) addElement(ElementCurrent);
        }

        ~SingleLinkedList() noexcept {
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

        void addElement(const E &ElementSource) noexcept {
            auto *NodeCurrent = new LinkedNode(ElementSource);
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
            auto *NodeTarget = new LinkedNode(ElementSource);
            NodeTarget->NodeNext = NodeCurrent->NodeNext;
            NodeCurrent->NodeNext = NodeTarget;
            ++NodeSize;
        }

        E getElement(intmax_t ElementIndex) const {
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
            while (NodeCurrent && NodeCurrent->NodeValue.doCompare(ElementSource)) NodeCurrent = NodeCurrent->NodeNext, ++NodeIndex;
            if (!NodeCurrent) return -1;
            return NodeIndex;
        }

        bool isContains(const E &ElementSource) const noexcept {
            LinkedNode *NodeCurrent = NodeHead;
            while (NodeCurrent) {
                if (NodeCurrent->NodeValue.doCompare(ElementSource) == 0) return true;
                NodeCurrent = NodeCurrent->NodeNext;
            }
            return false;
        }

        bool isEmpty() const noexcept {
            return NodeSize == 0;
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

        ArrayList<E> toArrayList() const noexcept {
            ArrayList<E> ListResult;
            LinkedNode *NodeCurrent = NodeHead;
            while (NodeCurrent) {
                ListResult.addElement(NodeCurrent->NodeValue);
                NodeCurrent = NodeCurrent->NodeNext;
            }
            return ListResult;
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

        std::forward_list<E> toSTLForwardList() const noexcept {
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
     * The SingleLinkedSet class provides support for discrete storage of unique objects
     */
    template<typename E>
    class SingleLinkedSet final : public Object {
    private:
        struct LinkedNode final {
            E NodeValue;
            LinkedNode *NodeNext;

            explicit LinkedNode(const E &NodeValueSource) noexcept: NodeValue(NodeValueSource), NodeNext(nullptr) {}
        } *NodeHead = nullptr, *NodeTail = nullptr;

        intmax_t NodeSize = 0;
    public:
        constexpr SingleLinkedSet() noexcept = default;

        SingleLinkedSet(const SingleLinkedSet<E> &SetSource) noexcept {
            LinkedNode *NodeCurrent = SetSource.NodeHead;
            while (NodeCurrent) {
                addElement(NodeCurrent->NodeValue);
                NodeCurrent = NodeCurrent->NodeNext;
            }
        }

        ~SingleLinkedSet() noexcept {
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

        void addElement(const E &ElementSource) noexcept {
            if (isContains(ElementSource)) return;
            auto *NodeCurrent = new LinkedNode(ElementSource);
            if (!NodeHead) {
                NodeHead = NodeCurrent;
                NodeTail = NodeCurrent;
            } else {
                NodeTail->NodeNext = NodeCurrent;
                NodeTail = NodeCurrent;
            }
            ++NodeSize;
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
                if (NodeCurrent->NodeValue.doCompare(ElementSource) == 0) return true;
                NodeCurrent = NodeCurrent->NodeNext;
            }
            return false;
        }

        bool isEmpty() const noexcept {
            return NodeSize == 0;
        }

        SingleLinkedSet<E> &operator=(const SingleLinkedSet<E>&) noexcept = delete;

        void removeElement(const E &ElementSource) {
            intmax_t ElementIndex = 0;
            LinkedNode *NodeCurrent = NodeHead;
            while (NodeCurrent && NodeCurrent->NodeValue.doCompare(ElementSource)) NodeCurrent = NodeCurrent->NodeNext, ++ElementIndex;
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
}
