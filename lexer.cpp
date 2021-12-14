#include<stdio.h>
#include<iostream>
#include<string>
#include<fstream>
#include "lexer.h"
#include "token.h"
#include "error.h"
#define BUFLEN 80
#define LEXERROR(code) Error::lexError(code)


char ch;
int lineLen = 0; //缓冲区内的数据大小
int readPos = -1; //读取位置
char line[BUFLEN];//缓冲区
int lineNum = 1;//行号
int colNum = 0;//列号
char lastch = ch;//上一个读取的字符

Scanner::Scanner(char* name)
{
    file=fopen(name,"r");//打开指定的待扫描文件
    if(!file){
        printf(FATAL"文件%s打开失败！请检查文件名和路径。\n",name);
        Error::errorNum++;//错误数累加
    }
    fileName=name;
    //初始化扫描状态
    lineLen=0;
    readPos=-1;
    lastch=0;
    lineNum=1;
    colNum=0;
}


Scanner::~Scanner()
{
    if(file)
    {
        PDEBUG(WARN"文件未全部扫描！\n");
        Error::warnNum++;//警告数累加
        fclose(file);
    }
}

/*
	显示字符
*/
void Scanner::showChar(char ch)
{
    if(ch==-1)printf("EOF");
    else if(ch=='\n')printf("\\n");
    else if(ch=='\t')printf("\\t");
    else if(ch==' ')printf("<blank>");
    else printf("%c",ch);
    printf("\t\t<%d>\n",ch);
}

/*
	基于缓冲区的字符扫描算法,文件扫描接受后自动关闭文件
	缓冲区：使用fread读取效率更高，数据更集中
	字符：从缓冲区内索引获取
*/
int Scanner::scan()
{
    if(!file)//没有文件
        return -1;
    if(readPos==lineLen-1)//缓冲区读取完毕
    {
        lineLen=fread(line,1,BUFLEN,file);//重新加载缓冲区数据
        if(lineLen==0)//没有数据了
        {
            //标记文件结束,返回文件结束标记-1
            lineLen=1;
            line[0]=-1;
        }
        readPos=-1;//恢复读取位置
    }
    readPos++;//移动读取点
    char ch=line[readPos];//获取新的字符
    if(lastch=='\n')//新行
    {
        lineNum++;//行号累加
        colNum=0;//列号清空
    }
    if(ch==-1)//文件结束，自动关闭
    {
        fclose(file);
        file=NULL;
    }
    else if(ch!='\n')//不是换行
        colNum++;//列号递增
    lastch=ch;//记录上个字符
    if(Args::showChar)showChar(ch);
    return ch;
}

/*
	获取文件名
*/
char* Scanner::getFile()
{
    return fileName;
}

/*
	获取行号
*/
int Scanner::getLine()
{
    return lineNum;
}

/*
	获取列号
*/
int Scanner::getCol()
{
    return colNum;
}

Keywords::Keywords()
{
    //add keyword mapping here ~
    keywords["int"] = KW_INT;
    keywords["char"] = KW_CHAR;
    keywords["void"]=KW_VOID;
    keywords["extern"]=KW_EXTERN;
    keywords["if"]=KW_IF;
    keywords["else"]=KW_ELSE;
    keywords["switch"]=KW_SWITCH;
    keywords["case"]=KW_CASE;
    keywords["default"]=KW_DEFAULT;
    keywords["while"]=KW_WHILE;
    keywords["do"]=KW_DO;
    keywords["for"]=KW_FOR;
    keywords["break"]=KW_BREAK;
    keywords["continue"]=KW_CONTINUE;
    keywords["return"]=KW_RETURN;
}

Tag Keywords::getTag(string name)
{
    //如果没有在keywords中找到，那么就认为这个词法记号为ID
    //如果在keywords中找到了，那么就认为是关键字
    return keywords.find(name)!=keywords.end()?keywords[name]:ID;
}



