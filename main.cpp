#include <iostream>
#include "mcpkg/utils/version.h"
using namespace mcpkg;
int main()
{
   auto version1 = Version("1.19.2");
   auto  version2 = Version("1.16.5");
   auto  version3 = Version("1.12");
   auto  version4 = Version("1.8.1");
//
    for (int i = 0; i < 10000; ++i) {
        VersionExpression* versionExpression = VersionExpression::from_string("[>1.12<1.19|1.8.*]");
        auto result = versionExpression->compatible(version1);
        result = versionExpression->compatible(version2);
        result = versionExpression->compatible(version3);
        result = versionExpression->compatible(version4);
    }

    std::cout<<"end";
}
