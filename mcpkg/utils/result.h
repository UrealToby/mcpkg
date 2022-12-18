//
// Created by toby on 2022/12/5.
//

#ifndef MCPKG_RESULT_H
#define MCPKG_RESULT_H
#include <string>
#include <functional>

template <class RETURN_T=std::string>
class Result{
public:
    enum {OK,ERR} state;
    RETURN_T data{};
    int errCode = 0;

    inline explicit operator bool(){
        return state == OK;
    }
    inline static Result Ok(const RETURN_T &_data=RETURN_T{}){
     return Result{OK, _data, 0};
    }

    inline static Result Err(int _code,const RETURN_T &_data=RETURN_T{}){
        return Result{ERR, _data, _code};
    }

    inline Result<RETURN_T>* onErr(std::function<void(Result<RETURN_T>*)> call){
        if (state!=OK){
            call(this);
        }

        return this;
    }
    inline  Result<RETURN_T>* onOk(std::function<void(Result<RETURN_T>*)> call){
        if (state==OK){
            call(this);
        }

        return this;
    }
};


#endif //MCPKG_RESULT_H
