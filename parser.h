//
// Created by lelouch on 2021/12/14.
//

#ifndef SIMPLE_C_COMPILER_PARSER_H
#define SIMPLE_C_COMPILER_PARSER_H
#include "common.h"

class Parser{
    //文法分析开始
    void program();
    void segment();
    Tag type();

    //声明与定义
    Var* defdata(bool ext,Tag t);//ext表示被分析的变量是否由extern声明，参数t用来表示变量的类型
    void deflist(bool ext,Tag t);
    Var* varrdef(bool ext,Tag t,bool ptr, string name);
    Var* init(bool ext,Tag t,bool ptr, string name);
    void def(bool ext, Tag t);
    void idtail(bool ext,Tag t,bool ptr,string name);

    //函数
    Var* paradatatail(Tag t ,string name);
    Var* paradata(Tag t);
    void para(vector<Var*> &list);
    void paralist(vector<Var*> &list);
    void funtail(Fun *f);
    void block();
    void subprogram();
    void localdef();

    //语句
    void statement();
    void whilestat();
    void dowhilestat();
    void forstat();
    void forinit();
    void ifstat();
    void elsestat();
    void switchstat();
    void casestat(Var* cond);
    Var* caselabel();

    //表达式
    Var* altexpr();
    Var* expr();
    Var* assexpr();
    Var* asstail(Var*lval);
    Var* orexpr();
    Var* ortail(Var*lval);
    Var* andexpr();
    Var* andtail(Var*lval);
    Var* cmpexpr();
    Var* cmptail(Var*lval);
    Tag cmps();
    Var* aloexpr();
    Var* alotail(Var*lval);
    Tag adds();
    Var* item();
    Var* itemtail(Var*lval);
    Tag muls();
    Var* factor();
    Tag lop();
    Var* val();
    Tag rop();
    Var* elem();
    Var* literal();
    Var* idexpr(string name);
    void realarg(vector<Var*> &args);
    void arglist(vector<Var*> &args);
    Var* arg();

    //词法分析
    Lexer &lexer;
    Token* look;//向前看记号

    //符号表
    SymTab &symtab;

    //中间代码生成器
    GenIR &ir;
    void move();
    bool match(Tag t);
    //语法错误恢复
    void recovery(bool cond,SynError lost,SynError wrong);

public:
    Parser(Lexer &lex, SymTab &tab, GenIR& inter);

    void analyse();//外部调用parser的接口
};
#endif //SIMPLE_C_COMPILER_PARSER_H
