# Sushiscript Language Specification

## 1. Introduction

The specification contains standard guideline for implementing and understanding sushiscript. It's currently under construction and reflects the progress of implementation.

## 2. Lexical Structure

### 2.1 Identifier

```
<identifier> = <ident head> <ident tail>*
<ident head> = <letter> | '_'
<ident tail> = <digit>  | <ident head>
```

#### 2.1.1 Keywords

Following identifiers are keywords:

```
or not and
define return export
if else
switch case default
for in break continue
redirect from to append here
```

### 2.2 Punctuations

Following punctuations are meaningful:

```
+ - * // %
> < >= <= == !=
, = : ;
[ ] { } ( )
! $ " ' #
```

### 2.3 Built-in Types

```
<built-in type>
	= "Int"  | "Bool"  | "Unit" | "String"   | "Char"
	| "Path" | "Array" | "Map"  | "ExitCode" | "FD"
```

### 2.4 Interpolation

_TODO: Explain interpolation in later section and add a link here_

Interpolation is a lexical construct that embed expression in some special contexts (typically string interpolation). If  `identifier` is provided after `$` , lexer suspend the enclosing context and eagerly recognize the first possible identifier.

```
<interpolation> = '$' ( '{' ... '}' | <identifier> )
<interpolate char> = '$ | '{' | '}'
```

Interpolation can be recognized recursively. e.g.

```
${ ... "${ ... }" ... }
"${ ... \" ${ ... } \" ... }"
```

### 2.5 Character

```
<line break> = '\n'
<whitespace> = ' ' | '\t'
```

#### 2.5.1 Character Escaping

Special characters inside string-like token can be expressed by escaping character, by putting a `\\` before a character to escape it. There are different meaningful escaped character in different context (specified below).  **Escaping a character that doesn't have special meaning in current context preserves the original character.** In some contexts there are characters that have to be escaped to avoid lexical ambiguity.

_TODO: string escaped characters may be incomplete_

```
<tradition special> =
	'\' ('a' | 'b' | 'f' | 'n' | 'r' | 't' | 'v' | '\')
```


#### 2.5.2 Raw Mode

