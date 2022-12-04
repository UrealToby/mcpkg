//
// Created by toby on 2022/12/4.
//

#include "package.h"

Result<mcpkg::Dependencies*> mcpkg::Package::checkDependencies(const std::vector<Package>& packages) {
        for (auto depend: dependencies) {
            if (!depend->compatible(packages)){
                return Result<Dependencies*>{depend};
            }
        }
    return Result<Dependencies*>{};
}
