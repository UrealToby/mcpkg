#include <iostream>
#include "mcpkg/utils/version.h"
using namespace mcpkg;
int main()
{
   auto version1 = Version("1.19.2");
   auto  version2 = Version("1.16.5");
   auto  version3 = Version("1.12");
   auto  version4 = Version("1.8.1");

    VersionExpression expression = VersionExpression("[^1.12<1.19|1.8.*]");
    auto result = expression.compatible(version1);
    result = expression.compatible(version2);
    result = expression.compatible(version3);
    result = expression.compatible(version4);
    result = expression.compatible(version4);
}
