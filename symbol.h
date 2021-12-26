//
// Created by lelouch on 2021/12/14.
//

#ifndef SIMPLE_C_COMPILER_SYMBOL_H
#define SIMPLE_C_COMPILER_SYMBOL_H
#pragma once

#include"common.h"

class Var{
    bool literal;	//判断是否为常量
    vector<int> scopePath; //作用域路径
    bool externed;	//是否为external声明
    Tag type;		//变量类型
    string name;	//变量名称
    bool isPtr;		//是否为指针变量

    bool isArray;	//是否为数组
    int arraySize;	//如果是数组，对应的数组长度
    bool isLeft;	//是否可以作为左值
    Var* initData;  //初始数值
    bool inited;	//是否被初始化
    union 			//int 、val初值
    {
        int intVal;
        char charVal;
    };
    string strVal;   //字符串常量的值
    string ptrVal;	 //初始化字符指针常量字符串的名字
    Var *ptr;		//变量的指针变量
    int size;		//变量的大小
    int offset;		//变量的栈帧偏移量

    //内部使用函数
    void setExtern(bool ext);//设置extern
    void setType(Tag t);//设置类型
    void setPtr(bool ptr);//设置指针
    void setName(string n);//设置名字
    void setArray(int len);//设定数组
    void clear();//清除关键字段信息


public:
    Var(vector<int> &sp,bool ext,Tag t,bool ptr,string name,Var* init = NULL);//变量
    Var(vector<int> &sp,bool ext,Tag t,string name,int len);//数组
    Var(Token* lt);//设定字面量
    Var(int val);//设置整数变量
    Var(vector<int> &sp,Tag t,bool ptr);//临时变量
    Var(vector<int> &sp,Var* v);//拷贝
    Var();//void类型的变量

    bool setInit();//设定初始化
    Var* getInitData();//获得初始化变量数据
    vector<int>& getPath();//获取scopePath
    bool getExtern();//获取extern
    Tag getType();//获取类型
    bool isChar();//判断是否是字符变量
    bool isCharPtr();//判断字符指针
    bool getPtr();//获取指针
    bool getArray();//获取数组
    string getName();//获取名字
    string getPtrVal();//获取指针变量
    string getRawStr();//获取原始字符串值
    Var* getPointer();//获取指针
    void setPointer(Var* p);//设置指针变量
    string getStrVal();//获取字符串常量内容
    void setLeft(bool lf);//设置变量的左值属性
    bool getLeft();//获取左值属性
    void setOffset(int off);//设置栈帧偏移
    int getOffset();//获取栈帧偏移
    int getSize();//获取变量大小
    void toString();//输出信息
    void value();//输出变量的中间代码形式
    bool isVoid();//是void——唯一静态存储区变量getVoid()使用
    bool isBase();//是基本类型
    bool isRef();//是引用类型
    bool isLiteral();//是基本类型常量（字符串除外），没有存储在符号表，需要单独内存管理


    //数据流分析接口
    int index;//列表索引
    bool unInit();//是否初始化
    bool notConst();//是否是常量
    int getVal();//获取常量值
    bool live;//记录变量的活跃性

    //寄存器分配信息
    int regId;//分配的寄存器编号，-1表示在内存，偏移地址为offset!!!
    bool inMem;//被取地址的变量的标记，不分配寄存器！

};

class Fun{
    //基本信息
    bool externed;
    Tag type;
    string name;
    vector<Var*> paraVar;

    //临时变量地址分配
    int maxDepth;//栈的最大深度
    int curEsp;//当前栈的位置
    bool relocated;

    vector<int> scopeEsp;//作用域栈指针的位置


};
#endif //SIMPLE_C_COMPILER_SYMBOL_H