The raw mode of lexer is turned on in special contexts (typically when [invoking shell command](#3.1.4.2 Invoking Command)) where getting rid of the "lexical restriction" conforms better to the tradition of shell and is more intuitive and convenient for users.

```
<raw token>    = <raw char>*
<raw char>     = [^<raw restrict>] | <raw escape>
<raw restrict> = ' ' | ';' | '"' | "'" | '\'
<raw escape>   = '\' (<raw restrict> | <interpolate char>)
```

#### 2.5.3 Character inside `String` literal

```
<string char> = [^<string restrict>] | <string escape>
<string escape>
	= '\' ( <string restrict> | <interpolate char>)
	| <tradition special>
<string restrict> = '"'
```

#### 2.5.4 Character inside `Char` literal

```
<char char> = [^<char restrict>] | <tradition special>
<char restrict> = "'"
```

### 2.6 Literals

#### 2.6.1 Bool & Unit & FD

```
<bool literal> = "true" | "false"
<unit literal> = "unit"
<fd literal>   = "stdin" | "stdout" | "stderr"
```

#### 2.6.2 Integer

_TODO: integer literal of various radix support_

```
<integer> = ('+' | '-')? <digit>+
```

#### 2.6.3 Character

```
<char literal> = "'" <char char> "'"
```

#### 2.6.4 String

```
<string literal> = '"' ( <string char> | <interpolation> ) * '"'
```

#### 2.6.5 Path

```
<path literal> = <path start> ( <raw char> | <interpolation> )*
<path start>   = "./" | "../" | "/" | "~"
```

### 2.7 Misc

#### 2.7.1 Comment

When a hash tag `#` is recognized, any following character of the same line is regarded as comment and discarded by lexer.

```
<comment> = '#' <any character> $
```

#### 2.7.2 Indentation

Indent of length `N` is `N` consecutive white spaces at line start, no tab `\t` is allowed.

```
<indent N> = ^[ ]{n}
```

#### 2.7.3 Line Joining

When the last character of a line is `\`, **this backslash**, **next line break** and **indentation of next line** are ignored.

## 3. Syntax Structure

### 3.1 Expression

#### 3.1.1 Literals

```
<literal>
	= <int literal>    | <bool literal>
	| <unit literal>   | <fd literal>
	| <string literal> | <path literal>
	| <array literal>  | <map literal>
	| <char literal>
```

##### 3.1.1.1 Array Literal

```
<array literal> = '[' <array items>? ']'
<array items>   = <expression> (',' <expression>)*
```

##### 3.1.1.2 Map Literal

```
<map literal> = '{' <map items>? '}'
<map items>   = <map item> (',' <map item>)*
<map item>    = <expression> ':' <expression>
```

#### 3.1.2 Atom Expression

```
<atom expr>  = <literal> | <identifier> | <paren expr>
<paren expr> = '(' <expression> ')'
```

#### 3.1.3 Operator Expression

##### 3.1.3.1 Binary Operator Expression

```
<binop expr> = <expression> <binary op> <expression>
<binary op>
	= '+'  | '-' | '*'  | '/'  | '%'
	| '>'  | '<' | '>=' | '<=' | '==' | '!='
	| "or" | "and"
```

##### 3.1.3.2 Binary Operator Precedence Table

| Operator          | Associativity | Precedence |
| ----------------- | ------------- | ---------- |
| `or` `and`        | left          | 0          |
| `== ` `!=`        | left          | 1          |
| `>` `<` `>=` `<=` | left          | 2          |
| `+` `-`           | left          | 3          |
| `*` `/` `%`       | left          | 4          |

##### 3.1.3.3 Unary Operator

```
<unop expr> = <unary op> <expression>
<unary op> = '+' | '-' | "not"
```

#### 3.1.4 Procedure Call

##### 3.1.4.1 Function Call

```
<function call> = <identifier> <atom expr>*
```

##### 3.1.4.2 Invoking Command

```
<command> = '!' <comamnd param>* <command end>
<command param>
	= <interpolation>
	| <raw token>
	| <string literal>
	| <char literal>
<command end> = <line break> | ';'
```

##### 3.1.4.4 Redirection

```
<redirection>   = "redirect" <redirect item> (',' <redirect item>)*
<redirect item> = <fd literal>? <redirect action>
<redirect action>
	= "to" (<expression> "append"? | "here")
	| "from" <expression>
```

##### 3.1.4.5 Procedure Call

```
<procedure call> = ( <function call> | <command> ) <redirection>? <pipeline>?
<pipeline> = '|' <procedure call>
```

#### 3.1.5 Indexing

```
<indexing> = <expression> '[' <expression> ']'
```


#### 3.1.6 Expression

```
<expression>
	= <procedure call>
	| <atom expr>
	| <binop expr>
	| <unop expr>
	| <indexing>
```

#### 3.1.7 Type Expression

```
<type> = <simple type> | <complex type>
<simple type>
	= "Int"  | "Bool" | "String"
	| "Path" | "Unit" | "FD" | "ExitCode"
<complex type> = <array type> | <map type>
<array type>   = "Array" <atom type>
<map type>     = "Map" <atom type> <atom type>
<atom type>    = <simple type> | '(' <complex type> ')'
```

### 3.2 Statement

#### 3.2.1 Indentation Issue

If we say "`<statement>` introduces new block on `<program>`", all indentations of token in `<program>` must be greater than the line which the starting token of `<statement>` is in if `<program>` is in a different line, which means the following:

```
<indent N> <statement> ( <program> | <line break>
<indent (N+x)> <program> )
```

And all statements within the same innermost block must have same level of indentation.

#### 3.2.1 Definition

```
<definition> = <define var> | <define func>
```

##### 3.2.1.1 Variable Definition

```
<define var> =
	"export"? "define" <identifier> ( ':' <type> )? '=' <expression>
```

##### 3.2.1.2 Function Definition

```
<define func> =
	"export"? "define" <identifier> '(' <param list>? ')' '=' <program>
<param list>  = <param dec> (',' <param dec>)*
<param dec>   = <identifier> ':' <type>
```

`<define func>` introduces new block on `<program>`.

#### 3.2.2 Assignment

```
<assignment> = <identifier> '=' <expression>
```

#### 3.2.3 return

`return` is used for returning from the control flow of current enclosing function.

```
<return> = return <expression>
```

#### 3.2.4 if

```
<if>   = "if" <expression> ':' <program> <else>?
<else> = "else" ':'? <program>
```

`<if>` and `<else>` both introduce new block on corresponding `<program>`.

The `if` and its matching `else` must be in same level of indentation, if two or more unmatched `if`s are in the same indentation level, next `else` will match the closest `if`.

#### 3.2.5 switch

```
<switch>  = "switch" <expression> <case>+ <default>?
<case>    = "case" <expression> ':' <program>
<default> = "default" ':'? <program>
```

`<case>` and `<default>` both introduce new block on corresponding `<program>`.

#### 3.2.6 for

```
<for> = "for" <loop condition> ':' <loop body>
<loop condition>
	= <identifier> "in" <expression>
	| <expression>
<loop body> = ( <statement> | <break> | <continue> )*
```

`<for>` introduces new block on `<loop body>`.

##### 3.2.6.1 Loop Control

```
<break>    = "break" <integer>?
<continue> = "continue" <integer>?
```

#### 3.2.7 Program

```
<statement>
	= <expression>
	| <definition>
	| <assignment>
	| <if>
	| <switch>
	| <for>
<program> = <statement>*
```

## 4. Type System

## 5. Semantic

## 6. Translation

### 6.0 Translation example

```sushi
for i in [1, 2, 3, 4, 5]:
  ! ./random.out redirect to ./inputs/in$i
  ! ./a.out redirect to ./outputs/out$i, from ./inputs/in$i
```

Traslate to -->

```bash
for i in ($((0)) $((1)) $((2)) $((3)) $((5))); do
  ./random.out > ./inputs/in$i
  ./a.out > ./outputs/out$i < ./inputs/in$i
done
```

### 6.1 Definition

|type     |declare option	|
|:-------:|:-------------:|
|Int 			|-i            	|
|Bool			|								|
|String		|								|
|Path			|								|
|Unit			| 							|
|FD				|								|
|ExitCode |								|
|Array		|-a							|
|Map			|-A							|
|Function |-f							|

#### Variable Definition

`define` statement will be all translated into `local` statement
`export` statement will be all translated into `declare` statement, with option `-gx`

##### Example

```bash
# define a : Int = 10
local -i a=10
# define s : String = "abc"
local s="abc"
# export define ex : String = "def"
declare -gx ex="def"
# define arr : Array Int = [1, 2, 3]
local -ai arr=($((1)) $((2)) $((3)))
# define map : Map String String = { "a": "abc", "b": "def" }
local -A map=(
	["a"]="abc"
	["b"]="def
)
```

#### Function Definition

Both `define` and `export` statement will be translated into the following style

```bash
func_name() {
	local first_para_name=$0
	local second_para_name=$1
	# func body
	echo -ne "return_value"
}
```

Return value will be "returned" by using `echo -ne`.

But for `export`, an `export` follows the function.

##### Example

```bash
# define Add(a : Int, b : Int) : Int =
# 	return a + b
Add() {
	local -i a=$0
	local -i b=$1
	echo -ne $((a + b))
}

# export define Subtract(a : Int, b : Int) : Int =
# 	return a - b
Subtract() {
	local -i a=$0
	local -i b=$1
	echo -ne $((a - b))
}
export -f Subtract
```
