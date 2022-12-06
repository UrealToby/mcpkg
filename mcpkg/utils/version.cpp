//
// Created by toby on 2022/12/5.
//

#include "version.h"

#include <stdexcept>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <iostream>

#include "cmath"

/// 比较两个版本
/// \param left
/// \param right
/// \param greater \c true 为大于，\c false 为小于
/// \param close
/// \return
Result<> Version::comparison(Version left, Version right, bool greater, bool close) {
    int maxLen = std::min(left.size(), right.size());
    for(auto i :left.data){
        std::cout<<i<<" ";
    }
    std::cout<<std::endl;

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
    for(auto i :base.data){
        std::cout<<i<<" ";
    }
    std::cout<<std::endl;

    return Version::comparison(version, base, greater, close);
}

// ........................................
//........................................
//................8.......................
//..............,O..................,.....
//..............8.............,...........
//......8.......=8....88........8.........
//......8...........O8+?8.......8.........
//......8~.........D???8........8 ........
//.......8.......=8+??+8........Z8........
//.......8......8+?????+8O......88..,.....
//......8.......8+???????+8..,..O,........
//....,,,....=8+?88??????88:..............
//...........OD???+?8888$???8,............
//.........88?88?+??????????+8888.........
//........O+???+888888888888????+O8.......
//........8+???????+++?+??????????8.......
//.........8+????????????????????88.......
//..........,8888O?+++?O888888888.........
//

class SyntaxTreeNode {
public:
    enum {
        nodeRoot, nodeOr, nodeMinimum, nodeMaximum, nodeGreater, nodeLess, nodeRange, nodeRaw
    } type;
    SyntaxTreeNode *parent = nullptr;
    void *value = nullptr;

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
                break;
            }
            case nodeRaw: {
                auto childList = valueSyntaxStringList();
                delete childList;
                break;
            }

            default: {
                delete valueSyntaxTreeNode();
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

            for (auto i: ParseTree(node->valueSyntaxTreeNode())->base.data) {
                expression->base.data.push_back(i);
            }
            return expression;
        }
        case SyntaxTreeNode::nodeMaximum: {
            auto expression = new VersionExpressionGreaterOrLess();
            expression->greater = false;

            for (auto i: ParseTree(node->valueSyntaxTreeNode())->base.data) {
                expression->base.data.push_back(i);
            }
            return expression;
        }
        case SyntaxTreeNode::nodeGreater: {
            auto expression = new VersionExpressionGreaterOrLess();
            expression->close = false;

            for (auto i: ParseTree(node->valueSyntaxTreeNode())->base.data) {
                expression->base.data.push_back(i);
            }
            return expression;
        }
        case SyntaxTreeNode::nodeLess: {
            auto expression = new VersionExpressionGreaterOrLess();
            expression->greater = false;
            expression->close = false;

            for (auto i: ParseTree(node->valueSyntaxTreeNode())->base.data) {
                expression->base.data.push_back(i);
            }
            return expression;
        }
        case SyntaxTreeNode::nodeRaw:
            auto expression = new VersionExpression();
            for (auto i: *node->valueSyntaxStringList()) {
                expression->base.data.push_back(i);
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

                if (head->type == SyntaxTreeNode::nodeOr) {
                    head->valueSyntaxTreeNodeList()->push_back(node);
                } else {
                    head->value = node;
                }

                for (auto child: *head->valueSyntaxTreeNodeList()) {
                    child;
                }

                head = node;

                for (auto child: *root.valueSyntaxTreeNode()->valueSyntaxTreeNodeList()) {
                    child;
                }

                break;
            }
            case '>': {
                auto node = new SyntaxTreeNode{SyntaxTreeNode::nodeGreater, head};

                if (head->type == SyntaxTreeNode::nodeOr) {
                    head->valueSyntaxTreeNodeList()->push_back(node);
                } else {
                    head->value = node;
                }

                head = node;
                break;
            }
            case '<': {
                if (head->parent->type == SyntaxTreeNode::nodeGreater or
                    head->parent->type == SyntaxTreeNode::nodeMinimum) {
                    // 为范围表达式

                    auto range = new SyntaxTreeNode{SyntaxTreeNode::nodeRange, head->parent->parent};

                    SyntaxTreeNode* left;
                    if (head->parent->parent->type == SyntaxTreeNode::nodeOr){
                        left = *head->parent->parent->valueSyntaxTreeNodeList()->rbegin();
                    }
                    else{
                        left = head->parent->parent->valueSyntaxTreeNode();
                    }

                    left->parent = range;
                    auto right = new SyntaxTreeNode{SyntaxTreeNode::nodeLess, range};

                    auto pair = new std::pair<SyntaxTreeNode *, SyntaxTreeNode *>;
                    pair->first = left;
                    pair->second = right;

                    // 将节点转为 range 类型
                    if (range->parent->type == SyntaxTreeNode::nodeOr) {
                        *range->parent->valueSyntaxTreeNodeList()->rbegin() = range;
                    } else {
                        head->parent->value = range;
                    }

                    range->value = pair;
                    head = right;

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
                node->value = new std::vector<SyntaxTreeNode>();

                head->value = node;

                head = node;
                break;
            }
            case '|': {
                while (head->type!=SyntaxTreeNode::nodeOr){
                    head = head->parent;
                }
                break;
            }
            case ']': {
                head = head->parent->parent;
                break;
            }
            case '.': {
                head->valueSyntaxStringList()->push_back("");
                break;
            }
            case ' ': {
                break;
            }
            default: {
                if (head->type != SyntaxTreeNode::nodeRaw) {
                    auto node = new SyntaxTreeNode{SyntaxTreeNode::nodeRaw, head};
                    if (head->type == SyntaxTreeNode::nodeOr) {
                        head->valueSyntaxTreeNodeList()->push_back(node);
                    } else {
                        head->value = node;
                    }
                    node->value = new std::vector<std::string>();
                    node->valueSyntaxStringList()->push_back("");
                    head = node;
                }
                head->valueSyntaxStringList()->rbegin()->push_back(c);

            }
        }
    }


    return ParseTree(&root);
//    return nullptr;
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
