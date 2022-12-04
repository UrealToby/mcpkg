//
// Created by toby on 2022/12/5.
//

#ifndef MCPKG_RESULT_H
#define MCPKG_RESULT_H
#include <string>

template <class RETURN_T=std::string>
class Result{
public:
    RETURN_T data{};
    enum {OK,ERR} state;
    int errCode = 0;

    explicit operator bool(){
        return state == OK;
    }
};


#endif //MCPKG_RESULT_H
