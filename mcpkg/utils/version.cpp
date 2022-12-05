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
        nodeRoot, nodeOr, nodeMinimum, nodeMaximum, nodeGreater, nodeLess, nodeRange, nodeRaw
    } type;
    SyntaxTreeNode *parent;
    void *value;
    std::string data;

    [[nodiscard]] SyntaxTreeNode *valueSyntaxTreeNode() const {
        return static_cast <SyntaxTreeNode *>(value);
    }

    [[nodiscard]] std::vector<SyntaxTreeNode *> *valueSyntaxTreeNodeList() const {
        return static_cast <std::vector<SyntaxTreeNode *> *>(value);
    }

    [[nodiscard]] std::vector<std::string> *valueSyntaxStringList() const {
        return static_cast <std::vector<std::string> *>(value);
    }

    [[nodiscard]] std::pair<SyntaxTreeNode *, SyntaxTreeNode *> *valuePair() const {
        return static_cast <std::pair<SyntaxTreeNode *, SyntaxTreeNode *> *>(value);
    }

    ~SyntaxTreeNode() {
        switch (type) {
            case nodeRoot: {
                auto child = valueSyntaxTreeNode();
                delete child;
                break;
            }
            case nodeOr: {
                auto childList = valueSyntaxTreeNodeList();
                for (auto child: *childList) {
                    delete child;
                }
                delete childList;
                break;
            }
            case nodeRange: {
                auto rangeTuple = valuePair();
                delete rangeTuple->first;
                delete rangeTuple->second;
            }

            default: {
                auto childList = static_cast <std::vector<std::string *> *>(value);
                for (auto child: *childList) {
                    delete child;
                }
                delete childList;
            }
        }
    }
};

VersionExpression *ParseTree(SyntaxTreeNode *node) {
//    VersionExpression* expression;
    switch (node->type) {
        case SyntaxTreeNode::nodeRoot:
            return ParseTree(node->valueSyntaxTreeNode());
        case SyntaxTreeNode::nodeRange: {
            auto expression = new VersionExpressionRange();
            expression->left = dynamic_cast<VersionExpressionGreaterOrLess *>(ParseTree(node->valuePair()->first));
            expression->right = dynamic_cast<VersionExpressionGreaterOrLess *>(ParseTree(node->valuePair()->second));

            return expression;
        }
        case SyntaxTreeNode::nodeOr: {
            auto expression = new VersionExpressionOr();
            for (auto child: *node->valueSyntaxTreeNodeList()) {
                expression->expressions.push_back(ParseTree(child));
            }
            return expression;
        }
        case SyntaxTreeNode::nodeMinimum: {
            auto expression = new VersionExpressionGreaterOrLess();

            expression->base = ParseTree(node->valueSyntaxTreeNode())->base;
            return expression;
        }
        case SyntaxTreeNode::nodeMaximum: {
            auto expression = new VersionExpressionGreaterOrLess();
            expression->greater = false;

            expression->base = ParseTree(node->valueSyntaxTreeNode())->base;
            return expression;
        }
        case SyntaxTreeNode::nodeGreater: {
            auto expression = new VersionExpressionGreaterOrLess();
            expression->close = false;

            expression->base = ParseTree(node->valueSyntaxTreeNode())->base;
            return expression;
        }
        case SyntaxTreeNode::nodeLess: {
            auto expression = new VersionExpressionGreaterOrLess();
            expression->greater = false;
            expression->close = false;

            expression->base = ParseTree(node->valueSyntaxTreeNode())->base;
            return expression;
        }
        case SyntaxTreeNode::nodeRaw:
            auto expression = new VersionExpression();
            expression->base.data = *node->valueSyntaxStringList();
            break;
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
                node->value = new std::vector<std::string>;

                if (head->type == SyntaxTreeNode::nodeOr) {
                    head->valueSyntaxTreeNodeList()->push_back(node);
                } else {
                    head->value = node;
                }

                head = node;
                break;
            }
            case '>': {
                auto node = new SyntaxTreeNode{SyntaxTreeNode::nodeGreater, head};
                node->value = new std::vector<std::string>;

                if (head->type == SyntaxTreeNode::nodeOr) {
                    head->valueSyntaxTreeNodeList()->push_back(node);
                } else {
                    head->value = node;
                }
                head = node;
                break;
            }
            case '<': {
                if (head->type != SyntaxTreeNode::nodeRoot and (head->parent->type == SyntaxTreeNode::nodeGreater or
                                                                head->parent->type == SyntaxTreeNode::nodeMinimum)) {
                    // 为范围表达式
                    auto left = head->parent->parent->valueSyntaxTreeNode();

                    auto right = new SyntaxTreeNode{SyntaxTreeNode::nodeLess, head};
                    right->value = new std::vector<std::string>();

                    auto pair = new std::pair<SyntaxTreeNode *, SyntaxTreeNode *>;

                    // 将节点转为 range 类型
                    auto range = new SyntaxTreeNode{SyntaxTreeNode::nodeRange, head->parent->parent};
                    if (head->type == SyntaxTreeNode::nodeOr) {
                        *head->parent->parent->valueSyntaxTreeNodeList()->rbegin() = range;
                    } else {
                        head->parent->value = range;
                    }

                    head->parent = range;

                    pair->first = left;
                    pair->second = right;
                    range->value = pair;
                    head = range;

                    break;
                }
                auto node = new SyntaxTreeNode{SyntaxTreeNode::nodeLess, head};
                node->value = new std::vector<std::string>();

                head->value = node;
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
                head->value = new std::vector<SyntaxTreeNode>();

                head = node;
                break;
            }
            case '|': {
                head = head->parent;
            }
            case ']': {
                head = head->parent;
                break;
            }
            case '.': {
                head->valueSyntaxStringList()->push_back("");
            }
            case ' ': {
                break;
            }
            default: {
                if (head->type != SyntaxTreeNode::nodeRaw) {
                    auto node = new SyntaxTreeNode{SyntaxTreeNode::nodeRaw, head};
                    if (head->type == SyntaxTreeNode::nodeOr) {
                        *head->parent->parent->valueSyntaxTreeNodeList()->rbegin() = node;
                    } else {
                        head->parent->value = node;
                    }
                    head->value = new std::vector<std::string>();
                    head->valueSyntaxStringList()->push_back("");
                }
                head->valueSyntaxStringList()->rbegin()->push_back(c);
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
