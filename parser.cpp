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

Parser::Parser(Lexer&lex,SymTab&tab,GenIR&inter)
        :lexer(lex),symtab(tab),ir(inter)
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
    //'ext' && 't' are inherent attributes
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
    else
        recovery(F(ID)_(MUL),TYPE_LOST,TYPE_WRONG);
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

/**<deflist>    ->      COMMA <defdata> <deflist> | SEMICON
 * @author Martin Xie
 * @details
 * <deflist>带有继承属性ext,t等等，将这些属性验证递归下降分析传递到下面
 * 同时后面的<defdata>表示变量的定义。
 *
 * @param ext 是否为外部变量
 * @param t 变量类型
 */
void Parser::deflist(bool ext, Tag t) {
    if(match(COMMA)){//如果成功匹配了逗号
        //那么需要将下一个可能要进行定义或者声明的变量传递到符号表中
        symtab.addVar(defdata(ext,t));//添加新的变量
        deflist(ext,t);
    }
    else if(match(SEMICON)){
        return;//结束变量定义
    }
    else{//如果匹配到的是ID或者MUL类型的，
        if(F(ID)_(MUL)){
            recovery(1,COMMA_LOST,COMMA_WRONG);
            symtab.addVar(defdata(ext,t));
            deflist(ext,t);
        }
        else{
            recovery(TYPE_FIRST||STATEMENT_FIRST||F(KW_EXTERN)_(RBRACE),
                     SEMICON_LOST,SEMICON_WRONG);

        }
    }
}

/**
 * <varrdef>       ->       LBRACK NUM RBRACK | <init>
 * @param ext
 * @param t
 * @param ptr
 * @param name
 * @return
 */
Var* Parser::varrdef(bool ext, Tag t, bool ptr, string name) {
    if(match(LBRACK)){//匹配了左括号
        int len = 0;
        if(F(NUM)){
            len = ((Num*)look)->val;
            move();
        }
        else{
            recovery(F(RBRACK),NUM_LOST,NUM_WRONG);
        }
        if(!match(RBRACE)){//如果没有匹配到右中括号，那么检查后面是否是逗号和分号。
            recovery(F(COMMA)_(SEMICON),RBRACE_LOST,RBRACE_WRONG);
        }
        return new Var(symtab.getScopePath(),ext,t,name,len);
    }
    else{
        return init(ext,t,ptr,name);
    }
}

/**
 * <init>       ->      ASSIGN <expr> | e
 * @param ext
 * @param t
 * @param ptr
 * @param name
 * @return
 */
Var* Parser::init(bool ext, Tag t, bool ptr, string name) {
    Var* initVal = NULL;
    if(match(ASSIGN)){
        initVal=expr();
    }
    return new Var(symtab.getScopePath(),ext,t,ptr,name, initVal);//返回带有初始值的变量
}

/**
 * <def>        ->          ID <idtail> | MUL ID <init> <deflist>
 * @param ext
 * @param t
 */
void Parser::def(bool ext, Tag t) {
    string name = "";
    if(match(MUL)){//指针变量
        if(F(ID)){
            name = ((Id*)look)->name;
            move();
        }
        else recovery(F(SEMICON)_(COMMA)_(ASSIGN),ID_LOST,ID_WRONG);
        symtab.addVar(init(ext,t,true,name));
        deflist(ext,t);
    }
    else{
        if(F(ID)){
            name = ((Id*)look)->name;
            move();
        }
        else recovery(F(SEMICON)_(COMMA)_(ASSIGN)_(LPAREN)_(LBRACK),ID_LOST,ID_WRONG);
        idtail(ext,t,false,name);
    }
}





/**表达式处理
 * <altexpr>        ->         <expr> | e
 * @return
 */
Var* Parser::aloexpr() {
    if(EXPR_FIRST){
        return expr();
    }
    //表明返回的类型为空
    return Var::getVoid();
}

/**
 * <expr>       ->          <assexpr>
 * */
Var* Parser::expr() {
    return assexpr();
}

