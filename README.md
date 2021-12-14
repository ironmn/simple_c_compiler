# simple-c-compiler
A compiler to subset of C programming language by C++


## build googletest on your local machine

On Ubuntu 20.04LTS:

1. Install dependency

```bash
sudo apt-get install libgtest-dev
sudo apt-get install cmake # install cmake
```

2. Download the source code of gtest, then cd into the top-level directory.
```bash
git clone https://github.com/google/googletest.git
```
3. Enter the command as follows:
```bash
# Compile the source code
sudo cmake CMakeLists.txt
sudo make

cd lib
# Copy or symlink libgtest.a and libgtest_main.a to your /usr/lib folder
sudo cp *.a /usr/lib
```
After these steps, you have installed gtest as link library locally.

## Compile the whole project and run

The easiest way is to click run button in CLion.
Or maybe you can try to comile by CLI.

Enter the command as follows:
```bash
cd build

sudo cmake ../
sudo make 
```
Then you will find out an ELF formatted file named simple-C-compiler. Run it:
```bash
./simple-C-compiler
```
And you will see the result.

## Components to be added:
* symtab
* a syntex-directed translator
* intermedia code generator
* linker
...

Welcome to communicate.
