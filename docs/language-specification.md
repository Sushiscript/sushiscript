# Sushiscript Language Specification

## 1. Introduction

The specification contains standard guideline for implementing and understanding sushiscript. It's currently under construction and reflects the progress of implementation.

## 2. Lexical Structure

### 2.1 Identifier

```
<identifier> = <ident head> <ident tail>*
<ident head> = <letter> | '_'
<ident tail> = <digit> | <ident head>
```

#### 2.1.1 Keywords

Following identifiers are keywords:

```
or not and
define return
if else
switch case default
for in
redirect from to append
```

### 2.2 Punctuations

Following punctuations are meaningful:

```
+ - * / %
> < >= <= == !=
, = :
[ ] { } ( )
! $ " ' #
```

### 2.3 Built-in Types

```
<built-in type>
	= "Int" | "Bool" | "Unit" | "String"
	| "Path" | "Array" | "Map" | "ExitCode" | "FD"
```

### 2.4 Character

```
<line break> = [^\] '\n'
<whitespace> = ' ' | '\t'
```

##### 2.4.1 Raw character

Raw characters and tokens are only used in special contexts where getting rid of the "lexical restriction" conforms better to the tradition of shell and is more intuitive and convenient for language users.

```
<raw char> = <normal char> | <escape char>
<normal char> = [^<whitespace>"\]
<escape char> = '\' ( ' ' | '"' | '$' | '{' | '}' | '\')

<raw token> = <raw char>*
```

##### 2.4.2 Character inside string

_TODO: string escaped characters may be incomplete_

```
<string char>
	= <string normal char> | <string escaped char>
<string normal char>
	= <normal char>
	| <whitespace>
<string escaped char>
	= <escape char>
	| '\' ( 'a' | 'b' | 'f' | 'n' | 'r' | 't' | 'v' )
```

### 2.5 Interpolation

_TODO: Explain interpolation in later section and add a link here_

In lexical analysis, interpolations create a new context and suspend the previous recognizing sequence, typically used in [string literal](#String), [path literal](#Path) and other syntax structures.

```
<interpolation> = '${' ... '}'
```

Interpolation can be recognized recursively. e.g.

```
${ ... "${ ... }" ... }
```

### 2.6 Literals

#### 2.6.1 Bool & Unit & FD

```
<bool lit> = "true" | "false"
<unit lit> = "unit"
<fd lit> = "stdin" | "stdout" | "stderr"
```

#### 2.6.2 Integer

_TODO: integer literal of various radix support_

```
<integer> = ('+' | '-')? <digit>+
```

#### 2.6.3 String

```
<string> = '"' ( <string char> | <interpolation> ) * '"'
```

#### 2.6.4 Path

```
<path>        = <path start> ( <raw char> | <interpolation> )*
<path start>  = "./" | "../" | "/"
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

#### 3.1.1 Function Call

```
<function call> = <identifier> <expression>*
```

#### 3.1.2 Command

```
<command> = '!' ( <interpolation> | <raw token> | <paren expr> )*
```

#### 3.1.3 Operator Expression

```
<op expr> = <expression> <operator> <expression>
<operator>
	= '+' | '-' | '*' | '/' | '%'
	| '>' | '<' | '>=' | '<=' | '==' | '!='
```

### 3.2 Statement

