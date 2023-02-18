#pragma once

#include <Core/Exception.hpp>

#include <array>
#include <forward_list>
#include <list>
#include <vector>

namespace eLibrary {
    template<typename E>
    class ArrayList final : public Object {
    private:
        intmax_t ElementCapacity, ElementSize;
        E *ElementContainer;

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
            while (ElementCapacity < ElementList.size())
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
                memcpy(ElementBuffer, ElementContainer, sizeof(E) * ElementSize);
                delete[] ElementContainer;
                ElementContainer = new E[ElementCapacity <<= 1];
                memcpy(ElementContainer, ElementBuffer, sizeof(E) * ElementSize);
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
                memcpy(ElementBuffer, ElementContainer, sizeof(E) * ElementSize);
                delete[] ElementContainer;
                ElementContainer = new E[ElementCapacity <<= 1];
                memcpy(ElementContainer, ElementBuffer, sizeof(E) * ElementSize);
                delete[] ElementBuffer;
            }
            memcpy(ElementContainer + ElementIndex + 1, ElementContainer + ElementIndex,
                   sizeof(E) * (ElementSize - ElementIndex - 1));
            ElementContainer[ElementIndex] = ElementSource;
            ++ElementSize;
        }

        void doAssign(const ArrayList<E> &ElementSource) noexcept {
            if (&ElementSource == this) return;
            delete[] ElementContainer;
            ElementContainer = new E[ElementCapacity = ElementSource.ElementCapacity];
            memcpy(ElementContainer, ElementSource.ElementContainer, sizeof(E) * (ElementSize = ElementSource.ElementSize));
        }

        void doClear() noexcept {
            if (ElementCapacity && ElementSize && ElementContainer) {
                ElementCapacity = 0;
                ElementSize = 0;
                delete[] ElementContainer;
                ElementContainer = nullptr;
            }
        }

        ArrayList<E> doConcat(const ArrayList<E> &ElementSource) const noexcept {
            std::array<E, this->ElementSize + ElementSource.ElementSize> ElementBuffer;
            memcpy(ElementBuffer, ElementContainer, sizeof(E) * ElementSize);
            memcpy(ElementBuffer + ElementSize, ElementSource.ElementContainer, sizeof(E) * ElementSource.ElementSize);
            return ElementBuffer;
        }

        void doReverse() noexcept {
            E *ElementBuffer = new E[ElementSize];
            for (intmax_t ElementIndex = 0;ElementIndex < ElementSize;++ElementIndex)
                ElementBuffer[ElementSize - ElementIndex - 1] = ElementContainer[ElementIndex];
            memcpy(ElementContainer, ElementBuffer, sizeof(E) * ElementBuffer);
            delete[] ElementBuffer;
        }

        E getElement(intmax_t ElementIndex) const {
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
                if (ElementContainer[ElementIndex] == ElementSource) return ElementIndex;
            return -1;
        }

        bool isContains(const E &ElementSource) const noexcept {
            for (intmax_t ElementIndex = 0; ElementIndex < ElementSize; ++ElementIndex)
                if (ElementContainer[ElementIndex] == ElementSource) return true;
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
            memcpy(ElementContainer + ElementIndex, ElementContainer + ElementIndex + 1,
                   sizeof(E) * (ElementSize - ElementIndex - 1));
            if (ElementCapacity == 1) doClear();
            else if (--ElementSize <= ElementCapacity >> 1) {
                auto *ElementBuffer = new E[ElementSize];
                memcpy(ElementBuffer, ElementContainer, sizeof(E) * ElementSize);
                delete[] ElementContainer;
                ElementContainer = new E[ElementCapacity >>= 1];
                memcpy(ElementContainer, ElementBuffer, sizeof(E) * ElementSize);
                delete[] ElementBuffer;
            }
        }

        void setElement(intmax_t ElementIndex, const E &ElementSource) {
            if (ElementIndex < 0) ElementIndex += ElementSize;
            if (ElementIndex < 0 || ElementIndex >= ElementSize)
                throw Exception(String(u"ArrayList<E>::setElement(intmax_t, const E&) ElementIndex"));
            ElementContainer[ElementIndex] = ElementSource;
        }

        auto toArray() const noexcept {
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
                CharacterStream.addString(String::valueOf(ElementContainer[ElementIndex]).toU16String());
                CharacterStream.addCharacter(u',');
            }
            if (ElementSize) CharacterStream.addString(String::valueOf(ElementContainer[ElementSize - 1]).toU16String());
            CharacterStream.addCharacter(u']');
            return CharacterStream.toString();
        }
    };

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
            if (ElementIndex < (NodeSize >> 1)) {
                LinkedNode *NodeCurrent = NodeHead;
                if (ElementIndex) while (--ElementIndex) NodeCurrent = NodeCurrent->NodeNext;
                auto *NodeTarget = new LinkedNode(ElementSource);
                NodeTarget->NodePrevious = NodeCurrent;
                NodeTarget->NodeNext = NodeCurrent->NodeNext;
                NodeCurrent->NodeNext = NodeTarget;
            } else {
                LinkedNode *NodeCurrent = NodeTail;
                ElementIndex = NodeSize - ElementIndex - 1;
                if (ElementIndex) while (--ElementIndex) NodeCurrent = NodeCurrent->NodePrevious;
                auto *NodeTarget = new LinkedNode(ElementSource);
                NodeTarget->NodeNext = NodeCurrent;
                NodeTarget->NodePrevious = NodeCurrent->NodePrevious;
                NodeCurrent->NodePrevious = NodeTarget;
                NodeTarget->NodePrevious->NodeNext = NodeTarget;
            }
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

        E getElement(intmax_t ElementIndex) const {
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

        intmax_t indexOf(const E &ElementSource) const {
            intmax_t NodeIndex = 0;
            LinkedNode *NodeCurrent = NodeHead;
            while (NodeCurrent && NodeCurrent->NodeValue != ElementSource) NodeCurrent = NodeCurrent->NodeNext, ++NodeIndex;
            if (!NodeCurrent) return -1;
            return NodeIndex;
        }

        void removeElement(const E &ElementSource) {
            intmax_t ElementIndex = doFindElement(ElementSource);
            if (ElementIndex == -1) throw Exception(String(u"DoubleLinkedList<E>::removeElement(const E&) ElementSource"));
            removeIndex(ElementIndex);
        }

        void removeIndex(intmax_t ElementIndex) {
            if (ElementIndex < 0) ElementIndex += NodeSize;
            if (ElementIndex < 0 || ElementIndex >= NodeSize)
                throw Exception(String(u"DoubleLinkedList<E>::removeElement(intmax_t) ElementIndex"));
            LinkedNode *NodeCurrent;
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
                    CharacterStream.addString(String::valueOf(NodeCurrent->NodeValue).toU16String());
                    CharacterStream.addCharacter(u',');
                    NodeCurrent = NodeCurrent->NodeNext;
                }
                CharacterStream.addString(String::valueOf(NodeCurrent->NodeValue).toU16String());
            }
            CharacterStream.addCharacter(u']');
            return CharacterStream.toString();
        }
    };

    template<typename E>
    class SingleLinkedList final : public Object {
    private:
        struct LinkedNode final {
            E NodeValue;
            LinkedNode *NodeNext;

            explicit LinkedNode(const E &Value) noexcept: NodeValue(Value), NodeNext(nullptr) {}
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

        intmax_t indexOf(const E &ElementSource) noexcept {
            intmax_t NodeIndex = 0;
            LinkedNode *NodeCurrent = NodeHead;
            while (NodeCurrent && NodeCurrent->NodeValue != ElementSource) NodeCurrent = NodeCurrent->NodeNext, ++NodeIndex;
            if (!NodeCurrent) return -1;
            return NodeIndex;
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
                    CharacterStream.addString(String::valueOf(NodeCurrent->NodeValue).toU16String());
                    CharacterStream.addCharacter(u',');
                    NodeCurrent = NodeCurrent->NodeNext;
                }
                CharacterStream.addString(String::valueOf(NodeCurrent->NodeValue).toU16String());
            }
            CharacterStream.addCharacter(u']');
            return CharacterStream.toString();
        }
    };
}
