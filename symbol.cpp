//
// Created by lelouch on 2021/12/14.
//
#include "common.h"
#include "symbol.h"
#include "symtab.h"
#include "token.h"
#include "error.h"

//打印错误信息
#define SEMERROR(code,name) Error::semError(code,name)
/*******************************************************************************
                                   变量结构
*******************************************************************************/

/**
 * 获取void特殊变量
 * @return
 */
Var* Var::getVoid() {
    return SymTab::voidVar;
}

/*
	void变量
*/
Var::Var()
{
    clear();
    setName("<void>");//特殊变量名字
    setLeft(false);
    intVal=0;//记录数字数值
    literal=false;//消除字面量标志
    type=KW_VOID;//hack类型
    isPtr=true;//消除基本类型标志
}

/**
 * 获取特殊变量
 */
 Var* Var::getTrue() {
     return SymTab::one;
 }

//关键信息初始化
void Var::clear() {
    scopePath.push_back(-1);
    externed = false;
    isPtr = false;
    isArray = false;
    isLeft = true;//默认变量能够作为左值使用
    inited = false;
    literal = false;
    size = 0;
    offset = 0;
    ptr =  NULL;
    index = -1;
    initData = NULL;
    live = false;
    regId = -1;//默认放在内存中
    inMem = false;
}

//临时变量
Var::Var(vector<int> &sp, Tag t, bool ptr) {
    clear();
    scopePath=sp;//初始化作用域路径
    setType(t);
    setPtr(ptr);
    setName("");
    setLeft(false);//默认不可为左值
}

//拷贝临时变量
Var::Var(vector<int> &sp, Var *v) {
    clear();
    scopePath=sp;
    setType(v->type);
    setPtr(v->isPtr || v->isArray);//数组或者指针都是指针类型的
    setName("");
    setLeft(false);
}

/*
 * 变量或者指针
 * */
Var::Var(vector<int> &sp, bool ext, Tag t, bool ptr, string name, Var *init) {
    clear();
    scopePath=sp;
    setExtern(ext);
    setType(t);
    setPtr(ptr);
    setName(name);
    initData = init;
}

//数组
Var::Var(vector<int> &sp, bool ext, Tag t, string name, int len) {
    clear();
    scopePath=sp;
    setExtern(ext);
    setType(t);
    setName(name);
    setArray(len);
}

//整数常量
Var::Var(int val) {
    clear();
    setName("<int>");
    literal=true;
    setLeft(false);
    setType(KW_INT);
    intVal=val;//记录数字值
}
//常量，但是不涉及作用域的变化，字符串存储在字符表中，其他的常量作为初始值
Var::Var(Token *lt) {
    clear();
    literal=true;
    setLeft(false);
    switch (lt->tag) {
        case NUM:
            setType(KW_INT);
            name="<int>";
            intVal=((Num*)lt)->val;
            break;
        case CH:
            setType(KW_CHAR);
            name="<char>";//类型作为名称
            intVal=0;
            charVal=((Char*)lt)->ch;
            break;
        case STR:
            setType(KW_CHAR);
            //name=GenIR::genLb();//产生一个新的名字
            strVal=((Str*)lt)->str;//记录字符串值
            setArray(strVal.size()+1);//字符串作为字符数组存储
            break;
    }
}
void Var::setExtern(bool ext) {
    externed = ext;
    size=0;
}

/*
 * 设置类型
 * */
void Var::setType(Tag t) {
    type = t;
    if(type == KW_VOID){
        SEMERROR(VOID_VAR,"");//不允许使用void类型的变量
        type = KW_INT;
    }
    if(!externed && type == KW_INT) size = 4;//int型的大小设置为4
    else if(!externed && type == KW_CHAR) size = 1;//char型设置为1
}


/**
 * 设置指针
 * @param ptr
 */
void Var::setPtr(bool ptr) {
    if(!ptr) return;
    isPtr = true;
    if(!externed) size = 4;//将指针设置为4字节
    //这里和之前自己做的那个tiny——c——compiler不同点在于，那个项目直接把指针扔到操作系统中，所以如果设置为4字节在64位机器上就会有问题
    //而这个地方的指针完全由我们自己来操作它的内存分配，所以不需要为了64位机器和32位机器把这个指针大小动态切换
}

void Var::setName(string n) {
    if(n == ""){//如果n是空串，就由IR来生成标号
        //n = GenIR::genLb();
    }
    name = n;
}

void Var::setArray(int len) {
    if(len <= 0){
        SEMERROR(ARRAY_LEN_INVALID,name);
        return ;
    }
    else{
        isArray=true;
        isLeft=false;//数组不能够作为左值
        arraySize = len;
        if(!externed) size *= len;
    }
}

//如果变量有初始化表达式，设置初始值
bool Var::setInit() {
    Var* init = initData;//取出初始数值
    if(!init) return false;//没有初始化表达式，返回false
    inited = false;//默认初始化过程是失败的
    if(externed){
        SEMERROR(DEC_INIT_DENY,name);//外部声明的变量不允许初始化
    }
//    else if(!GenIR::typeCheck(this,init))
//        SEMERROR(VAR_INIT_ERR,name);//类型检查不兼容

    else if(init->literal){//如果初始化的表达式是一个常量
        inited = true;
        if(init->isArray){//如果初始化常量是数组，就假定它是字符创
            ptrVal=init->name;//字符指针变量的初始值=常量字符串的名字
        }
        else intVal = init->intVal;//拷贝数值
    }
    else{
        if(scopePath.size() == 1){
            SEMERROR(GLB_INIT_ERR,name);
        }
        else return true;
    }
    return false;
}

