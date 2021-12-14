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
    Var *prt;		//变量的指针变量
    int size;		//变量的大小
    int offset;		//变量的栈帧偏移量

    //内部使用函数
    void setExtern(bool ext);//设置extern
    void setType(Tag t);//设置类型
    void setPtr(bool ptr);//设置指针
    void setName(string n);//设置名字
    void setArray(int len);//设定数组
    void clear();//清除关键字段信息
};

class Fun{
    //基本信息
    bool externed;
    Tag type;
    string name;
    vector<Var*> paraVar;

    //临时变量地址分配
    int maxDepth;
    int curEsp;
    bool relocated;

    vector<int> scopeEsp;

};
#endif //SIMPLE_C_COMPILER_SYMBOL_H
