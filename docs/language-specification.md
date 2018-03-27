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
+ - * /
> < >= <= == !=
, = :
[ ] { } ( )
! $ " '
```

### 2.3 Built-in Types

```
<built-in type>
	= "Int" | "Bool" | "Unit" | "String"
	| "Path" | "Array" | "Map" | "ExitCode" | "FD"
```

### 2.4 Literals

#### 2.4.1 Bool / Unit / FD

```
<bool lit> = "true" | "false"
<unit lit> = "unit"
<fd lit> = "stdin" | "stdout" | "stderr"
```

#### 2.4.2 Integer

_TODO: integer literal of various radix support_

```
<integer> = <digit>+
```

#### 2.4.3 Character

_TODO: escaped characters may be incomplete_

```
<line break> = [^\] '\n'
<whitespace> = ' ' | '\t'

<normal char> = [^<whitespace>"\]
<enclosed normal char>
	= <normal char>
	| <whitespace>

<escape char> = '\' ( ' ' | '"' | '$' | '{' | '}' | '\')
<enclosed escaped char>
	= '\' ( 'a' | 'b' | 'f' | 'n' | 'r' | 't' | 'v' )
	| <escape char>
```

#### 2.4.4 String

```
<string> = '"' ( <enclosed normal char> | <enclosed escaped char> )* '"'
```

#### 2.4.5 Path

```
<path>        = <path start> ( <escaped char> | <normal char> )*
<path start>  = "./" | "../" | "/"
```

### 2.5 Misc

#### 2.5.1 Indentation

Indent of length `N` is `N` consecutive white spaces at line start, no tab `\t` is allowed.

```
<indent N> = ^ {n}
```

#### 2.5.2 Line Joining

When the last character of a line is `\`, **this backslash**, **next line break** and **indentation of next line** are ignored.

## 3. Syntax Structure

### 3.1 Expression

### 3.2 Statement

