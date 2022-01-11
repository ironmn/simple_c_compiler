//
// Created by lelouch on 2021/12/31.
//

#include "genir.h"
#include <sstream>
#include "symbol.h"
#include "symtab.h"
#include "error.h"
//打印语义错误
#define SEMERROR(code) Error::semError(code)



/*
	初始化
*/
GenIR::GenIR(SymTab&tab):symtab(tab)
{
    symtab.setIr(this);//构建符号表与代码生成器的一一关系
    lbNum=0;
    push(NULL,NULL);//初始化作用域
}

/**
	获取唯一名字的标签
*/
string GenIR::genLb()
{
    //流操作的IO速度比起普通的字符串拼接要快
    lbNum++;
    string lb=".L";//为了和汇编保持一致！
    stringstream ss;
    ss<<lbNum;
    return lb+ss.str();
}


/**
 * @details 双目运算表达式生成
 * @param lval
 * @param opt
 * @param rval
 * @return
 */
Var* GenIR::genTwoOp(Var*lval,Tag opt,Var*rval)
{
    if(!lval || !rval)return NULL;
    if(lval->isVoid()||rval->isVoid()){
        SEMERROR(EXPR_IS_VOID);//void函数返回值不能出现在表达式中
        return NULL;
    }
    //赋值单独处理
    if(opt==ASSIGN)return genAssign(lval,rval);//赋值
    //先处理(*p)变量
    if(lval->isRef())lval=genAssign(lval);
    if(rval->isRef())rval=genAssign(rval);
    if(opt==OR)return genOr(lval,rval);//或
    if(opt==AND)return genAnd(lval,rval);//与
    if(opt==EQU)return genEqu(lval,rval);//等于
    if(opt==NEQU)return genNequ(lval,rval);//不等于
    if(opt==ADD)return genAdd(lval,rval);//加
    if(opt==SUB)return genSub(lval,rval);//减
    if(!lval->isBase() || !rval->isBase())
    {
        SEMERROR(EXPR_NOT_BASE);//不是基本类型
        return lval;
    }
    if(opt==GT)return genGt(lval,rval);//大于
    if(opt==GE)return genGe(lval,rval);//大于等于
    if(opt==LT)return genLt(lval,rval);//小于
    if(opt==LE)return genLe(lval,rval);//小于等于
    if(opt==MUL)return genMul(lval,rval);//乘
    if(opt==DIV)return genDiv(lval,rval);//除
    if(opt==MOD)return genMod(lval,rval);//取模
    return lval;
}


Var* GenIR::genAssign(Var *lval, Var *rval) {
    if(!lval->getLeft()){//左参数不能当做左值
        SEMERROR(EXPR_NOT_LEFT_VAL);//产生左值错误
        return rval;//直接返回右值
    }
    //类型检查
    if(!typeCheck(lval,rval)){
        SEMERROR(ASSIGN_TYPE_ERR);
        return rval;
    }
    //若右值为引用类型
    if(rval -> isRef()){
        if(!lval->isRef()){
            symtab.addIn
        }
    }
}

/**
 * 类型检查，查看左右两值是否可以互相转换
 * @param lval
 * @param rval
 * @return
 */
bool GenIR::typeCheck(Var *lval, Var *rval) {
    bool flag = false;
    if(!rval) return false;//右值为空则返回false
    if(lval->isBase() && rval->isBase()){//基本类型之间可以进行互相转换
        flag = true;
    }
    else if(!lval->isBase() && !rval->isBase()){//如果都不是基本类型
        flag = lval->getType() == rval->getType();//判断二者的类型是否相同
    }
    return flag;//其他类型的不同类型数据默认不能进行互相转换（为了方便构建编译器）
}
