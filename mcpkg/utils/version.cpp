//
// Created by toby on 2022/12/5.
//

#include "version.h"
#include "cmath"

Result<> VersionExpressionGreaterOrLess::compatible(Version version) {
    unsigned long section_counts = std::max(base.data.size(),version.data.size());

    for (int i = 0; i < section_counts; ++i) {
        if (base.data[i] == version.data[i]){
            continue;
        }else{
            int base_section = std::stoi(base.data[i]);
            int version_section = std::stoi(version.data[i]);
            if ((base_section > version_section) == greater){
                return Result<>{Result<>::OK};
            }else {
                return Result<>{Result<>::ERR,"Inadequate",1};
            }
        }
    }
    return close ? Result<>{Result<>::OK} : Result<>{Result<>::ERR,"Inadequate",2};
}

class ParseTreeNode{
public:
    enum {nodeRoot,nodeOr,nodeOrItem,nodeMinimum,nodeMaximum,nodeGreater,nodeLess,nodeSection,nodeV}type;
    ParseTreeNode * parent;
    std::vector<ParseTreeNode> child;
    std::string data;
};

VersionExpression *VersionExpression::from_string(std::string str) {
    ParseTreeNode root = ParseTreeNode{ParseTreeNode::nodeRoot, nullptr};
    ParseTreeNode* head=&root;

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
