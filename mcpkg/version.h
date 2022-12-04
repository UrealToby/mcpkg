//
// Created by toby on 2022/12/5.
//

#ifndef MCPKG_VERSION_H
#define MCPKG_VERSION_H

#include <string>
#include <vector>

#include "result.h"

class Version {
public:
    std::vector<std::string> data;

};


class VersionExpression{
public:
    std::string expression;
    Version base;
    // todo: 将 string 转为 版本表达式
    VersionExpression* from_string(std::string);
    virtual Result<> compatible(Version) = 0;
};

class VersionExpressionGreaterOrLess : public VersionExpression{
    /// 大于还是小于
    bool greater{};
    /// 是否为开区间
    bool close{};

    // todo: 判断是否满足
    Result<> compatible(Version) override;
};

class VersionExpressionSection : public VersionExpression{
    VersionExpressionGreaterOrLess left;
    VersionExpressionGreaterOrLess right;

    // todo: 判断是否满足
    Result<> compatible(Version) override;
};

class VersionExpressionOr : public VersionExpression{
    std::vector<VersionExpression> expressions;

    Result<> compatible(Version) override;
};


#endif //MCPKG_VERSION_H
