//
// Created by toby on 2022/12/5.
//

#ifndef MCPKG_RESULT_H
#define MCPKG_RESULT_H
#include <string>

template <class RETURN_T=std::string>
class Result{
public:
    enum {OK,ERR} state;
    RETURN_T data{};
    int errCode = 0;

    explicit operator bool(){
        return state == OK;
    }
    static Result Ok(RETURN_T _data=RETURN_T{}){
     return Result{OK, _data, 0};
    }

    static Result Err(int _code,RETURN_T _data=RETURN_T{}){
        return Result{ERR, _data, _code};
    }
};


#endif //MCPKG_RESULT_H
