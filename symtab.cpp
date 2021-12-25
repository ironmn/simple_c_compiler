//
// Created by lelouch on 2021/12/14.
//
#include "symbol.h"
#include "symtab.h"
#include "error.h"

//打印语义错误
#define SEMERROR(code,name) Error::semError(code,name)

/*******************************************************************************
                                   符号表
*******************************************************************************/


Var* SymTab::voidVar=NULL;//特殊变量的初始化
Var* SymTab::zero=NULL;//特殊变量的初始化
Var* SymTab::one=NULL;//特殊变量的初始化
Var* SymTab::four=NULL;//特殊变量的初始化

/**
 *初始化符号表
 */
SymTab::SymTab() {
    voidVar = new Var();
    zero=new Var(1);
    one = new Var(1);
    four = new Var(4);//常量4
    addVar(voidVar);
    addVar(one);
    addVar(zero);
    addVar(four);

    scopeId = 0;
    curFun = NULL;
    //ir = NULL;
    scopePath.push_back(0);
}

SymTab::~SymTab() {
    //首先释放函数表的所有内存空间
    hash_map<string ,Fun*,string_hash> ::iterator funIt,funEnd = funTab.end();
    for(funIt = funTab.begin();funIt != funEnd;++funIt){
        delete funIt -> second;
    }
    //释放变量表的所有信息
    hash_map<string,vector<Var*> *,string_hash>::iterator  varIt,varEnd = varTab.end();
    for(varIt = varTab.begin();varIt != varEnd; ++ varIt){
        vector<Var*> &list = *varIt->second;
        for(int i = 0;i<list.size();i++){//避免出现野指针
            delete list[i];
        }
        delete &list;
    }
    //释放串表的内存空间
    hash_map<string,Var*,string_hash> ::iterator strIt,strEnd = strTab.end();
    for(strIt = strTab.begin();strIt != strEnd;++strEnd){
        delete strIt->second;
    }
}


//添加一个新的变量
void SymTab::addVar(Var *var) {
    //在变量表中没有找到v这个变量
    if(varTab.find(var->getName()) == varTab.end()){
        varTab[var->getName()] = new vector<Var*>;
        varTab[var->getName()]->push_back(var);
        varList.push_back(var->getName());
    }
    else{//在变量列表中找到了这个变量，说明产生了重定义
        vector<Var* > &list = *varTab[var->getName()];
        int i;
        for(i = 0;i<list.size();i++){
            //如果作用域重定义
            if(list[i]->getPath().back() == var->getPath().back()){
                break;
            }
        }
        if(i == list.size() || var->getName()[0] == '<'){//作用域没有冲突或者var为常量
            list.push_back(var);
        }
        else{//否则报告重定义的语义错误
            SEMERROR(VAR_RE_DEF,var->getName());
            delete var;
            return;
        }
    }
//    if(ir){
//        int flag=ir->genVarInit(var);//产生变量初始化语句,常量返回0
//        if(curFun&&flag)curFun->locate(var);//计算局部变量的栈帧偏移
//    }
}

//添加一个字符串常量
void SymTab::addStr(Var *v) {
    strTab[v->getName()] = v;
}

//当使用一个声明时，根据最近原则， 寻找作用域路径最长匹配的那个变量
Var* SymTab::getVar(string name){//获取一个变量
    Var* select = NULL;//最佳匹配
    if(varTab.find(name) != varTab.end()){
        vector<Var*> &list = *varTab[name];
        int pathLen = scopePath.size();
        int maxLen = 0;
        for(int i = 0;i<list.size();i++){
            int len = list[i]->getPath().size();
            if(len <= pathLen && list[i]->getPath()[len-1] == scopePath[pathLen - 1]){
                maxLen = len;
                select = list[i];
            }
        }
    }
    if(!select) SEMERROR(VAR_UN_DEC,name);//变量未声明
    return select;
}