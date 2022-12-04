//
// Created by toby on 2022/12/4.
//

#include "dependencies.h"

Result<> mcpkg::PackageDependencies::compatible(const std::vector<Package>& packages) {
    for (const auto& package: packages) {
        if (packageId != package.id){
            continue;
        }
        else if (!version->compatible(package.version)){
            return Result<>{"There are dependent packages, but the version does not conform to",Result<>::ERR,1};
        } else{
            return Result<>{"Package satisfies dependency"};
        }
    }
    return Result<>{"Missing dependent packages",Result<>::ERR,2};
}