/**
 * <assexpr>    ->      <orexpr><asstail>
 * @return
 */
Var* Parser::assexpr() {
    Var* lval = orexpr();
    return asstail(lval);
}
/**
 * <asstail>            ->      ASSIGN <assexpr>| e
 * @param lval
 * @return
 * @describe 由于赋值语句是右结合的，所以在递归的过程中，应当使用后序遍历的方式，
 *           即直接遍历到AST的叶子节点上，然后开始返回
 */

Var* Parser::asstail(Var *lval) {
    if(match(ASSIGN)){
        Var* rval = assexpr();//囊括了右边的所有子表达式
        Var* result = ir.genTwoOp(lval,ASSIGN,rval);
        return asstail(result);
    }
    return lval;//表明为空字符集，没有匹配到等于符号，这时直接返回参数中的属性值
}

/**
 * @syntax <orexpr>     ->      <andexpr> <ortail>
 * @return
 */
Var* Parser::orexpr() {
    Var *lval = andexpr();
    return ortail(lval);
}

/**
 *
 * @syntax <ortail>      ->     OR <andexpr> <ortail> | e
 * @param lval 从左边传递过来的左值
 * @return
 */
Var* Parser::ortail(Var *lval) {
    if(match(OR)){
        Var* rval = andexpr();
        Var* result = ir.genTwoOp(lval,OR,rval);
        return ortail(result);
    }
    return lval;
}

/**
 * @syntax  <andexpr>       ->      <cmpexpr><andtail>
 * @return
 */
Var* Parser::andexpr() {
    Var* lval = cmpexpr();
    return andtail(lval);
}

/**
 * @syntax <andtail>        ->          AND <cmpexpr> <andtail> |  e
 * @param lval
 * @return
 */
Var* Parser::andtail(Var *lval) {
    if(match(AND)){
        Var* rval = cmpexpr();
        Var* result = ir.genTwoOp(lval,AND,rval);
        return andtail(result);
    }
    return lval;
}

/**
 * @syntax <cmpexpr>       ->    <aloexpr> <cmptail>
 * @return
 */
Var* Parser::cmpexpr() {
    Var* lval = aloexpr();
    return cmptail(lval);
}


/**
 * @syntax <cmptail>        ->   <cmps> <aloexpr> <cmptail> | e
 * @param lval
 * @return
 */
Var* Parser::cmptail(Var *lval) {
    if(F(GT)_(GE)_(LT)_(LE)_(EQU)_(NEQU)){
        Tag opt=cmps();
        Var*rval=aloexpr();
        Var* result=ir.genTwoOp(lval,opt,rval);
        return cmptail(result);
    }
    return lval;
}

/*
	<cmps>				->	gt|ge|ls|le|equ|nequ
*/
Tag Parser::cmps(){
    Tag opt = look->tag;
    move();
    return opt;
}

/**
 * @describe 算术运算表达式
 * @syntax <aloexpr>			->	<item><alotail>
 * @return
 */
Var* Parser::aloexpr() {
    Var *lval = item();//运算单元
    return alotail(lval);
}

/**
 * @symtax <alotail>        ->     <adds> <item> <alotail> | e
 * @param lval
 * @return
 */
Var *Parser::alotail(Var *lval) {
    if(F(ADD)_(SUB)){
        Tag opt = adds();
        Var* rval = item();
        Var* result = ir.genTwoOp(lval,opt,rval);
        return alotail(result);
    }
    return lval;
}

/**
 * @syntax <adds>       ->      ADD | SUB
 * @return
 */
Tag Parser::adds(){
    Tag opt = look->tag;
    move();
    return opt;
}

/**
 * @syntax  <item>      ->      <factor> <itemtail>
 * @return
 */
Var* Parser::item(){
    Var *lval = factor();
    return itemtail(lval);

}

/**
 * @syntax  <itemtail>      ->      <muls><factor><itemtail>  | e
 * @param lval
 * @return
 */

