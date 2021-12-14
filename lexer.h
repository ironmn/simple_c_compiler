//
// Created by lelouch on 2021/12/14.
//

#ifndef SIMPLE_C_COMPILER_LEXER_H
#define SIMPLE_C_COMPILER_LEXER_H
#pragma once
#include "compiler.h"
#include "common.h"
#include "token.h"
#include <ext/hash_map>
using namespace __gnu_cxx;

class Scanner
{
    //文件指针
    char*fileName;//文件名
    FILE*file;//文件指针

    //内部状态
    static const int BUFLEN=80;//扫描缓冲区长度
    char line[BUFLEN];
    int lineLen;//当前行的长度
    int readPos;//读取的位置
    char lastch;//上一个字符，主要用于判断换行位置

    //读取状态
    int lineNum;//记录行号
    int colNum;//列号

    //显示字符
    void showChar(char ch);

public:

    //构造与初始化
    Scanner(char* name);
    ~Scanner();

    //扫描
    int scan();//基于缓冲区的字符扫描算法,文件扫描接受后自动关闭文件

    //外部接口
    char*getFile();//获取文件名
    int getLine();//获取行号
    int getCol();//获取列号

};

class Keywords
{
    struct string_hash
    {
        size_t operator () (const string &str) const {
            return __stl_hash_string(str.c_str());
        }
    };
    //将string类型的数据结构映射为自定义的Tag类型，通过hash值进行排序
    hash_map<string, Tag, string_hash> keywords;

public:
    Keywords();
    Tag getTag(string name);

};

class Lexer
{
    static Keywords keywords;//关键字列表

    char ch;//读入的字符
    Scanner &scanner;//扫描器
    Token* token;//记录扫描的词法记号

public:
    bool scan(char need=0);//扫描与匹配
    Lexer (Scanner&sc);
    ~Lexer();
    Token* tokenize();//有限自动机匹配，词法记号解析
};
#endif //SIMPLE_C_COMPILER_LEXER_H
