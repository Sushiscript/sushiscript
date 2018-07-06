# Sushiscript

[![Build Status](https://travis-ci.org/Sushiscript/sushiscript.svg?branch=master)](https://travis-ci.org/Sushiscript/sushiscript)
[![contributions welcome](https://img.shields.io/badge/contributions-welcome-brightgreen.svg?style=flat)](https://github.com/Sushiscript/sushiscrip/issues)

Sushiscript is new programming language based on shell, we want to offer a better experience when use shell.

## Build

**Prerequisite**
+ linux or unix OS
+ [conan](https://conan.io/)
+ [cmake (3.5 or higher)](https://cmake.org/)

```shell
$ mkdir build && cd build

# for ubuntu, mac os do not need "-s compiler.libcxx=libstdc++11"
$ conan remote add bincrafters https://api.bintray.com/conan/bincrafters/public-conan
$ conan install ..  -s compiler.libcxx=libstdc++11 --build=missing

# cmake and build
$ cmake .. && make install
```

## Browsing the Sources

`sushiscript` is implemented mostly in `c++14` and make a few uses of `boost::optional`, `boost::variant` and some utility functions like `format` and `join`.

All sources are located in `src/`, `include/` and `test/`. The structure of `include/`  shows the structure of modules, and although `src/` does follow the top level module structure, it doesn't really distinguish the source files of submodule in a certain module.

Except for testing sources, all header files (`*.h`) are under `include/`, all source files (`*.cpp`) are under `src/`

And here are the source files (directories) related to major parts of `sushiscript`.

- Frontend (lexer & parser): `{include, src}/sushi/lexer/`, `{include, src}/sushi/parser/`, `include/sushi/ast/`
- AST Analyze (scoping & type checking): `{include, src}/sushi/scope/`, `{include, src}/sushi/type-system/`, `include/sushi/ast/`
- Code Generation: `include/sushi/type-system/type.h`, `include/sushi/scope/scope.h`, `{include, src}/sushi/code-generation/`, `include/sushi/ast/`
- Main: `{include, src}/pipeline/`, `src/sushi/main.cpp`

## Syntax Quick Guide

Here we take a quick tour over `sushiscript`'s basic syntax. For detailed information, consult [language specification](./docs/language-specification.md)

### Comments

```sushi
# this is comment
```

### Basic Expressions

### Literals

```
1; 100; 0     # integer
"hello world" # string
true; false   # boolean
./hello/world # relative path
/hello/world  # absolute path
'a'; 'b';     # character
stdin; stdout; stderr # file descriptor
```

#### Arithmetic

```sushi
<unary op> 1 # <unary op> is +(abs value), -(negate)
1 <arith op> 1 # <arith op> is +, -, *, //(int division), %(mod) 
1  <rel op>  1 # <rel op>   is >, <, <=, >=, ==, !=
```

#### String Operation

```sushi
"hello " + "world" # concatenation
"hi " * 4          # "hi hi hi hi "
"hi " <rel op> "hello" # lexical comparison 
"hello world"[0]   # indexing
```

#### Path Operation

```
./hello // ./world # ./hello/world
/hello // ./world # ./hello/world
/hello // /world # ERROR, rhs has to be relative path
```

#### Command

```
! echo "hello world"
```

#### Array

```
{1, 2, 3} # array of int
{"hi", "world"}  # array of string
{"hello", 123}   # ERROR, only homogeneous array allowed
{{1, 2}, {5, 6}} # ERROR, nested array not allowed
{1, 2, 3, 4}[0]  # indexing 
```

#### Map

```
{1: 2, 3: 4}             # map from int to int
{"haha": 1, "hihi": 2}   # map from string to int
{1: {1, 2, 3}}           # ERROR, cannot map to/from array or map
{"ha": 1, "hi": 2}["ha"] # indexing
```

### Basic Statements

#### Types

```
() # unit
Bool, Char, Int, Bool
Fd      # file descriptor
Path    # absolute Path
RelPath # relative path)
ExitCode
Array Int   # array of Int
Map Int Int # map from int to int 
```

#### Variable Definition

```
define a: Int = 1
define b: String = "hahaha"
define c: Array Int = {1, 2, 3}
define d = ./hello/world # deduced d: RelPath
```

#### Function

```
# definition
define Add(a: Int, b: Int, c: Int): Int =
	return a + b + c
define Hello() = ! echo "hello"
# call
define sum = Add 100 200 300
Hello() # calling function with no parameter
```

#### Assignment

```
define a = 1
a = 2 # assign to variable
define b = {1, 2, 3}
b[2] = 2 # assign to indexing
define c = "hello"
c[0] = "H"
```

#### if

```
if 1 < 2:
	! echo "one less than two" # indent new block
else if 1 > 2:
	! echo "one greater than two"
else: ! echo "one equal to two" # or same line

if ! find some file: # command as condition
	echo "file found!"
# or...
define success = ! find some file
if success: echo "file found!"
```

#### switch

```
define a = 1
switch a + 1
case 1:
	! echo "one plus one is one"
case 2:
	! echo "one plus one is two"
default:
	! echo "one plus one is else"
```

#### for

```
define a = 0

for a < 1024: # with only loop condition
	a = a + a

for i in {1, 2, 3, 4}: # array based range loop
	a = a + i

for true:
	if a == 2: continue
	if a > 1024: break
	a = a + a
```

### More on String and Command

#### Interpolation

```
define name = "Sushi"
define a: String = "hello ${name}" # interpolation
define b: String = "say ${"hello ${name}" + "!"} # nested interpolation

# interpolation also worked in...
define c: Path = /hello/${name}
define d: RelPath = ./hello/${name}

! echo hello name    # hello name
! echo hello ${name} # hello Sushi
```

#### Redirection

```
# redirecting command, note the ',' after command
! cat somefile, redirect to ./another/file
! cat, redirect from ./input-file, to ./output-file

! echo hello, redirect to ./out
# grabbing command output, "here" is a keyword
define content: String = ! cat ./out, redirect to here

# function calls are redirectable, but no need for ','
define f() = echo "hello"
f() redirect to ./out
```

#### Pipe

```
! find . | ! sort | ! xargs cat
function call | ! sort | anotherfunction call
```