Var* Parser::itemtail(Var *lval) {
    if(F(MUL)_(DIV)_(MOD)){
        Tag opt = muls();
        Var* rval = factor();
        Var* result = ir.genTwoOp(lval,opt,rval);
        return itemtail(result);
    }
    return lval;
}

/**
 * @syntax   <muls>     ->      MUL | DIV | MOD
 * @return
 */
Tag Parser::muls(){
    Tag opt = look->tag;
    move();
    return opt;

}


/**
 * @syntax      <factor>     ->      <lop> <factor> | <val>
 * @return
 */
Var* Parser::factor(){
    if(F(NOT)_(SUB)_(LEA)_(MUL)_(INC)_(DEC)){
        Tag opt = lop();
        Var* v = factor();
        return ir.genOneOpLeft(opt,v);//左单目操作
    }
    else{
        return val();
    }
}

/**
 * @syntax     <lop>    ->      NOT | SUB | LEA | MUL | INC | DEC
 * @return
 */
Tag Parser::lop(){
    Tag opt = look->tag;
    move();
    return opt;
}


/**
 * @syntax      <val>       ->          <elem><rop>
 * @return
 */
Var* Parser::val(){
    Var* v = elem();
    if(F(INC)_(DEC)){
        Tag opt = rop();
        v = ir.genOneOpRight(v,opt);
    }
    return v;
}

/**
 * @syntax  <rop>       ->      INCR  | DECR | e
 * @return
 */
Var* Parser::rop(){
    Tag opt = look->tag;
    move();
    return opt;
}

/**
 * @syntax  <elem>		->	ID <idexpr>|LPAREN <expr> RPAREN|<literal>
 * @return
 */
Var* Parser::elem() {
    Var* v = NULL;
    if(F(ID)){
        string name = ((Id*)look)->name;
        move();
        v = idexpr(name);
    }
    else if(match(LPAREN)){
        v = expr();
        if(!match(RPAREN)){
            recovery(LVAL_OPR,RPAREN_LOST,RPAREN_WRONG);
        }
    }
    else v = literal();//常量
    return v;
}

/**
 * @syntax      <literal>   ->      NUM | STR | CH
 * @return
 */
Var* Parser::literal() {
    Var* v = NULL;
    if(F(NUM)_(STR)_(CH)){
        v = new Var(look);
        if(F(STR)){
            symtab.addStr(v);
        }
        else symtab.addVar(v);
        move();
    }
    else recovery(RVAL_OPR,LITERAL_LOST,LITERAL_WRONG);
    return v;
}

/**
 * <idexpr>       ->        LBRACK  <expr>  RBRACK | LPAREN <realarg>  RPAREN | e
 * @param name
 * @return
 */
Var* Parser::idexpr(string name) {
    Var *v = NULL;
    if(match(LBRACK)){
        Var* index = expr();
        if(!match(RBRAK)){
            recovery(LVAL_OPR,LBRACK_LOST,LBRACK_WRONG);
        }
        Var* array = symtab.getVar(name);//获取数组
        v = ir.genArray(array,index);//产生数组运算表达式
    }
    else if(match(LPAREN)){
        vector<Var* > args;
        realarg(args);
        if(!match(RBRACE)){
            recovery(RVAL_OPR,RPAREN_LOST,RPAREN_WRONG);
        }
        Fun *function = symtab.getFun(name,args);
        v = ir.genCall(function,args);
    }
    else v = symtab.genVar(name);//获取变量
    return v;
}


/**
 * @syntax   <realarg>    ->      <arg> <argilst>  | e
 * @param args
 */
void Parser::realarg(vector<Var *> &args) {
    if(EXPR_FIRST){
        args.push_back(arg());
        arglist(args);
    }
}

/**
 * @syntax <arglist>        ->      COMMA<arg><arglist> | e
 * @param args
 */
void Parser::arglist(vector<Var *> &args) {
    if(match(COMMA)){
        args.push_back(arg());
        arglist(args);
    }
}

/**
 * @syntax <arg>    ->      <expr>
 * @return
 */
Var* Parser::arg() {
    return expr();//添加一个实际参数
}






