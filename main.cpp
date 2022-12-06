#include <iostream>
#include "mcpkg/utils/version.h"

int main()
{
   Version version1 = Version("1.19.2");
    Version version2 = Version("1.16.5");
    Version version3 = Version("1.12");
    Version version4 = Version("1.8.1");
//
    VersionExpression* versionExpression = VersionExpression::from_string("[>1.12<1.19|1.8.*]");

    auto result = versionExpression->compatible(version1);
    std::cout<<result.data<<"("<<result.state<<")"<<std::endl;
    result = versionExpression->compatible(version2);
    std::cout<<result.data<<"("<<result.state<<")"<<std::endl;
    result = versionExpression->compatible(version3);
    std::cout<<result.data<<"("<<result.state<<")"<<std::endl;
    result = versionExpression->compatible(version4);
    std::cout<<result.data<<"("<<result.state<<")"<<std::endl;
}
