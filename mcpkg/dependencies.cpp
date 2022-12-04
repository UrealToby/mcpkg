//
// Created by toby on 2022/12/4.
//

#include "dependencies.h"

#include <algorithm>


mcpkg::CompatibleResult mcpkg::PackageDependencies::compatible(const std::vector<Package> &packages) {
    if(!conflict){
        for (auto package: packages) {
            if (packageId != package.id) {
                continue;
            } else if (!version->compatible(package.version)) {
                // 存在依赖的包，但是版本不满足
                lastResult = mcpkg::CompatibleResult{mcpkg::CompatibleResult::ERR, &package,  1};
                return lastResult;
            } else {
                // 满足依赖
                lastResult= mcpkg::CompatibleResult{mcpkg::CompatibleResult::OK};
                return lastResult;
            }
        }
        return mcpkg::CompatibleResult{.state=mcpkg::CompatibleResult::ERR, .errCode=2};
    }
    for (auto package: packages) {
        if (packageId != package.id or !version->compatible(package.version)) {
            continue;
        } else {
            // 存在冲突的包
            lastResult = mcpkg::CompatibleResult{mcpkg::CompatibleResult::ERR,&package,3};
            return lastResult;
        }
    }
    // 不存在冲突
    lastResult = mcpkg::CompatibleResult{.state=mcpkg::CompatibleResult::OK};
    return lastResult;
}

mcpkg::CompatibleResult mcpkg::TagConflictDependencies::compatible(const std::vector<Package> &packages) {
    for (auto package: packages) {
            if(std::find(package.tags.begin(), package.tags.end(), tag) != package.tags.end()){
                // 存在冲突的标签
                lastResult = mcpkg::CompatibleResult{mcpkg::CompatibleResult::ERR,&package, 1};
                return lastResult;
            }
    }
    lastResult = mcpkg::CompatibleResult{mcpkg::CompatibleResult::OK};
    return lastResult;
}
