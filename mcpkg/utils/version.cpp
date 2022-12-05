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
    int maxLen = std::max(left.size(),right.size());
    for (int i = 0; i < maxLen; ++i) {
        if (left.data[i] == right.data[i]){
            continue;
        }else{
            int base_section;
            int version_section;
            try{
                base_section = std::stoi(left.data[i]);
                version_section = std::stoi(right.data[i]);
            }
            catch (std::invalid_argument& e){
                return Result<>::Err(2,"cannot compare sizes between strings");
            }
            if ((base_section > version_section) == greater){
                return Result<>::Ok();
            }else {
                return Result<>::Err(1,"unsatisfactory relationship");
            }
        }
    }
    return close ? Result<>::Ok() : Result<>::Err(1,"unsatisfactory relationship");
}

bool Version::operator==(const Version& version) const {
    return version.data == data;
}

bool Version::operator>=(const Version &version) const {
    return (bool)comparison(*this,version,true, true);
}

bool Version::operator<=(const Version &version) const {
    return (bool)comparison(*this,version, false, true);
}

bool Version::operator>(const Version &version) const {
    return (bool)comparison(*this,version,true, false);
}

bool Version::operator<(const Version &version) const {
    return (bool)comparison(*this,version, false, false);
}

int Version::size() const {
    return (int)data.size();
}


Result<> VersionExpressionGreaterOrLess::compatible(Version version) {
    return Version::comparison(base,version,greater,close);
}

class ParseTreeNode{
public:
    enum {nodeRoot,nodeOr,nodeOrItem,nodeMinimum,nodeMaximum,nodeGreater,nodeLess,nodeSection,nodeV}type;
    ParseTreeNode * parent;
    std::vector<ParseTreeNode> child;
    std::string data;
};

VersionExpression *VersionExpression::from_string(const std::string& str) {
    ParseTreeNode root = ParseTreeNode{ParseTreeNode::nodeRoot, nullptr};
    ParseTreeNode* head=&root;

    /// 构建语法树
    for (auto c: str) {
        if (c == '^' ){
            if (head->parent->type == ParseTreeNode::nodeMinimum or head->parent->type == ParseTreeNode::nodeGreater){
                auto sec = new ParseTreeNode{ParseTreeNode::nodeSection,head->parent->parent};
                head->parent = sec;
                head = sec;
            }

            auto node = new ParseTreeNode{ParseTreeNode::nodeMinimum,head};
            head->child.push_back(*node);
            head = node;
        }else if (c=='>'){
            if (head->parent->type == ParseTreeNode::nodeMinimum or head->parent->type == ParseTreeNode::nodeGreater){
                auto sec = new ParseTreeNode{ParseTreeNode::nodeSection,head->parent->parent};
                head->parent = sec;
                head = sec;
            }

            auto node = new ParseTreeNode{ParseTreeNode::nodeGreater,head};
            head->child.push_back(*node);
            head = node;
        }
        else if (c=='<'){
            auto node = new ParseTreeNode{ParseTreeNode::nodeLess,head};
            head->child.push_back(*node);
            head = node;
        }
        else if (c=='='){
            if (head->parent->type == ParseTreeNode::nodeGreater){
                head->parent->type = ParseTreeNode::nodeMinimum;
            }else if (head->parent->type == ParseTreeNode::nodeLess){
                head->parent->type = ParseTreeNode::nodeMaximum;
            }
        }
        else if (c=='['){
            auto node = new ParseTreeNode{ParseTreeNode::nodeOr,head};
            head->child.push_back(*node);
            head = node;
            node = new ParseTreeNode{ParseTreeNode::nodeOrItem,head};
            head->child.push_back(*node);
            head = node;
        }
        else if (c=='|'){
            head = head->parent;
            auto node = new ParseTreeNode{ParseTreeNode::nodeOrItem,head};
            head->child.push_back(*node);
            head = node;
        } else if (c==']'){
            head = head->parent->parent;
        }
        else if (c=='.'){
            head = head->parent;
        }
        else{
            if (head->type != ParseTreeNode::nodeV){
                auto node = new ParseTreeNode{ParseTreeNode::nodeV,head};
                head->child.push_back(*node);
                node->data += c;
            }
        }
    }
    return nullptr;
}

Result<> VersionExpression::compatible(Version version) {
    return base == version? Result<>::Ok() : Result<>::Err(1);
}
