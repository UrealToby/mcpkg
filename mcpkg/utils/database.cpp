//
// Created by toby on 2022/12/17.
//

#include "database.h"

Result<> Database::open(const std::string& path) {
    if(sqlite3_open(path.c_str(),&db)){
        return Result<>::Err(1,"cannot open database.");
    }
    return Result<>::Ok();
}

Result<> Database::close() const {
    if(sqlite3_close(db)){
        return Result<>::Err(1,"cannot close database.");
    }
    return Result<>::Ok();
}

Result<DatabaseExecResult> Database::exec(const std::string& cmd) const {
    char *err_msg;
    DatabaseExecResult execResult;

    auto state = sqlite3_exec(db, cmd.c_str(), execCallBack, &execResult, &err_msg);
    if(state){
        return Result<DatabaseExecResult>::Err(state,DatabaseExecResult::Err(err_msg));
    }
    DatabaseExecResult result;

    return Result<DatabaseExecResult>::Ok(execResult);
}
#include <iostream>
int Database::execCallBack(void *param, int argc,char **values, char **names) {
    auto* result = static_cast<DatabaseExecResult *>(param);

    std::map<std::string ,std::string > data;

    for (int i = 0; i < argc; ++i) {
        data[names[i]] = values[i];
    }
    result->data.push_back(data);

    return 0;
}
