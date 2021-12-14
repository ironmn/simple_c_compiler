//
// Created by lelouch on 2021/12/14.
//

#ifndef SIMPLE_C_COMPILER_SYMTAB_H
#define SIMPLE_C_COMPILER_SYMTAB_H
#pragma once
#include "common.h"
#include "symbol.h"
#include <ext/hash_map>
using namespace __gnu_cxx;


class SymTab{
    //hash函数
    struct string_hash{
        size_t operator()(const string& str) const{
            return __stl_hash_string(str.c_str());
        }
    };

    hash_map<string,vector<Var*> *,string_hash> varTab;
    hash_map<string,Var*, string_hash> strTab;
    hash_map<string,Fun*,string_hash> funTab;
};



#endif //SIMPLE_C_COMPILER_SYMTAB_H
