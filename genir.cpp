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
 * @details 数组索引语句
 * @param array
 * @param index
 * @return
 */
Var* GenIR::genArray(Var *array, Var *index) {
    if(!array || !index)return NULL;
    if(array->isVoid() || index->isVoid()){
        SEMERROR(EXPR_IS_VOID);//void函数返回值不能出现在表达式中
        return NULL;
    }
    if(array->isBase() || !index->isBase()){
        SEMERROR(ARR_TYPE_ERR);
        return index;
    }
    return genPtr(genAdd(array,index));
}

/*
	实际参数传递
*/
void GenIR::genPara(Var* arg)
{
    /*
        注释部分为删除代码，参数入栈不能通过拷贝，只能push！
    */
    if(arg->isRef())arg=genAssign(arg);
    //无条件复制参数！！！传值，不传引用！！！
    //Var*newVar=new Var(symtab.getScopePath(),arg);//创建参数变量
    //symtab.addVar(newVar);//添加无效变量，占领栈帧！！
    InterInst* argInst=new InterInst(OP_ARG,arg);//push arg!!!
    //argInst->offset=newVar->getOffset();//将变量的地址与arg指令地址共享！！！没有优化地址也能用
    //argInst->path=symtab.getScopePath();//记录路径！！！为了寄存器分配时计算地址
    symtab.addInst(argInst);
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
            symtab.addInst(new InterInst(OP_GET,lval,rval->getPointer()));
            return lval;
        }
        else rval = genAssign(rval);
    }
    //赋值运算
    if(lval->isRef()){
        symtab.addInst(new InterInst(OP_SET,rval,lval->getPointer()));
    }
    else{//常量运算
        symtab.addInst(new InterInst(OP_AS,lval,rval));
    }
    return lval;
}


/**
 * 拷贝赋值语句
 * @param val
 * @return
 */
Var* GenIR::genAssign(Var *val) {
    Var* temp = new Var(symtab.getScopePath(),val);
    symtab.addVar(temp);
    if(val->isRef()){//如果val是引用类型
        symtab.addInst(new InterInst(OP_GET,temp,val->getPointer()));
    }
    else symtab.addInst(new InterInst(OP_AS,temp,val));
    return temp;
}

/**
 * 或运算
 * @param lval
 * @param rval
 * @return
 */
Var* GenIR::genOr(Var *lval, Var *rval) {
    Var* tmp = new Var(symtab.getScopePath(),KW_INT, false);
    symtab.addVar(tmp);
    symtab.addInst(new InterInst(OP_OR,tmp,lval,rval));
    return tmp;
}

/**
 * 与运算
 * @param lval
 * @param rval
 * @return
 */
Var* GenIR::genAnd(Var *lval, Var *rval) {
    Var* tmp = new Var(symtab.getScopePath(),KW_INT, false);
    symtab.addVar(tmp);
    symtab.addInst(new InterInst(OP_AND,tmp,lval,rval));
    return tmp;
}

/**
 * 大于语句
 * @param lval
 * @param rval
 * @return
 */
Var* GenIR::genGt(Var *lval, Var *rval) {
    Var* tmp = new Var(symtab.getScopePath(),KW_INT, false);
    symtab.addVar(tmp);
    symtab.addInst(new InterInst(OP_GT,tmp,lval,rval));
    return tmp;
}

/**
 * 大于等于语句
 * @param lval
 * @param rval
 * @return
 */
Var* GenIR::genGe(Var *lval, Var *rval) {
    Var* tmp = new Var(symtab.getScopePath(),KW_INT, false);
    symtab.addVar(tmp);
    symtab.addInst(new InterInst(OP_GE,tmp,lval,rval));
    return tmp;
}


/**
	小于语句
*/
Var* GenIR::genLt(Var*lval,Var*rval)
{
    Var*tmp=new Var(symtab.getScopePath(),KW_INT,false);//基本类型
    symtab.addVar(tmp);
    symtab.addInst(new InterInst(OP_LT,tmp,lval,rval));//中间代码tmp=lval<rval
    return tmp;
}

/**
	小于等于语句
*/
Var* GenIR::genLe(Var*lval,Var*rval)
{
    Var*tmp=new Var(symtab.getScopePath(),KW_INT,false);//基本类型
    symtab.addVar(tmp);
    symtab.addInst(new InterInst(OP_LE,tmp,lval,rval));//中间代码tmp=lval<=rval
    return tmp;
}

