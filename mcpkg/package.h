//
// Created by toby on 2022/12/4.
//

#ifndef MCPKG_PACKAGE_H
#define MCPKG_PACKAGE_H

#include <string>
#include <vector>
#include <map>

#include "dependencies.h"
#include "utils/version.h"
#include "utils/result.h"

namespace mcpkg{
    class Dependencies;

    class Path{
        std::string basic;
        /**
         * JavaHome/JavaPath
         * MinecraftPath: 一般来说是.minecraft/文件夹，如果开启了版本隔离，则是.minecraft/versions/[version]/
         * MinecraftVersion: 我的世界版本
         * MinecraftBin: 我的世界jar路径
         * ModPath: [MinecraftPath]/mods/ 默认，可覆盖
         * ShaderPath: [MinecraftPath]/shaders/ 默认，可覆盖
         * SavePath: [MinecraftPath]/saves/ 默认，可覆盖
         * LibPath: [MinecraftPath]/libraries/ 默认，可覆盖
         * ConfigPath: [MinecraftPath]/libraries/config/ 默认，可覆盖
         * TempPath: 临时路径，安装结束后销毁。
         * UserPath: 用户路径
         * @return
         */
        Result<std::string> toAbsolute(std::map<std::string,std::string>);
    };


    class Package {
    public:
        struct FileInfo{
            std::string name;
            std::string downloadPath;
            std::string md5;
            size_t size;
        } fileInfo;

        struct PackageInfo{
            std::string name;
            Version version;
            std::string license;
            std::string description;
            std::string author;
            std::string author_site;
            std::string site;
        };
        std::string name;
        std::string localName;
        std::string id;
        std::string describe;
        std::vector<Dependencies*> dependencies;

        std::vector<std::string > tags;

        Version version;

        Result<Dependencies*> checkDependencies(const std::vector<Package>&);

    };

    /// 静态文件，比如 Mod,材质,光影,只需要根据配置文件放入指定文件夹
    /// 不存储文件，只存储配置

    class StaticPackage : public Package{
        // 是否解压
        enum {
            noExtract,
            Extract,
            ExtractToNewFolder,
        }extract = noExtract;
        Path path;
    };

    /// 需要执行的文件，比如ModLoader,Optifine
    /// 不存储被执行文件，只存储执行配置
    class ExecutablePackage: public Package{
        // 是否需要通过 java
        bool byJava{};
        std::vector<std::string> params;
        Path path;
    };
    /// 需要执行脚本文件，比如，整合包
    /// 不存储脚本文件，只存储脚本配置，其中包含脚本地址、执行配置、其他资源文件
    class ScriptRequiredPackage : public Package{
        Path scriptPath;
    };
}


#endif //MCPKG_PACKAGE_H
