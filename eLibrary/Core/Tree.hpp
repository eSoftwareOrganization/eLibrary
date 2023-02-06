#pragma once

#include <Core/Exception.hpp>

#include <optional>

namespace eLibrary {
    template<Comparable K, typename V>
    class RedBlackTree final : public Object {
    private:
        enum NodeColorEnumeration {
            ColorRed,
            ColorBlack,
        };

        struct RedBlackNode final {
            K NodeKey;
            V NodeValue;
            NodeColorEnumeration NodeColor;
            RedBlackNode *NodeChildLeft, *NodeChildRight, *NodeParent;

            constexpr RedBlackNode(const K &NodeKeySource, const V &NodeValueSource) noexcept: NodeChildLeft(nullptr), NodeChildRight(nullptr), NodeColor(ColorRed), NodeKey(NodeKeySource), NodeParent(nullptr), NodeValue(NodeValueSource) {}

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
        } *NodeRoot;

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
            auto NodeRelation = NodeCurrent->NodeKey.doCompare(NodeKey);
            if (NodeRelation == 0) return NodeCurrent;
            else if (NodeRelation < 0) return doSearchCore(NodeCurrent->NodeChildRight, NodeKey);
            else return doSearchCore(NodeCurrent->NodeChildLeft, NodeKey);
        }

        uintmax_t getHeightCore(RedBlackNode *NodeCurrent) const noexcept {
            if (NodeCurrent->NodeChildLeft && NodeCurrent->NodeChildRight)
                return getMaximum(getHeightCore(NodeCurrent->NodeChildLeft),
                                  getHeightCore(NodeCurrent->NodeChildRight)) + 1;
            if (NodeCurrent->NodeChildLeft) return getHeightCore(NodeCurrent->NodeChildLeft) + 1;
            if (NodeCurrent->NodeChildRight) return getHeightCore(NodeCurrent->NodeChildRight) + 1;
            return 1;
        }

        uintmax_t getSizeCore(RedBlackNode *NodeCurrent) const noexcept {
            if (!NodeCurrent) return 0;
            return getSizeCore(NodeCurrent->NodeChildLeft) + getSizeCore(NodeCurrent->NodeChildRight) + 1;
        }

    public:
        constexpr RedBlackTree() noexcept: NodeRoot(nullptr) {}

        ~RedBlackTree() noexcept {
            if (NodeRoot) {
                delete NodeRoot;
                NodeRoot = nullptr;
            }
        }