/**
	等于语句
*/
Var* GenIR::genEqu(Var*lval,Var*rval)
{
    Var*tmp=new Var(symtab.getScopePath(),KW_INT,false);//基本类型
    symtab.addVar(tmp);
    symtab.addInst(new InterInst(OP_EQU,tmp,lval,rval));//中间代码tmp=lval==rval
    return tmp;
}

/**
	不等于语句
*/
Var* GenIR::genNequ(Var*lval,Var*rval)
{
    Var*tmp=new Var(symtab.getScopePath(),KW_INT,false);//基本类型
    symtab.addVar(tmp);
    symtab.addInst(new InterInst(OP_NE,tmp,lval,rval));//中间代码tmp=lval!=rval
    return tmp;
}

/**
 * 加法语句
 * @param lval
 * @param rval
 * @return
 */
Var* GenIR::genAdd(Var *lval, Var *rval) {
    Var* tmp = NULL;
    if((lval->getArray()||lval->getPtr()) && rval->isBase()){
        tmp = new Var(symtab.getScopePath(),lval);
        rval = genMul(rval,Var::getStep(lval));
    }
    else if(rval->isBase() && (rval->getArray()||rval->getPtr())){
        tmp = new Var(symtab.getScopePath(),rval);
        lval = genMul(lval,Var::getStep(rval));
    }
    else if(lval->isBase() && rval->isBase()){
        tmp = new Var(symtab.getScopePath(),KW_INT, false);
    }
    else{
        SEMERROR(EXPR_NOT_BASE);
        return lval;
    }
    symtab.addVar(tmp);
    symtab.addInst(new InterInst(OP_ADD,tmp,lval,rval));
    return tmp;
}

/**
 * 减法语句
 * @param lval
 * @param rval
 * @return
 */
Var* GenIR::genSub(Var *lval, Var *rval) {
    Var* tmp = NULL;
    if(!rval->isBase())
    {
        SEMERROR(EXPR_NOT_BASE);//类型不兼容,减数不是基本类型
        return lval;
    }
    //指针和数组
    if((lval->getArray()||lval->getPtr())){
        tmp=new Var(symtab.getScopePath(),lval);
        rval=genMul(rval,Var::getStep(lval));
    }
    else{//基本类型
        tmp=new Var(symtab.getScopePath(),KW_INT,false);//基本类型
    }
    //减法命令
    symtab.addVar(tmp);
    symtab.addInst(new InterInst(OP_SUB,tmp,lval,rval));//中间代码tmp=lval-rval
    return tmp;
}

/**
 * 乘法语句
 * @param lval
 * @param rval
 * @return
 */
Var* GenIR::genMul(Var *lval, Var *rval) {
    Var* tmp = new Var(symtab.getScopePath(),KW_INT,false);
    symtab.addVar(tmp);
    symtab.addInst(new InterInst(OP_MUL,tmp,lval,rval));
    return tmp;
}

/*
	除法语句
*/
Var* GenIR::genDiv(Var*lval,Var*rval)
{
    Var*tmp=new Var(symtab.getScopePath(),KW_INT,false);//基本类型
    symtab.addVar(tmp);
    symtab.addInst(new InterInst(OP_DIV,tmp,lval,rval));//中间代码tmp=lval/rval
    return tmp;
}

/*
	模语句
*/
Var* GenIR::genMod(Var*lval,Var*rval)
{
    Var*tmp=new Var(symtab.getScopePath(),KW_INT,false);//基本类型
    symtab.addVar(tmp);
    symtab.addInst(new InterInst(OP_MOD,tmp,lval,rval));//中间代码tmp=lval%rval
    return tmp;
}

/**
 * 左单目运算符
 * @param opt
 * @param val
 * @return
 */
Var* GenIR::genOneOpLeft(Tag opt, Var *val) {
    if(!val) return NULL;
    if(val->isVoid()){
        SEMERROR(EXPR_IS_VOID);
        return NULL;
    }
    if(opt == LEA) return genLea(val);
    if(opt == MUL) return genPtr(val);

    if(opt == INC) return genIncL(val);
    if(opt == DEC) return genDecL(val);

    if(val->isRef()) val= genAssign(val);
    if(opt == NOT) return genNot(val);
    if(opt == SUB) return genMinus(val);
    return val;
}

/**
 * 取反运算符
 * @param val
 * @return
 */
Var* GenIR::genNot(Var *val) {
    Var* tmp = new Var(symtab.getScopePath(),KW_INT, false);
    symtab.addVar(tmp);
    symtab.addInst(new InterInst(OP_NOT,tmp,val));
    return tmp;
}

