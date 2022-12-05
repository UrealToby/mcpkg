//
// Created by toby on 2022/12/5.
//

#include "version.h"

#include <stdexcept>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

#include "cmath"

/// 比较两个版本
/// \param left
/// \param right
/// \param greater \c true 为大于，\c false 为小于
/// \param close
/// \return
Result<> Version::comparison(Version left, Version right, bool greater, bool close) {
    int maxLen = std::min(left.size(), right.size());

    for (int i = 0; i < maxLen; ++i) {
        auto a = left.data[i];
        auto b = right.data[i];
        if (left.data[i] == right.data[i]) {
            continue;
        } else {
            int left_section;
            int right_section;
            try {
                left_section = std::stoi(left.data[i]);
                right_section = std::stoi(right.data[i]);
            }
            catch (std::invalid_argument &e) {
                return Result<>::Err(2, "cannot compare sizes between strings");
            }
            if ((left_section > right_section) == greater) {
                return Result<>::Ok();
            } else {
                return Result<>::Err(1, "unsatisfactory relationship");
            }
        }
    }
    if (left.size() == right.size()) {
        return close ? Result<>::Ok() : Result<>::Err(1, "unsatisfactory relationship");
    }
    if (left.size() > right.size() == greater) {
        return Result<>::Ok();
    }

    return Result<>::Err(1, "unsatisfactory relationship");
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

Version::Version(std::string src) {
    boost::split(data, src, boost::is_any_of("."), boost::token_compress_on);
}


Result<> VersionExpressionGreaterOrLess::compatible(const Version &version) {
    return Version::comparison(version, base, greater, close);
}

class SyntaxTreeNode {
public:
    enum {
        nodeRoot, nodeOr, nodeOrItem, nodeMinimum, nodeMaximum, nodeGreater, nodeLess, nodeRange, nodeV
    } type;
    SyntaxTreeNode *parent;
    std::vector<SyntaxTreeNode *> child;
    std::string data;

    ~SyntaxTreeNode() {
        for (auto c: child) {
            delete c;
        }
    }
};

VersionExpression *ParseTree(SyntaxTreeNode *root) {
//    VersionExpression* expression;
    if (root->type == SyntaxTreeNode::nodeRoot or root->type == SyntaxTreeNode::nodeOrItem) {
        if (root->child[0]->type == SyntaxTreeNode::nodeV) {
            auto *expression = new VersionExpression();
            for (auto child: root->child) {
                expression->base.data.push_back(ParseTree(child)->base.data[0]);
            }
            return expression;
        }
        return ParseTree(root->child[0]);
    } else if (root->type == SyntaxTreeNode::nodeRange) {
        auto expression = new VersionExpressionRange();
        expression->left = dynamic_cast<VersionExpressionGreaterOrLess *>(ParseTree(root->child[0]));
        expression->right = dynamic_cast<VersionExpressionGreaterOrLess *>(ParseTree(root->child[1]));
        return expression;
    } else if (root->type == SyntaxTreeNode::nodeV) {
        auto expression = new VersionExpression();
        expression->base.data.push_back(root->data);
        return expression;
    } else if (root->type == SyntaxTreeNode::nodeOr) {
        auto expression = new VersionExpressionOr();
        for (auto node: root->child) {
            expression->expressions.push_back(ParseTree(node));
        }
        return expression;
    } else if (root->type == SyntaxTreeNode::nodeMinimum) {
        auto expression = new VersionExpressionGreaterOrLess();
        for (const auto &node: root->child) {
            expression->base.data.push_back(ParseTree(node)->base.data[0]);
        }
        return expression;
    } else if (root->type == SyntaxTreeNode::nodeGreater) {
        auto expression = new VersionExpressionGreaterOrLess();
        expression->close = false;
        for (const auto &node: root->child) {
            expression->base.data.push_back(ParseTree(node)->base.data[0]);
        }
        return expression;
    } else if (root->type == SyntaxTreeNode::nodeMaximum) {
        auto expression = new VersionExpressionGreaterOrLess();
        expression->greater = false;
        for (const auto &node: root->child) {
            expression->base.data.push_back(ParseTree(node)->base.data[0]);
        }
        return expression;
    } else if (root->type == SyntaxTreeNode::nodeLess) {
        auto expression = new VersionExpressionGreaterOrLess();
        expression->greater = false;
        expression->close = false;
        for (const auto &node: root->child) {
            expression->base.data.push_back(ParseTree(node)->base.data[0]);
        }
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
                auto node = new SyntaxTreeNode{SyntaxTreeNode::nodeMinimum, head};
                head->child.push_back(node);
                head = node;
                break;
            }
            case '>': {
                auto node = new SyntaxTreeNode{SyntaxTreeNode::nodeGreater, head};
                head->child.push_back(node);
                head = node;

                break;
            }
            case '<': {
                if (head->parent->type == SyntaxTreeNode::nodeMinimum or
                    head->parent->type == SyntaxTreeNode::nodeGreater) {
                    auto range = new SyntaxTreeNode{SyntaxTreeNode::nodeRange, head->parent->parent};
                    auto left = *head->parent->parent->child.rbegin();
                    *head->parent->parent->child.rbegin() = range;
                    range->child.push_back(left);
                    head = range;
                }
                auto node = new SyntaxTreeNode{SyntaxTreeNode::nodeLess, head};
                head->child.push_back(node);
                head = node;
                break;
            }
            case '=': {
                if (head->type == SyntaxTreeNode::nodeGreater) {
                    head->type = SyntaxTreeNode::nodeMinimum;
                } else if (head->type == SyntaxTreeNode::nodeLess) {
                    head->type = SyntaxTreeNode::nodeMaximum;
                }
                break;
            }
            case '[': {
                auto node = new SyntaxTreeNode{SyntaxTreeNode::nodeOr, head};
                head->child.push_back(node);
                head = node;
                node = new SyntaxTreeNode{SyntaxTreeNode::nodeOrItem, head};
                head->child.push_back(node);
                head = node;
                break;
            }
            case '|': {
                while (head->type != SyntaxTreeNode::nodeOr) {
                    head = head->parent;
                }
                auto node = new SyntaxTreeNode{SyntaxTreeNode::nodeOrItem, head};
                head->child.push_back(node);
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
                    head->child.push_back(node);
                    head = node;
                }
                head->data += c;
            }
        }
    }

    return ParseTree(&root);
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
    auto lResult = left->compatible(version);
    auto rResult = right->compatible(version);

    return lResult and rResult ? Result<>::Ok() : Result<>::Err(1, "version is not in the specified range");
}

VersionExpressionRange::~VersionExpressionRange() {
    delete left;
    delete right;
}

Result<> VersionExpressionOr::compatible(const Version &version) {
    for (auto expr: expressions) {
        if (expr->compatible(version)) {
            return Result<>::Ok();
        }
    }
    return Result<>::Err(1, "the version cannot meet any conditions");
}

VersionExpressionOr::~VersionExpressionOr() {
    for (auto a: expressions) {
        delete a;
    }
}
