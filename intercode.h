//
// Created by lelouch on 2021/12/24.
//

#ifndef SIMPLE_C_COMPILER_INTERCODE_H
#define SIMPLE_C_COMPILER_INTERCODE_H
#include "common.h"

/*
 * 四元式类，定义了中间代码的指定格式
 * */
class InterInst {
private:
    string label;//标号
    Operator op;//操作符

    Var *result;//运算结果
    InterInst* target;//跳转标号

    Var* arg1;//参数1
    Fun* fun;//函数
    Var* arg2;//参数2

    bool first;//是否是首指令
    void init();//初始化
};


#endif //SIMPLE_C_COMPILER_INTERCODE_H
