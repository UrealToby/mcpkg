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
Result<> mcpkg::Version::comparison(Version left, Version right, bool greater, bool close) {
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

bool mcpkg::Version::operator==(const Version &version) const {
    return version.data == data;
}

bool mcpkg::Version::operator>=(const Version &version) const {
    return (bool) comparison(*this, version, true, true);
}

bool mcpkg::Version::operator<=(const Version &version) const {
    return (bool) comparison(*this, version, false, true);
}

bool mcpkg::Version::operator>(const Version &version) const {
    return (bool) comparison(*this, version, true, false);
}

bool mcpkg::Version::operator<(const Version &version) const {
    return (bool) comparison(*this, version, false, false);
}

int mcpkg::Version::size() const {
    return (int) data.size();
}

mcpkg::Version::Version(std::string src) {
    for (char i:src) {
        if (i=='.') { data.push_back("");
            continue;}
    }
    boost::split(data, src, boost::is_any_of("."), boost::token_compress_on);
}


Result<> mcpkg::VersionExpressionGreaterOrLess::compatible(const Version &version) {

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
    enum Type {
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

    SyntaxTreeNode(Type type, SyntaxTreeNode *parent) {
        this->type = type;
        this->parent = parent;
    }

    SyntaxTreeNode(const SyntaxTreeNode &copySource) // Copy constructor
    {
        switch (copySource.type) {
            case nodeOr: {
                this->value = new std::vector<SyntaxTreeNode *>(*copySource.valueSyntaxTreeNodeList());
                std::for_each(this->valueSyntaxTreeNodeList()->begin(), this->valueSyntaxTreeNodeList()->end(),
                              [this](SyntaxTreeNode *node) { node->parent = this; });
                break;
            }
            case nodeRange: {

                this->value = new std::pair<SyntaxTreeNode *, SyntaxTreeNode *>(
                        new SyntaxTreeNode(*copySource.valuePair()->first),
                        new SyntaxTreeNode(*copySource.valuePair()->second));;
                        this->valuePair()->first->parent = this;
                        this->valuePair()->second->parent = this;
                break;
            }
            case nodeRaw: {
                this->value = new std::vector<std::string>(*copySource.valueSyntaxStringList());
                break;
            }

            default: {
                this->value = new SyntaxTreeNode(*copySource.valueSyntaxTreeNode());
                this->valueSyntaxTreeNode()->parent = this;
            }
        }
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

/**
 * 将语法树构建为版本表达式
 * @param node
 * @return
 */
mcpkg::VersionExpression *parse_syntax_tree(SyntaxTreeNode *node) {
//    VersionExpression* expression;
    switch (node->type) {
        case SyntaxTreeNode::nodeRoot:
            return parse_syntax_tree(node->valueSyntaxTreeNode());
        case SyntaxTreeNode::nodeRange: {
            auto expression = new mcpkg::VersionExpressionRange();
            expression->left = dynamic_cast<mcpkg::VersionExpressionGreaterOrLess *>(parse_syntax_tree(
                    node->valuePair()->first));
            expression->right = dynamic_cast<mcpkg::VersionExpressionGreaterOrLess *>(parse_syntax_tree(
                    node->valuePair()->second));

            return expression;
        }
        case SyntaxTreeNode::nodeOr: {
            auto expression = new mcpkg::VersionExpressionOr();
            for (auto child: *node->valueSyntaxTreeNodeList()) {
                expression->expressions.push_back(parse_syntax_tree(child));
            }
            return expression;
        }
        case SyntaxTreeNode::nodeMinimum: {
            auto expression = new mcpkg::VersionExpressionGreaterOrLess();
            expression->base.data =  parse_syntax_tree(node->valueSyntaxTreeNode())->base.data;
            return expression;
        }
        case SyntaxTreeNode::nodeMaximum: {
            auto expression = new mcpkg::VersionExpressionGreaterOrLess();
            expression->greater = false;
            expression->base.data = parse_syntax_tree(node->valueSyntaxTreeNode())->base.data;

            return expression;
        }
        case SyntaxTreeNode::nodeGreater: {
            auto expression = new mcpkg::VersionExpressionGreaterOrLess();
            expression->close = false;
            expression->base.data = parse_syntax_tree(node->valueSyntaxTreeNode())->base.data;

            return expression;
        }
        case SyntaxTreeNode::nodeLess: {
            auto expression = new mcpkg::VersionExpressionGreaterOrLess();
            expression->greater = false;
            expression->close = false;

            for (auto i: parse_syntax_tree(node->valueSyntaxTreeNode())->base.data) {
                expression->base.data.push_back(i);
            }
            return expression;
        }
        case SyntaxTreeNode::nodeRaw:
            auto expression = new mcpkg::VersionExpression();
            for (auto i: *node->valueSyntaxStringList()) {
                expression->base.data.push_back(i);
            }
            return expression;
    }

    return nullptr;
}

SyntaxTreeNode build_syntax_tree(const std::string &src) {
    SyntaxTreeNode root = SyntaxTreeNode{SyntaxTreeNode::nodeRoot, nullptr};
    SyntaxTreeNode *head = &root;

    /// 构建语法树
    for (auto c: src) {
        switch (c) {
            case '^': {
                auto node = new SyntaxTreeNode{SyntaxTreeNode::nodeMinimum, head};

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

                    SyntaxTreeNode *left;
                    if (head->parent->parent->type == SyntaxTreeNode::nodeOr) {
                        left = *head->parent->parent->valueSyntaxTreeNodeList()->rbegin();
                    } else {
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
                while (head->type != SyntaxTreeNode::nodeOr) {
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

    return root;
}

mcpkg::VersionExpression *mcpkg::VersionExpression::from_string(const std::string &str) {
    SyntaxTreeNode root = build_syntax_tree(str);
    VersionExpression *expression = parse_syntax_tree(&root);

    return expression;
//    return nullptr;
}


Result<> mcpkg::VersionExpression::compatible(const Version &version) {
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

Result<> mcpkg::VersionExpressionRange::compatible(const Version &version) {
    auto lResult = left->compatible(version);
    auto rResult = right->compatible(version);

    return lResult and rResult ? Result<>::Ok() : Result<>::Err(1, "version is not in the specified range");
}

mcpkg::VersionExpressionRange::~VersionExpressionRange() {
    delete left;
    delete right;
}

Result<> mcpkg::VersionExpressionOr::compatible(const Version &version) {
    for (auto expr: expressions) {
        if (expr->compatible(version)) {
            return Result<>::Ok();
        }
    }
    return Result<>::Err(1, "the version cannot meet any conditions");
}

mcpkg::VersionExpressionOr::~VersionExpressionOr() {
    for (auto a: expressions) {
        delete a;
    }
}
