//
// Created by toby on 2022/12/4.
//

#include "dependencies.h"

#include <algorithm>
#include <tuple>

mcpkg::CompatibleResult mcpkg::PackageDependencies::compatible(const std::vector<Package> &packages) {
    if(!conflict){
        for (const auto &package: packages) {
            if (packageId != package.id) {
                continue;
            } else if (!version->compatible(package.version)) {
                // 存在依赖的包，但是版本不满足
                return mcpkg::CompatibleResult{mcpkg::CompatibleResult::ERR, package,  1};
            } else {
                // 满足依赖
                return mcpkg::CompatibleResult{mcpkg::CompatibleResult::OK};
            }
        }
        return mcpkg::CompatibleResult{.state=mcpkg::CompatibleResult::ERR, .errCode=2};
    }
    for (const auto &package: packages) {
        if (packageId != package.id) {
            continue;
        } else if (!version->compatible(package.version)) {
            continue;
        } else {
            // 存在冲突的包
            return mcpkg::CompatibleResult{mcpkg::CompatibleResult::ERR,package,3};
        }
    }
    // 不存在冲突
    return mcpkg::CompatibleResult{.state=mcpkg::CompatibleResult::OK};
}

mcpkg::CompatibleResult mcpkg::TagConflictDependencies::compatible(const std::vector<Package> &packages) {
    for (const auto &package: packages) {
            if(std::find(package.tags.begin(), package.tags.end(), tag) != package.tags.end()){
                // 存在冲突的标签
                return mcpkg::CompatibleResult{mcpkg::CompatibleResult::ERR,package, 1};
            }
    }
    return mcpkg::CompatibleResult{mcpkg::CompatibleResult::OK};
}
