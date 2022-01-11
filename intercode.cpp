//
// Created by lelouch on 2021/12/24.
//

#include "intercode.h"
#include "symbol.h"
#include "genir.h"
/**
 * 单个指令初始化
 */
void InterInst::init() {

    op=OP_NOP;
    this->result=NULL;
    this->target=NULL;
    this->arg1=NULL;
    this->fun=NULL;
    this->arg2=NULL;
    first=false;
    isDead=false;
}

/*
	一般运算指令
*/
InterInst::InterInst (Operator op,Var *rs,Var *arg1,Var *arg2)
{
    init();
    this->op=op;
    this->result=rs;
    this->arg1=arg1;
    this->arg2=arg2;
}

/**
 *
 * @param op 操作符
 * @param fun 符号表中的函数
 * @param rs
 */
InterInst::InterInst (Operator op,Fun *fun,Var *rs){//函数调用指令,ENTRY,EXIT
    init();
    this->op = op;
    this->result = rs;
    this->fun = fun;
    this->arg2 = NULL;
}

/**
 * 参数入栈指令
 * @param op push
 * @param arg1 入栈的参数
 */
InterInst::InterInst(Operator op, Var *arg1) {
    init();
    this->op=op;
    this->result=NULL;
    this->arg1=arg1;
    this->arg2=NULL;
}

/**
 * 产生唯一标号
 */
InterInst::InterInst() {
    init();
    label=GenIR::genLb();
}

/**
*	条件跳转指令
*/
InterInst::InterInst (Operator op,InterInst *tar,Var *arg1,Var *arg2)
{
    init();
    this->op=op;
    this->target=tar;
    this->arg1=arg1;
    this->arg2=arg2;
}

/**
	替换表达式指令信息
    与上面的生成表达式不同点在于，替换表达式没有对新的指令标号做初始化
*/
void InterInst::replace(Operator op,Var *rs,Var *arg1,Var *arg2)
{
    this->op=op;
    this->result=rs;
    this->arg1=arg1;
    this->arg2=arg2;
}


/**
	替换跳转指令信息，条件跳转优化
*/
void InterInst::replace(Operator op,InterInst *tar,Var *arg1,Var *arg2)
{
    this->op=op;
    this->target=tar;
    this->arg1=arg1;
    this->arg2=arg2;
}

/**
	替换操作符，用于将CALL转化为PROC
*/
void InterInst::callToProc()
{
    this->result=NULL;//清除返回值
    this->op=OP_PROC;
}


/*
	清理常量内存
*/
InterInst::~InterInst()
{
    //if(arg1&&arg1->isLiteral())delete arg1;
    //if(arg2&&arg2->isLiteral())delete arg2;
}


/*******************************************************************************
                                   中间代码
*******************************************************************************/


/*
	添加中间代码
*/
void InterCode::addInst(InterInst*inst)
{
    code.push_back(inst);
}

/*
	输出指令信息
*/
void InterCode::toString()
{
    for(int i=0;i<code.size();i++)
    {
        code[i]->toString();
    }
}

/**
 * 释放内存
 */
InterCode::~InterCode() {
    for(int i=0;i<code.size();i++)
    {
        delete code[i];
    }
}


/*
	获取中间代码序列
*/
vector<InterInst*>& InterCode::getCode()
{
    return code;
}


