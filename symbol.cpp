//
// Created by lelouch on 2021/12/14.
//

#include "symbol.h"
#include "token.h"
#include "error.h"
#include "symbol.h"

string Var::getName() {
    return this->name;
}

//获取作用域路径
vector<int>& Var::getPath() {
    return scopePath;
}