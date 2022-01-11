//
// Created by lelouch on 2021/12/14.
//

#ifndef SIMPLE_C_COMPILER_SYMTAB_H
#define SIMPLE_C_COMPILER_SYMTAB_H
#pragma once
#include "common.h"
#include "symbol.h"
#include "genir.h"
#include <ext/hash_map>
using namespace __gnu_cxx;


class SymTab{

    //声明顺序记录
    vector<string>varList;//记录变量的添加顺序
    vector<string>funList;//记录函数的添加顺序
    //hash函数
    struct string_hash{
        size_t operator()(const string& str) const{
            return __stl_hash_string(str.c_str());
        }
    };

    hash_map<string,vector<Var*> *,string_hash> varTab;
    hash_map<string,Var*, string_hash> strTab;
    hash_map<string,Fun*,string_hash> funTab;
    //辅助分析数据
    Fun *curFun;//当前的函数栈帧
    int scopeId;//作用域唯一的编号
    vector<int> scopePath;//动态记录作用域的路径

    //中间代码生成器
    GenIR* ir;

public:
    static Var* voidVar;//特殊变量
    static Var* zero;//特殊变量
    static Var* one;//特殊变量
    static Var* four;//特殊变量

    SymTab();//初始化符号表
    ~SymTab();//清除内存

    //作用域管理
    void enter();//进入新的作用域
    void leave();//离开作用域

    //变量管理
    void addVar(Var* v);//添加一个变量
    void addStr(Var* v);//添加一个字符串常量
    Var* getVar(string name);//获取一个变量
    vector<Var*> getGlbVars();//获取所有全局变量

    //函数管理
    void decFun(Fun *fun);//函数声明控制
    void defFun(Fun* fun);//函数定义控制
    void endDefFun(Fun* fun);//结束函数定义
    Fun* getFun(string name,vector<Var*>& args);//根据调用类型，获取一个函数
    void addInst(InterInst*inst);//添加一条中间代码

    //外部调用接口
    void setIr(GenIR*ir);//设置中间代码生成器
    vector<int>& getScopePath();//获取scopePath
    Fun*getCurFun();//获取当前分析的函数
    void toString();//输出信息
    void printInterCode();//输出中间指令
    void optimize();//执行优化操作
    void printOptCode();//输出中间指令
    void genData(FILE*file);//输出数据
    void genAsm(char*fileName);//输出汇编文件

};



#endif //SIMPLE_C_COMPILER_SYMTAB_H
