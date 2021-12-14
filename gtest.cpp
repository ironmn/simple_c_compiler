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

int main() {
    testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}
