//
// Created by toby on 2022/12/5.
//

#ifndef MCPKG_VERSION_H
#define MCPKG_VERSION_H

#include <string>
#include <vector>

#include "result.h"

namespace mcpkg {
    class Version {
    public:
        std::vector<std::string> data{};

        [[nodiscard]] inline int size() const;

        Version() = default;

        explicit Version(const std::string &);

        /**
         * 比较两个版本
         * @param greater @code true @endcode 为大于，@endcode false 为小于
         * @param close 是否允许等于
         * @return @code .state=0  为满足条件，@code .state=1为不满足条件，@code .state=2为无法转换字符串到数字进行比较大小
         */
        static Result<> comparison(Version left, Version right, bool greater, bool close);

        inline bool operator==(const Version &version) const;

        bool operator>=(const Version &version) const;

        bool operator<=(const Version &version) const;

        bool operator>(const Version &version) const;

        bool operator<(const Version &version) const;
    };

    class Comparable {
    public:
        /**
         * 比较给定版本是否满足要求
         * @return
         */
        virtual Result<> compatible(const Version &) = 0;

        virtual ~Comparable() = default;
    };

    /**
     * 纯版本比较，例如 1.12 或 1.12.*
     */
    class VersionCompareEqual : public Comparable {
    public:
        Version base;

        Result<> compatible(const Version &version) override;
    };

    /**
    * 大小比较，例如 ^1.12 或 <1.12
    */
    class VersionCompareGreaterOrLess : public Comparable {
    public:
        Version base;
        /// 大于还是小于
        bool greater = true;
        /// 是否为开区间
        bool close = true;

        Result<> compatible(const Version &) override;
    };

    /**
    * 比较版本是否在范围内，例如 ^1.12<1.16.5
    */
    class VersionCompareRange : public Comparable {
    public:
        VersionCompareGreaterOrLess left;
        VersionCompareGreaterOrLess right;

        Result<> compatible(const Version &) override;
    };

    /**
     * 与一些列表达式进行比较，例如 [1.8.*,1.9,*,>1.16.5]
     */
    class VersionCompareOr : public Comparable {
    public:
        std::vector<Comparable *> expressions;

        Result<> compatible(const Version &) override;

        ~VersionCompareOr() override;
    };

    static Comparable *from_string(const std::string &);

    class VersionExpression {
    public:
        Comparable *comparable;
        /**
         * 构建版本表达式
         */
        explicit VersionExpression(const std::string &);
        /**
         * 判断版本是否符合表达式
         * @return
         */
        Result<> compatible(const Version &);

        ~VersionExpression();
    };
}

#endif //MCPKG_VERSION_H
