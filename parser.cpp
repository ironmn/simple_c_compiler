//
// Created by lelouch on 2021/12/14.
//

#include "parser.h"
#include "token.h"
#include "lexer.h"
#include "symbol.h"
#include "symtab.h"
#include "error.h"
#include "compiler.h"

Parser::Parser(Lexer&lex,SymTab&tab)
        :lexer(lex),symtab(tab)
{}
//outsider entry interface for parser
void Parser::analyse(){
    move();
    program();
}

/*
    移进
*/
void Parser::move(){
    //读取一个词法记号
    look = lexer.tokenize();
    if(Args::showToken){
        printf("%s\n",look->toString().c_str());
    }
}



/*
    匹配，查看并移动
*/
bool Parser::match(Tag need){
    //if we match the terminal lexer we need, then move
    if(look->tag == need){
        move();
        return true;
    }
        //如果匹配的终结符有问题，那么就需要返回false并报告错误
        //报告错误的工作在语句分析模块里完成
    else return false;
}
//报告错误
#define SYNERROR(code,t) Error::synError(code,t)

/*
错误修复
*/
#define _(T) || look->tag == T
//定义F(C)这个操作的原因是为了对look当前所指向的词法记号进行自定义操作
//如果所有的词法记号匹配都是通过match完成的，那么假如匹配成功，会直接调到
//下一，但是我们设计的词法记号链表是单向流动的，不能够回头，在需要记录相关
//信息的时候必须把读取和移进的操作分离
#define F(C) look->tag == C
void Parser::recovery(bool cond,SynError lost,SynError wrong){
    if(cond)//如果在给定的follow集合内
        SYNERROR(lost,look);
    else
        SYNERROR(wrong,look);
    move();
}

//类型
#define TYPE_FIRST F(KW_INT)_(KW_CHAR)_(KW_VOID)
//表达式
#define EXPR_FIRST F(LPAREN)_(NUM)_(CH)_(STR)_(ID)_(NOT)_(SUB)_(LEA)_(MUL)_(INC)_(DEC)
//左值运算
#define LVAL_OPR F(ASSIGN)_(OR)_(AND)_(GT)_(GE)_(LT)_(LE)_(EQU)_(NEQU)_(ADD)_(SUB)_(MUL)_(DIV)\
_(MOD)_(INC)_(DEC)
//右值运算
#define RVAL_OPR F(OR)_(AND)_(GT)_(GE)_(LT)_(LE)_(EQU)_(NEQU)_(ADD)_(SUB)_(MUL)_(DIV)\
_(MOD)
//语句
#define STATEMENT_FIRST (EXPR_FIRST)_(SEMICON)_(KW_WHILE)_(KW_FOR)_(KW_DO)_(KW_IF)\
_(KW_SWITCH)_(KW_RETURN)_(KW_BREAK)_(KW_CONTINUE)

/*
    文法描述：
    <program> -> <segment><program> | e

*/
void Parser::program(){
    //如果已经读取到了文件的末尾，那么就结束分析
    //这是整个文法分析程序的递归边界
    if(F(END)){
        return;
    }
    else{
        segment();
        program();
    }
}

/*
    <program>属于第一层的抽象，它所描述的文法从抽象逻辑的角度来看，就是
    整个源程序分为N个部分，其中每一个部分可以被分解为第二层的抽象逻辑<segment>
    对应的文法如下：
    <segment>      ->       KW_EXTERN <type> <def> | <type> <def>
*/
void Parser::segment(){
    //匹配外部声明关键字
    bool ext = match(KW_EXTERN);
    Tag t = type();
    //'ext' && 't' are inhierent attributes
    def(ext,t);
}

/*
    type对应上面segment右边的<type>语法记号
    之所以将type设置为有返回类型的函数，是因为
    <type>和后面的<def>有上下文关联，为了后续的语义分析，
    则必须要将某个文法左边的语法记号意义传递给右边的语法记号

    <type>      ->          KW_INT | KW_CHAR | KW_VOID
*/
Tag Parser::type(){
    Tag tmp = KW_INT;
    if(TYPE_FIRST){//说明匹配到了文法中要求的终结符
        tmp = look->tag;
        move();
    }
        //否则报错
        //----------------------------------------need extra judge code
    else return ERR;
}

/*
    def主要分为变量定义和函数定义。
    由于变量在程序运行的时候需要使用内存，所以必须为其在符号表中注册信息
    所以我们在返回的时候直接返回一个Var类型的指针
    <defdata>       ->       ID <varrdef> | MUL ID <init>


*/
Var* Parser::defdata(bool ext,Tag t){
    string name = "";
    if(F(ID)){//如果向前看符号属于ID类型
        name = (((Id*)look) -> name);
        move();
        //和前面解释的原因一致。因为当前读取的非终结符会对后面的文法分析语义产生影响，
        //所以必须要将这些参数进行传递
        //inherited attributes
        return varrdef(ext,t,false,name);
    }
        //如果下一个终结符是MUL类型的,则说明为指针变量
    else if(match(MUL)){
        if(F(ID)){
            name = (((Id*)look) -> name);
            move();
        }
        else{//终结符失配，说明出现语法错误
            recovery(F(SEMICON)_(COMMA)_(ASSIGN),ID_LOST,ID_WRONG);
        }
        return init(ext,t,false,name);
    }
    else{
        recovery(F(SEMICON)_(COMMA)_(ASSIGN)_(LBRACK),ID_LOST,ID_WRONG);
        return varrdef(ext,t,false,name);
    }
}