Token* Lexer::tokenize(){
    for(;ch != -1;){
        Token *t = NULL;
        //跳过空白字符
        while (ch == '\t' || ch == ' ' || ch == '\n')
        {
            scan();
        }

        //标识符或关键字的识别
        if(ch == '_' || (ch >= 'a' && ch <= 'z') || ch >= 'A' && ch <= 'Z'){
            std::string name = "";
            do{
                name.push_back(ch);
                scan();

            }while(ch >= 'a' && ch <= 'z' || ch >= 'A' && ch <= 'Z' || ch == '_' || ch >= '0' && ch <= '9');
            Tag tag = keywords.getTag(name);
            //通过前面的Keywords类来识别读入的ID是否为关键字或者保留字
            if(tag == ID){
                t = new Id(name);
            }
            else{
                t = new Token(tag);
            }
        }
        else if(ch == '"'){//字符串
            std::string str;
            while(!scan('"')){
                if(ch == '\\'){
                    scan();
                    if(ch == 'n') str.push_back('\n');
                    else if(ch=='\\')str.push_back('\\');
                    else if(ch=='t')str.push_back('\t');
                    else if(ch=='"')str.push_back('"');
                    else if(ch=='0')str.push_back('\0');
                    else if(ch=='\n');//什么也不做，字符串换行
                    else if(ch==-1){
                        LEXERROR(STR_NO_R_QUTION);//产生无右双引号的异常
                        t=new Token(ERR);
                        break;
                    }
                    else str.push_back(ch);

                }
                else if(ch=='\n'||ch == -1){//文件结束或者换行
                    LEXERROR(STR_NO_R_QUTION);
                    t=new Token(ERR);
                    break;
                }
                else
                    str.push_back(ch);
            }
            if(!t) t = new Str(str);
        }
            //数字类型的词法分析
        else if(ch >= '0' && ch <='9'){
            int val = 0;
            if(ch != '0'){
                do{
                    val = val * 10 + ch - '0';
                    scan();
                }
                while(ch >= '0' &&ch <= '9');
            }
            else {
                scan();
                //16进制读取
                if(ch == 'x' || ch == 'X'){
                    scan();
                    if(ch >= '0' && ch <= '9' || ch >= 'A' && ch <= 'F'|| ch >= 'a' && ch <= 'f')
                    {
                        do
                        {
                            if(ch >= '0' && ch <= '9'){
                                val = val * 16 + ch - '0';
                            }
                            else if(ch >= 'A' && ch <= 'F'){
                                val = val * 16 + ch - 'A' + 10;
                            }
                            else if(ch >= 'a' && ch <= 'f'){
                                val = val * 16 + ch - 'a' + 10;
                            }

                            //扫描下一个字符
                            scan();
                        } while (ch >= '0' && ch <= '9' || ch >= 'A' && ch <= 'F'|| ch >= 'a' && ch <= 'f');
                    }
                    else{
                        //如果后面不是16进制数字，那么就表明发生了错误，返回错误词法记号.
                        LEXERROR(NUM_HEX_TYPE);
                        t = new Token(ERR);
                        break;
                    }
                }
                    //二进制数字
                else if(ch == 'b' || ch == 'B'){
                    scan();
                    if(ch == '0' || ch == '1'){
                        do{
                            val = val * 2 + ch - '0';
                            scan();
                        }while(ch == '0' || ch == '1');
                    }
                        //非二进制表示，产生异常
                    else{
                        LEXERROR(NUM_BIN_TYPE);
                        t = new Token(ERR);
                        break;
                    }
                }
                    //8进制
                else if(ch <= '7' && ch >= '0'){
                    do{
                        val = val * 8 + ch - '0';
                        scan();
                    }
                    while(ch <= '7' && ch >= '0');
                }
            }
            //返回最终的数字类型的词法单元
            if(!t) t = new Num(val);
        }
            //单字符识别
        else if(ch=='\''){
            char c;
            scan();
            if(ch=='\\'){//转义，则需要继续往下读一个单元
                scan();
                if(ch=='n')c='\n';
                else if(ch=='\\')c='\\';
                else if(ch=='t')c='\t';
                else if(ch=='0')c='\0';
                else if(ch=='\'')c='\'';
                else if(ch==-1||ch=='\n'){//文件结束或者换行
                    LEXERROR(CHAR_NO_R_QUTION);
                    t=new Token(ERR);
                }
                else c=ch;//没有转义,则默认c为斜杠记号
            }
            else if(ch=='\n'||ch==-1){//行 文件结束
                LEXERROR(CHAR_NO_R_QUTION);
                t=new Token(ERR);
            }
            else if(ch=='\''){//没有数据
                LEXERROR(CHAR_NO_DATA);
                t = new Token(ERR);
                scan();//读掉引号
            }
            else c=ch;//正常字符
            if(!t){
                if(scan('\'')){//匹配右侧引号,读掉引号
                    t = new Char(c);
                }
                else{
                    LEXERROR(CHAR_NO_R_QUTION);
                    t = new Token(ERR);
                }
            }
        }
            //界符
        else{
            switch (ch)
            {
                //Arithmetic Operators算术运算符
                //+ && ++
                case '+':
                    if(scan('+'))t = new Token(INC);
                    else t = new Token(ADD);
                    break;

                case '#'://忽略行（忽略宏定义）
                    while(ch!='\n' && ch!= -1)
                        scan();//行 文件不结束
                    t=new Token(ERR);
                    break;
                    //	- && --
                case '-':
                    if(scan('-')) t = new Token(DEC);
                    else t = new Token(SUB);
                    break;

                    //	*
                case '*':
                    t = new Token(MUL);
                    scan();
                    break;
                    //	/
                case '/':
                    scan();
                    if(ch == '/'){//单行注释
                        while(!(ch == '\n' || ch == -1)){
                            scan();
                        }
                        t = new Token(ERR);

                    }
                    else if(ch == '*'){
                        while(!scan(-1)){
                            if(ch == '*'){
                                //为了防止注释中也有*
                                while(scan('*'));
                                if(scan('/')){
                                    t = new Token(ERR);
                                    break;
                                }

                            }
                        }
                        //表明直到文件结束也没有将注释读取完毕
                        if(!t && ch == -1){
                            LEXERROR(COMMENT_NO_END);
                            t = new Token(ERR);
                        }
                    }
                    else {//除法符号
                        t = new Token(DIV);

                    }
                    break;
                    //	%
                case '%':
                    t = new Token(MOD);
                    scan();
                    break;
                    //Relational operators
                case '>':
                    if(scan('=')) t = new Token(GE);
                    else t = new Token(GT);
                    break;
                case '<':
                    if(scan('=')) t = new Token(LE);
                    else t = new Token(LT);
                    break;
                case '=':
                    if(scan('=')) t = new Token(EQU);
                    else t = new Token(ASSIGN);
                    break;
                    //Logical Operators

                case '&':
                    if(scan('&')) t = new Token(AND);
                    else t = new Token(LEA);
                    break;
                case '|':
                    if(scan('|')) t = new Token(OR);
                    else t = new Token(ERR);
                    if(t->tag == ERR){
                        LEXERROR(OR_NO_PAIR);//|符号没有成对，产生错误
                    }
                    break;
                case '!':
                    if(scan('=')) t = new Token(NEQU);
                    else t = new Token(NOT);
                    break;

                case ',':
                    t = new Token(COMMA);
                    scan();
                    break;
                case ':':
                    t = new Token(COLON);
                    scan();
                    break;
                case ';':
                    t = new Token(SEMICON);
                    scan();
                    break;
                case '(':
                    t = new Token(LPAREN);
                    scan();
                    break;
                case ')':
                    t = new Token(RPAREN);
                    scan();
                    break;
                case '[':
                    t = new Token(LBRACK);
                    scan();
                    break;
                case ']':
                    t = new Token(RBRACK);
                    scan();
                    break;
                case '{':
                    t = new Token(LBRACE);
                    scan();
                    break;
                case '}':
                    t = new Token(RBRACE);
                    scan();
                    break;
                case -1:
                    t = new Token(END);
                    scan();
                    break;
                default:
                    t = new Token(ERR);
                    LEXERROR(TOKEN_NO_EXIST);
                    scan();
            }
        }
        //词法记号内存管理
        if( token ) delete token;
        token = t;//强制记录
        if(token && token->tag != ERR)//有效,直接返回
            return token;
        else
            continue;//否则一直扫描直到结束
    }
    //文件结束
    if(token) delete token;
    return token = new Token(END);
}

//C++的静态成员变量需要在在外面初始化
Keywords Lexer::keywords;

Lexer::Lexer (Scanner &sc):scanner(sc)
{
    token=NULL;//初始化词法记号记录，该变量被共享
    ch=' ';//初始化为空格
}

Lexer::~Lexer ()
{
    if(!token)//删除已经记录的词法记号变量的内存，防止内存溢出
    {
        delete token;
    }
}

bool Lexer::scan(char need)
{
    ch=scanner.scan();//扫描出字符
    if(need){
        if(ch!=need)//与预期不吻合
            return false;
        ch=scanner.scan();//与预期吻合，扫描下一个
        return true;
    }
    return true;
}
