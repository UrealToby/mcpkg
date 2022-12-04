//
// Created by toby on 2022/12/4.
//

#ifndef MCPKG_DEPENDENCIES_H
#define MCPKG_DEPENDENCIES_H

#include <string>
#include "utils/result.h"
#include "package.h"
#include "utils/version.h"

namespace mcpkg{
    class Package;

    enum DependenciesType{
        kConflict,
        kDepend
    };

    class Dependencies {
    public:
        DependenciesType type;
        virtual Result<> compatible(const std::vector<Package>&) = 0;
    };

    class PackageDependencies : public Dependencies{
    public:
        /// 类型::id
        /// Examples:
        ///     "mod::forge::jei" "mod_loader::forge" "minecraft"
        std::string packageId;

        /// Examples:
        ///     等于: \c "1.19.2"
        ///     任意版本: \c "1.7.*"
        ///     大于: \c ">1.6"
        ///     大于等于且小于: \c ">=1.6 <1.7"
        ///     最小版本: \c "^1.15"
        ///     或: \c "1.5|1.19"
        ///
        VersionExpression* version;
        Result<> compatible(const std::vector<Package>&) override;
    };

}

#endif //MCPKG_DEPENDENCIES_H
