# Sushiscript

[![Build Status](https://travis-ci.org/Sushiscript/sushiscript.svg?branch=master)](https://travis-ci.org/Sushiscript/sushiscript)
[![contributions welcome](https://img.shields.io/badge/contributions-welcome-brightgreen.svg?style=flat)](https://github.com/Sushiscript/sushiscrip/issues)

Sushiscript is new programming language based on shell, we want to offer a better experience when use shell.

## Build

**Prerequisite**

+ [conan](https://conan.io/)
+ [cmake(minimum version 3.5)](https://cmake.org/)

```bash
mkdir build

# if in ubuntu maybe install gtest need
conan remote add bincrafters https://api.bintray.com/conan/bincrafters/public-conan
conan install ..  -s compiler.libcxx=libstdc++11 --build=missing

cmake ..
make
```

## REFERENCE

+ [Coding Style](https://vinalx.github.io/articles/2018-03/google-cpp-style)