/*
	获取初始化变量数据
*/
Var* Var::getInitData()
{
    return initData;
}

string Var::getName() {
    return this->name;
}

/*
	获取extern
*/
bool Var::getExtern()
{
    return externed;
}

/*
	获取作用域路径
*/
vector<int>& Var::getPath()
{
    return scopePath;
}

/*
	获取类型
*/
Tag Var::getType()
{
    return type;
}

/*
	判断是否是字符变量
*/
bool Var::isChar()
{
    return (type==KW_CHAR) && isBase();//是基本的字符类型
}

/*
	判断字符指针
*/
bool Var::isCharPtr()
{
    return (type==KW_CHAR) && !isBase();//字符指针或者字符数组
}

/*
	获取指针
*/
bool Var::getPtr()
{
    return isPtr;
}


/*
	获取数组
*/
bool Var::getArray()
{
    return isArray;
}

/*
	设置指针变量
*/
void Var::setPointer(Var* p)
{
    ptr=p;
}

/*
	获取指针变量
*/
Var* Var::getPointer()
{
    return ptr;
}

/*
	获取字符指针内容
*/
string Var::getPtrVal()
{
    return ptrVal;
}

/*
	获取字符串常量内容
*/
string Var::getStrVal() {
    return strVal;
}


/*
	设置变量的左值
*/
void Var::setLeft(bool lf)
{
    isLeft=lf;
}

/*
	获取变量的左值
*/
bool Var::getLeft()
{
    return isLeft;
}

/*
	设置栈帧偏移
*/
void Var::setOffset(int off)
{
    offset=off;
}

/*
	获取栈帧偏移
*/
int Var::getOffset()
{
    return offset;
}


/*
	获取变量大小
*/
int Var::getSize()
{
    return size;
}

/*
	是数字
*/
bool Var::isVoid()
{
    return type==KW_VOID;
}

/*
	是基本类型
*/
bool Var::isBase()
{
    return !isArray && !isPtr;
}

/*
	是引用类型
*/
bool Var::isRef()
{
    return !!ptr;
}

/*
	是否初始化
*/
bool Var::unInit()
{
    return !inited;
}

/*
	是否是常量
*/
bool Var::notConst()
{
    return !literal;
}

/*
	获取常量值
*/
int Var::getVal()
{
    return intVal;
}

/*
		是基本类型常量（字符串除外），没有存储在符号表，需要单独内存管理
*/
bool Var::isLiteral()
{
    return this->literal&&isBase();
}

/*
	获取字符串常量原始内容，将特殊字符转义
*/
string Var::getRawStr()
{
    string raw;
    for(int i=0;i<strVal.size();i++){
        switch(strVal[i])
        {
            case '\n':raw.append("\\n");break;
            case '\t':raw.append("\\t");break;
            case '\0':raw.append("\\000");break;
            case '\\':raw.append("\\\\");break;
            case '\"':raw.append("\\\"");break;
            default:raw.push_back(strVal[i]);
        }
    }
    raw.append("\\000");//结束标记
    return raw;
}


Fun::Fun(bool ext, Tag t, string n, vector<Var *> &paraList) {
    externed = ext;
    type = t;
    name = n;
    paraVar = paraList;
    curEsp = 0;
    maxDepth = 0;
    for(int i = 0,argOff = 4;i < paraVar.size();i++,argOff += 4){
        paraVar[i]->setOffset(argOff);
    }

    relocated = false;
}

Fun::~Fun() {

}

/**
 * 定义局部变量的栈帧偏移
 * @param var
 */
void Fun::locate(Var *var) {
    int size = var->getSize();
    size += (4 - size%4) % 4;//4字节对齐
    scopeEsp.back() += size;
    curEsp += size;
    var->setOffset(-curEsp);//局部变量的栈帧偏移为负数
}

/*
	声明定义匹配
*/
#define SEMWARN(code,name) Error::semWarn(code,name)
bool Fun::match(Fun *f) {
    //区分函数的返回值
    if(name!=f->name)
        return false;
    if(paraVar.size()!=f->paraVar.size())
        return false;
    int len=paraVar.size();
    for(int i=0;i<len;i++){
        if(GenIR::typeCheck(paraVar[i],f->paraVar[i])){//类型兼容
            if(paraVar[i]->getType()!=f->paraVar[i]->getType()){//但是不完全匹配
                SEMWARN(FUN_DEC_CONFLICT,name);//函数声明冲突——警告
            }
        }
        else
            return false;
    }
    //匹配成功后再验证返回类型
    if(type!=f->type){
        SEMWARN(FUN_RET_CONFLICT,name);//函数返回值冲突——警告
    }
    return true;
}

bool Fun::match(vector<Var*>&args)
{
    if(paraVar.size()!=args.size())
        return false;
    int len=paraVar.size();
    for(int i=0;i<len;i++){
        if(!GenIR::typeCheck(paraVar[i],args[i]))//类型检查不兼容
            return false;
    }
    return true;
}
