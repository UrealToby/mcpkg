//
// Created by toby on 2022/12/5.
//

#ifndef MCPKG_VERSION_H
#define MCPKG_VERSION_H

#include <string>
#include <vector>

#include "result.h"

namespace mcpkg{
    class Version {
    public:
        std::vector<std::string> data{};

        [[nodiscard]] inline int size() const;
        Version()= default;
        explicit Version(std::string);

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

        // 将字符串构建为版本表达式
        static VersionExpression* from_string(const std::string&);
        virtual Result<> compatible(const Version&);
    };

    class VersionExpressionGreaterOrLess : public VersionExpression{
    public:
        /// 大于还是小于
        bool greater = true;
        /// 是否为开区间
        bool close = true;

        Result<> compatible(const Version&) override;
    };

    class VersionExpressionRange : public VersionExpression{
    public:
        VersionExpressionGreaterOrLess* left;
        VersionExpressionGreaterOrLess* right;

        ~VersionExpressionRange();
        Result<> compatible(const Version&) override;
    };

    class VersionExpressionOr : public VersionExpression{
    public:
        std::vector<VersionExpression*> expressions;

        Result<> compatible(const Version&) override;
        ~VersionExpressionOr();
    };
}



#endif //MCPKG_VERSION_H
