//
// Created by toby on 2022/12/17.
//

#ifndef MCPKG_DATABASE_H
#define MCPKG_DATABASE_H
#include <string>
#include <functional>
#include <sqlite3.h>
#include <map>
#include <list>

#include "result.h"

class DatabaseExecResult{
public:
    std::list<std::map<std::string,std::string>> data;
    std::string errMsg;

public:
    static DatabaseExecResult Err(std::string msg){
        DatabaseExecResult result;
        result.errMsg = msg;
        return result;
    }
};

class Database {
public:
    sqlite3* db;
    Result<> open(const std::string& path);
    Result<> close() const;
    Result<DatabaseExecResult> exec(const std::string& cmd) const;
private:
    static int execCallBack(void *param, int f_num, char **f_value, char **f_name);
};


#endif //MCPKG_DATABASE_H
