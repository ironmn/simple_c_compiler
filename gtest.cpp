#include <gtest/gtest.h>
#include<iostream>
#include <fcntl.h>
#include "lexer.h"
#include "common.h"

//Class Scanner . the scan method passed
TEST(Scanner, scan){
    Scanner scan("scanner_test.txt");
    ASSERT_EQ('#',scan.scan());

    ASSERT_EQ('i',scan.scan());
    ASSERT_EQ('n',scan.scan());
    ASSERT_EQ('c',scan.scan());
    ASSERT_EQ('l',scan.scan());
    ASSERT_EQ('u',scan.scan());
    ASSERT_EQ('d',scan.scan());
    // ASSERT_EQ('1',scan.scan());
}

//Scanner :: getFile method passed
TEST(Scanner, getfilename){
    Scanner scanner("scanner_test.txt");
    ASSERT_EQ(0,strcmp("scanner_test.txt",scanner.getFile()));
}

//Lexer :: scan method passed
TEST(Lexer, scan){
    Scanner scanner("scanner_test.txt");
    Lexer lexer(scanner);
    ASSERT_EQ(lexer.scan('#'),true);
}

//lexer :: getTag test passed
TEST(Keywords,getTag){
    Keywords k;
    ASSERT_EQ(k.getTag("while"),KW_WHILE);
    ASSERT_EQ(k.getTag("for"),KW_FOR);
    ASSERT_EQ(k.getTag("sdfasfaf"),ID);
}

//Lexer::tokenize test passed(for only correct testcases)

TEST(Lexer, tokenize){
    Scanner scanner("lexer_correct_test.txt");
    Lexer lexer(scanner);
    //keywords tests passed
    ASSERT_EQ(lexer.tokenize()->tag,KW_WHILE);
    ASSERT_EQ(lexer.tokenize()->tag,KW_FOR);
    ASSERT_EQ(lexer.tokenize()->tag,KW_SWITCH);
    //ID tests passeed
    ASSERT_EQ(lexer.tokenize()->tag,ID);
    ASSERT_EQ(lexer.tokenize()->tag,ID);
    ASSERT_EQ(lexer.tokenize()->tag,KW_CHAR);
    ASSERT_EQ(lexer.tokenize()->tag,STR);
    //NUM tests passed
    Token* nt = lexer.tokenize();
    ASSERT_EQ(nt->tag,NUM);
    Token* nt2 = lexer.tokenize();
    ASSERT_EQ(nt2->tag,NUM);
    ASSERT_EQ(lexer.tokenize()->tag,NUM);
    ASSERT_EQ(lexer.tokenize()->tag,NUM);

    //char tests   passed
    ASSERT_EQ(lexer.tokenize()->tag,CH);
    ASSERT_EQ(lexer.tokenize()->tag,CH);
    ASSERT_EQ(lexer.tokenize()->tag,CH);
    //operator test
    ASSERT_EQ(lexer.tokenize()->tag,ADD);
    ASSERT_EQ(lexer.tokenize()->tag,INC);
    ASSERT_EQ(lexer.tokenize()->tag,SUB);
    ASSERT_EQ(lexer.tokenize()->tag,DEC);
    ASSERT_EQ(lexer.tokenize()->tag,MUL);
    ASSERT_EQ(lexer.tokenize()->tag,DIV);
    ASSERT_EQ(lexer.tokenize()->tag,MOD);
    ASSERT_EQ(lexer.tokenize()->tag,GE);
    ASSERT_EQ(lexer.tokenize()->tag,LE);
    ASSERT_EQ(lexer.tokenize()->tag,LT);
    ASSERT_EQ(lexer.tokenize()->tag,GT);
    ASSERT_EQ(lexer.tokenize()->tag,LEA);
    ASSERT_EQ(lexer.tokenize()->tag,AND);
    ASSERT_EQ(lexer.tokenize()->tag,OR);
    ASSERT_EQ(lexer.tokenize()->tag,COMMA);
    ASSERT_EQ(lexer.tokenize()->tag,SEMICON);
    ASSERT_EQ(lexer.tokenize()->tag,COLON);
    ASSERT_EQ(lexer.tokenize()->tag,LPAREN);
    ASSERT_EQ(lexer.tokenize()->tag,RPAREN);
    ASSERT_EQ(lexer.tokenize()->tag,LBRACK);
    ASSERT_EQ(lexer.tokenize()->tag,RBRACK);
    ASSERT_EQ(lexer.tokenize()->tag,LBRACE);
    ASSERT_EQ(lexer.tokenize()->tag,RBRACE);
    ASSERT_EQ(lexer.tokenize()->tag,KW_INT);
}
int main() {
    testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}
