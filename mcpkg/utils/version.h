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
    Version base;

    static VersionExpression* from_string(const std::string&);
    virtual Result<> compatible(const Version&);
};

class VersionExpressionGreaterOrLess : public VersionExpression{
public:
    /// 大于还是小于
    bool greater{};
    /// 是否为开区间
    bool close = true;

    Result<> compatible(const Version&) override;
};

class VersionExpressionRange : public VersionExpression{
    VersionExpressionGreaterOrLess left;
    VersionExpressionGreaterOrLess right;

    Result<> compatible(const Version&) override;
};

class VersionExpressionOr : public VersionExpression{
public:
    std::vector<VersionExpression*> expressions;

    Result<> compatible(const Version&) override;
};



#endif //MCPKG_VERSION_H