        void doInsert(const K &NodeKey, const V &NodeValue) noexcept {
            auto *NodeTarget = new RedBlackNode(NodeKey, NodeValue);
            if (NodeRoot == nullptr) {
                NodeTarget->NodeColor = ColorBlack;
                NodeRoot = NodeTarget;
                return;
            }
            auto *NodeParent = NodeRoot;
            while (NodeParent) {
                auto NodeRelation = NodeTarget->NodeKey.doCompare(NodeParent->NodeKey);
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
            while ((NodeParent = NodeTarget->NodeParent) && NodeParent->NodeColor == ColorRed)
                if (NodeParent == NodeParent->NodeParent->NodeChildLeft) {
                    RedBlackNode *NodeUncle = NodeParent->NodeParent->NodeChildRight;
                    if (NodeUncle && NodeUncle->NodeColor == ColorRed) {
                        NodeParent->NodeColor = ColorBlack;
                        NodeParent->NodeParent->NodeColor = ColorRed;
                        NodeUncle->NodeColor = ColorBlack;
                        NodeTarget = NodeParent->NodeParent;
                        continue;
                    }
                    if (NodeParent->NodeChildRight == NodeTarget) {
                        doRotateLeft(NodeParent);
                        std::swap(NodeTarget, NodeParent);
                    }
                    NodeParent->NodeColor = ColorBlack;
                    NodeParent->NodeParent->NodeColor = ColorRed;
                    doRotateRight(NodeParent->NodeParent);
                } else {
                    RedBlackNode *NodeUncle = NodeParent->NodeParent->NodeChildLeft;
                    if (NodeUncle && NodeUncle->NodeColor == ColorRed) {
                        NodeParent->NodeColor = ColorBlack;
                        NodeParent->NodeParent->NodeColor = ColorRed;
                        NodeUncle->NodeColor = ColorBlack;
                        NodeTarget = NodeParent->NodeParent;
                        continue;
                    }
                    if (NodeParent->NodeChildLeft == NodeTarget) {
                        doRotateRight(NodeParent);
                        std::swap(NodeTarget, NodeParent);
                    }
                    NodeParent->NodeColor = ColorBlack;
                    NodeParent->NodeParent->NodeColor = ColorRed;
                    doRotateLeft(NodeParent->NodeParent);
                }
            NodeRoot->NodeColor = ColorBlack;
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
                if (NodeColor == ColorBlack) goto doRemoveFixup;
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
            if (NodeColor == ColorRed) return;
            doRemoveFixup:
            while ((!NodeChild || NodeChild->NodeColor == ColorBlack) && NodeChild != NodeRoot)
                if (NodeParent->NodeChildLeft == NodeChild) {
                    RedBlackNode *NodeUncle = NodeParent->NodeChildRight;
                    if (NodeUncle->NodeColor == ColorRed) {
                        NodeParent->NodeColor = ColorRed;
                        NodeUncle->NodeColor = ColorBlack;
                        doRotateLeft(NodeParent);
                        NodeUncle = NodeParent->NodeChildRight;
                    }
                    if ((!NodeUncle->NodeChildLeft || NodeUncle->NodeChildLeft->NodeColor == ColorBlack) &&
                        (!NodeUncle->NodeChildRight || NodeUncle->NodeChildRight->NodeColor == ColorBlack)) {
                        NodeUncle->NodeColor = ColorRed;
                        NodeChild = NodeParent;
                        NodeParent = NodeChild->NodeParent;
                    } else {
                        if (!NodeUncle->NodeChildRight || NodeUncle->NodeChildRight->NodeColor == ColorBlack) {
                            NodeUncle->NodeChildLeft->NodeColor = ColorBlack;
                            NodeUncle->NodeColor = ColorRed;
                            doRotateRight(NodeUncle);
                            NodeUncle = NodeParent->NodeChildRight;
                        }
                        NodeUncle->NodeColor = NodeParent->NodeColor;
                        NodeParent->NodeColor = ColorBlack;
                        NodeUncle->NodeChildRight->NodeColor = ColorBlack;
                        doRotateLeft(NodeParent);
                        NodeChild = NodeRoot;
                        break;
                    }
                } else {
                    RedBlackNode *NodeUncle = NodeParent->NodeChildLeft;
                    if (NodeUncle->NodeColor == ColorRed) {
                        NodeParent->NodeColor = ColorRed;
                        NodeUncle->NodeColor = ColorBlack;
                        doRotateRight(NodeParent);
                        NodeUncle = NodeParent->NodeChildLeft;
                    }
                    if ((!NodeUncle->NodeChildLeft || NodeUncle->NodeChildLeft->NodeColor == ColorBlack) &&
                        (!NodeUncle->NodeChildRight || NodeUncle->NodeChildRight->NodeColor == ColorBlack)) {
                        NodeUncle->NodeColor = ColorRed;
                        NodeChild = NodeParent;
                        NodeParent = NodeChild->NodeParent;
                    } else {
                        if (!NodeUncle->NodeChildLeft || NodeUncle->NodeChildLeft->NodeColor == ColorBlack) {
                            NodeUncle->NodeChildRight->NodeColor = ColorBlack;
                            NodeUncle->NodeColor = ColorRed;
                            doRotateLeft(NodeUncle);
                            NodeUncle = NodeParent->NodeChildLeft;
                        }
                        NodeUncle->NodeColor = NodeParent->NodeColor;
                        NodeParent->NodeColor = ColorBlack;
                        NodeUncle->NodeChildLeft->NodeColor = ColorBlack;
                        doRotateRight(NodeParent);
                        NodeChild = NodeRoot;
                        break;
                    }
                }
            if (NodeChild) NodeChild->NodeColor = ColorBlack;
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
}
