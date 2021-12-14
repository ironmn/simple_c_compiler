//
// Created by lelouch on 2021/12/14.
//

#ifndef SIMPLE_C_COMPILER_TOKEN_H
#define SIMPLE_C_COMPILER_TOKEN_H

#pragma once
#include "common.h"
class Token{
public:
    Tag tag;
    Token (Tag t);
    virtual string toString();
    virtual ~Token();
};

class Id : public Token{
public:
    string name;
    Id(string n);
    virtual string toString();
};

class Num : public Token{
public:
    int val;
    Num (int v);
    virtual string toString();
};

class Char : public Token{
public:
    char ch;
    Char (char ch);
    virtual string toString();
};

class Str : public Token{
public:
    string str;
    Str (string s);
    virtual string toString();
};
#endif //SIMPLE_C_COMPILER_TOKEN_H
