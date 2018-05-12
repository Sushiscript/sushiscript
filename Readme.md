# Sushiscript

Sushiscript is new programming language based on shell, we want to offer a better experience when use shell.

## Build

**Prerequisite**

+ [conan](https://conan.io/)
+ [cmake(minimum version 3.5)](https://cmake.org/)

```bash
mkdir build
conan install ..

# if in ubuntu maybe install gtest need
conan remote add lasote https://api.bintray.com/conan/lasote/conan-repo
conan install .. -s compiler=gcc -s compiler.libcxx=libstdc++11 --build gtest

cmake ..
make
```

## REFERENCE

+ [Coding Style](https://vinalx.github.io/articles/2018-03/google-cpp-style)