/**
	取负
*/
Var* GenIR::genMinus(Var*val)
{
    if(!val->isBase()){
        SEMERROR(EXPR_NOT_BASE);//运算对象不是基本类型
        return val;
    }
    Var*tmp=new Var(symtab.getScopePath(),KW_INT,false);//生成整数
    symtab.addVar(tmp);
    symtab.addInst(new InterInst(OP_NEG,tmp,val));//中间代码tmp=-val
    return tmp;
}

/**
 * 左自增
 * @param val
 * @return
 */
Var* GenIR::genIncL(Var *val) {
    if(!val->getLeft()){
        SEMERROR(EXPR_NOT_LEFT_VAL);
        return val;
    }
    if(val->isRef()){//++*p情况 => t1=*p t2=t1+1 *p=t2
        Var* t1 = genAssign(val);
        Var* t2 = genAdd(t1,Var::getStep(val));

    }
    symtab.addInst(new InterInst(OP_ADD,val,val,Var::getStep(val)));
    return val;
}

/**
 * 左自减
 * @param val
 * @return
 */
Var* GenIR::genDecL(Var*val)
{
    if(!val->getLeft()){
        SEMERROR(EXPR_NOT_LEFT_VAL);
        return val;
    }
    if(val->isRef()){//--*p情况 => t1=*p t2=t1-1 *p=t2
        Var* t1=genAssign(val);//t1=*p
        Var* t2=genSub(t1,Var::getStep(val));//t2=t1-1
        return genAssign(val,t2);//*p=t2
    }
    symtab.addInst(new InterInst(OP_SUB,val,val,Var::getStep(val)));//中间代码--val
    return val;
}

/**
 * 取地址运算
 * @param val
 * @return
 */
Var* GenIR::genLea(Var *val) {
    if(!val->getLeft()){
        SEMERROR(EXPR_NOT_LEFT_VAL);
        return val;
    }
    if(val->isRef()){
        return val->getPointer();
    }
    else {
        Var* tmp = new Var(symtab.getScopePath(),val->getType(), true);//产生指针类型的变量
        symtab.addVar(tmp);
        symtab.addInst(new InterInst(OP_LEA,tmp,val));
        return tmp;
    }
}

/**
 * 指针取值运算
 * @param val
 * @return
 */
Var* GenIR::genPtr(Var *val) {
    if(val->isBase()){
        SEMERROR(EXPR_IS_BASE);
        return val;
    }
    Var* tmp = new Var(symtab.getScopePath(),val->getType(),false);
    tmp->setLeft(true);//指针运算的结果为左值
    tmp->setPointer(val);//设置指针变量
    symtab.addVar(tmp);
    return tmp;
}

/**
 * 右单目运算符
 * @param val
 * @param opt
 * @return
 */
Var* GenIR::genOneOpRight(Var*val,Tag opt)
{
    if(!val)return NULL;
    if(val->isVoid()){
        SEMERROR(EXPR_IS_VOID);//void函数返回值不能出现在表达式中
        return NULL;
    }
    if(!val->getLeft()){
        SEMERROR(EXPR_NOT_LEFT_VAL);
        return val;
    }
    if(opt==INC)return genIncR(val);//右自加语句
    if(opt==DEC)return genDecR(val);//右自减语句
    return val;
}


/**
 * 右自加
 * @param val
 * @return
 */
Var* GenIR::genIncR(Var*val)
{
    Var*tmp=genAssign(val);//拷贝
    symtab.addInst(new InterInst(OP_ADD,val,val,Var::getStep(val)));//中间代码val++
    return tmp;
}


/**
 * 右自减
 * @param val
 * @return
 */
Var* GenIR::genDecR(Var*val)
{
    Var*tmp=genAssign(val);//拷贝
    symtab.addInst(new InterInst(OP_SUB,val,val,Var::getStep(val)));//val--
    return tmp;
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


/*
	添加一个作用域
*/
void GenIR::push(InterInst*head,InterInst*tail)
{
    heads.push_back(head);
    tails.push_back(tail);
}


/**
 * @details 函数调用语句
 * @param function
 * @param args
 * @return
 */
Var* GenIR::genCall(Fun *function, vector<Var *> &args) {
    if(!function) return NULL;
    for(int i = args.size() - 1; i >= 0;i--){
        genPara(args[i]);
    }
    if(function->getType()==KW_VOID){
        symtab.addInst(new InterInst(OP_PROC,function));
        return Var::getVoid();
    }
    else{
        Var* ret = new Var(symtab.getScopePath(),function->getType(),false);
        symtab.addInst(new InterInst(OP_CALL,function,ret));
        symtab.addVar(ret);
        return ret;
    }
}


