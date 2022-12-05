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

    inline int size() const;

    static Result<> comparison(Version left, Version right, bool greater, bool close);
    inline bool operator ==(const Version& version) const;
    bool operator >=(const Version& version) const;
    bool operator <=(const Version& version) const;
    bool operator >(const Version& version) const;
    bool operator <(const Version& version) const;
};


class VersionExpression{
public:
    std::string expression;
    Version base;
    // todo: 将 string 转为 版本表达式
    static VersionExpression* from_string(const std::string&);
    virtual Result<> compatible(Version);
};

class VersionExpressionGreaterOrLess : public VersionExpression{
    /// 大于还是小于
    bool greater{};
    /// 是否为开区间
    bool close = true;

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

    // todo: 判断是否满足
    Result<> compatible(Version) override;
};


#endif //MCPKG_VERSION_H
