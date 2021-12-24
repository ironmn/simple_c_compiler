cmake_minimum_required(VERSION 3.20)
project(simple_C_compiler)


set(CMAKE_CXX_STANDARD 11)

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

set( test_files
        test/scanner_test.txt
        test/lexer_correct_test.txt
        )

foreach(file ${test_files})
    file(COPY ${file} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endforeach()


add_executable(simple_C_compiler gtest.cpp common.h token.h token.cpp
        lexer.h lexer.cpp
        error.h error.cpp
        compiler.h compiler.cpp
        symbol.cpp symbol.h
        symtab.cpp symtab.h
        # parser.cpp parser.h

        )


target_link_libraries(simple_C_compiler gtest_main pthread)


include(GoogleTest)
gtest_discover_tests(simple_C_compiler)
