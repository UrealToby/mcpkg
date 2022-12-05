//
// Created by toby on 2022/12/5.
//

#include <stdexcept>
#include "version.h"
#include "cmath"

/// 比较两个版本
/// \param left
/// \param right
/// \param greater \c true 为大于，\c false 为小于
/// \param close
/// \return
Result<> Version::comparison(Version left, Version right, bool greater, bool close) {
    int maxLen = std::max(left.size(), right.size());
    for (int i = 0; i < maxLen; ++i) {
        if (left.data[i] == right.data[i]) {
            continue;
        } else {
            int base_section;
            int version_section;
            try {
                base_section = std::stoi(left.data[i]);
                version_section = std::stoi(right.data[i]);
            }
            catch (std::invalid_argument &e) {
                return Result<>::Err(2, "cannot compare sizes between strings");
            }
            if ((base_section > version_section) == greater) {
                return Result<>::Ok();
            } else {
                return Result<>::Err(1, "unsatisfactory relationship");
            }
        }
    }
    return close ? Result<>::Ok() : Result<>::Err(1, "unsatisfactory relationship");
}

bool Version::operator==(const Version &version) const {
    return version.data == data;
}

bool Version::operator>=(const Version &version) const {
    return (bool) comparison(*this, version, true, true);
}

bool Version::operator<=(const Version &version) const {
    return (bool) comparison(*this, version, false, true);
}

bool Version::operator>(const Version &version) const {
    return (bool) comparison(*this, version, true, false);
}

bool Version::operator<(const Version &version) const {
    return (bool) comparison(*this, version, false, false);
}

int Version::size() const {
    return (int) data.size();
}


Result<> VersionExpressionGreaterOrLess::compatible(const Version &version) {
    return Version::comparison(base, version, greater, close);
}

class SyntaxTreeNode {
public:
    enum {
        nodeRoot, nodeOr, nodeOrItem, nodeMinimum, nodeMaximum, nodeGreater, nodeLess, nodeSection, nodeV
    } type;
    SyntaxTreeNode *parent;
    std::vector<SyntaxTreeNode> child;
    std::string data;
};

VersionExpression *ParseTree(SyntaxTreeNode root) {

//    VersionExpression* expression;
    if (root.type == SyntaxTreeNode::nodeRoot or root.type == SyntaxTreeNode::nodeOrItem) {
        if (root.child[0].type == SyntaxTreeNode::nodeV) {
            auto *expression = new VersionExpression();
            for (const auto &child: root.child) {
                expression->base.data.push_back(ParseTree(child)->base.data[0]);
            }
            return expression;
        }
        return ParseTree(root);
    } else if (root.type == SyntaxTreeNode::nodeV) {
        auto expression = new VersionExpression();
        expression->base.data.push_back(root.data);
    } else if (root.type == SyntaxTreeNode::nodeOr) {
        auto expression = new VersionExpressionOr();
        for (const auto &node: root.child) {
            expression->expressions.push_back(ParseTree(node));
        }
    } else if (root.type == SyntaxTreeNode::nodeMinimum) {
        return new VersionExpressionGreaterOrLess();
    } else if (root.type == SyntaxTreeNode::nodeGreater) {
        auto expression = new VersionExpressionGreaterOrLess();
        expression->close = false;
        return expression;
    } else if (root.type == SyntaxTreeNode::nodeMaximum) {
        auto expression = new VersionExpressionGreaterOrLess();
        expression->greater = false;
        return expression;
    } else if (root.type == SyntaxTreeNode::nodeLess) {
        auto expression = new VersionExpressionGreaterOrLess();
        expression->greater = false;
        expression->close = false;
        return expression;
    }
    return nullptr;
}


VersionExpression *VersionExpression::from_string(const std::string &str) {
    SyntaxTreeNode root = SyntaxTreeNode{SyntaxTreeNode::nodeRoot, nullptr};
    SyntaxTreeNode *head = &root;

    /// 构建语法树
    for (auto c: str) {
        switch (c) {
            case '^': {
                if (head->parent->type == SyntaxTreeNode::nodeMinimum or
                    head->parent->type == SyntaxTreeNode::nodeGreater) {
                    auto sec = new SyntaxTreeNode{SyntaxTreeNode::nodeSection, head->parent->parent};
                    head->parent = sec;
                    head = sec;
                }

                auto node = new SyntaxTreeNode{SyntaxTreeNode::nodeMinimum, head};
                head->child.push_back(*node);
                head = node;
                break;
            }
            case '>': {
                if (head->parent->type == SyntaxTreeNode::nodeMinimum or
                    head->parent->type == SyntaxTreeNode::nodeGreater) {
                    auto sec = new SyntaxTreeNode{SyntaxTreeNode::nodeSection, head->parent->parent};
                    head->parent = sec;
                    head = sec;
                }

                auto node = new SyntaxTreeNode{SyntaxTreeNode::nodeGreater, head};
                head->child.push_back(*node);
                head = node;

                break;
            }
            case '<': {
                auto node = new SyntaxTreeNode{SyntaxTreeNode::nodeLess, head};
                head->child.push_back(*node);
                head = node;
                break;
            }
            case '=': {
                if (head->parent->type == SyntaxTreeNode::nodeGreater) {
                    head->parent->type = SyntaxTreeNode::nodeMinimum;
                } else if (head->parent->type == SyntaxTreeNode::nodeLess) {
                    head->parent->type = SyntaxTreeNode::nodeMaximum;
                }
                break;
            }
            case '[': {
                auto node = new SyntaxTreeNode{SyntaxTreeNode::nodeOr, head};
                head->child.push_back(*node);
                head = node;
                node = new SyntaxTreeNode{SyntaxTreeNode::nodeOrItem, head};
                head->child.push_back(*node);
                head = node;
            }
            case '|': {
                head = head->parent;
                auto node = new SyntaxTreeNode{SyntaxTreeNode::nodeOrItem, head};
                head->child.push_back(*node);
                head = node;
                break;
            }
            case ']': {
                head = head->parent->parent;
                break;
            }
            case '.': {
                head = head->parent;
                break;
            }
            default: {
                if (head->type != SyntaxTreeNode::nodeV) {
                    auto node = new SyntaxTreeNode{SyntaxTreeNode::nodeV, head};
                    head->child.push_back(*node);
                    node->data += c;
                }
            }
        }
    }

    return ParseTree(root);
}


Result<> VersionExpression::compatible(const Version &version) {
    if (base == version) {
        return Result<>::Ok();
    }
    int len = base.size();
    if (len != version.size()) {
        return Result<>::Err(1, "inconsistent version");
    }
    for (int i = 0; i < len; ++i) {
        if (base.data[i] == "*") {
            continue;
        }
        if (base.data[i] != version.data[i]) {
            return Result<>::Err(1, "inconsistent version");
        }
    }
    return Result<>::Ok();
}

Result<> VersionExpressionRange::compatible(const Version &version) {
    auto lResult = left.compatible(version);
    auto rResult = right.compatible(version);

    return lResult and rResult ? Result<>::Ok() : Result<>::Err(1, "version is not in the specified range");
}

Result<> VersionExpressionOr::compatible(const Version &version) {
    for (auto expr: expressions) {
        if (expr->compatible(version)) {
            return Result<>::Ok();
        }
    }
    return Result<>::Err(1, "The version cannot meet any conditions");
}

VersionExpressionOr::~VersionExpressionOr() {
    for (auto a:expressions) {
        delete a;
    }
}
