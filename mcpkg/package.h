//
// Created by toby on 2022/12/4.
//

#ifndef MCPKG_PACKAGE_H
#define MCPKG_PACKAGE_H

#include <string>
#include <vector>

#include "dependencies.h"
#include "utils/version.h"
#include "utils/result.h"
namespace mcpkg{
    class Dependencies;

    class Package {
    public:
        std::string name;
        std::string id;
        std::string describe;
        std::vector<Dependencies*> dependencies;

        Version version;

        Result<Dependencies*> checkDependencies(const std::vector<Package>&);

    };

    /// 静态文件，比如 Mod,材质,光影,只需要根据配置文件放入指定文件夹
    /// 不存储文件，只存储配置
    class StaticPackage : public Package{
        // 是否解压
        bool extract = false;
        // todo: 存放路径
    };

    /// 需要执行的文件，比如ModLoader,Optifine
    /// 不存储被执行文件，只存储执行配置
    class ExecutablePackage: public Package{
        // 是否需要通过 java
        bool byJava{};
        //todo: 执行参数
        //todo: 存放路径
    };
    /// 需要执行脚本文件，比如，整合包
    /// 不存储脚本文件，只存储脚本配置，其中包含脚本地址、执行配置、其他资源文件
    class ScriptRequiredPackage : public Package{
        std::string scriptPath;
    };
}


#endif //MCPKG_PACKAGE_H
