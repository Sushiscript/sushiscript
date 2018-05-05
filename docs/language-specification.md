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

Interpolation is a lexical construct that embed expression in some special contexts (typically string interpolation). If  `identifier` is provided after `$` , lexer suspend the enclosing context and eagerly recognize the first possible identifier.

```
<interpolation> = "${" ... "}"
```

Interpolation can be recognized recursively. e.g.

```
${ ... "${ ... }" ... }
"${ ... "${ ... }" ... }"
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
<path literal> = <path start> ("/" ( <raw char> | <interpolation> )* )?
<path start>   = "." | ".." | "~"
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

When the last character of a line is `\`, **this backslash**, **next line break** and **indentation of next line** are ignored. And if the current line is empty, the only the indentation of next line will count.

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
	= '+'  | '-' | '*'  | '//'  | '%'
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

### 4.1 Introduction

The type system provide abstraction over many operating system concepts such as "Path" in filesystem and  the exit status of a process and distinguish one of those concepts from another on the aspect of language although their low level representations may be the same.
`sushiscript` is designed to be a staticly and strongly typed language. "Static" means that the type checking process on the type correctness of program is performed at compile time by the _type checker_. And "strong" means that there are few implicit conversions between different types are tried by type checker when the actual type of an expression and expected type are not exactly the same. (there are some, though).
The type system also has a strong connection with the syntax structure, every expression has a unambiguous type that depend on the context. And most `<expression>`s  appeared in the syntax structure means "expression of some certain types", expressions that aren't in the list of that "certain types" are rejected even if they are valid in syntax. And the requirements of types often distinguish different semantics of syntax structure.

### 4.2 Notations and Terms

#### 4.2.1 `<E>` and `<S>`

We use `monospaced` string and angle bracket around them to represent some expandable syntactic rules. And `<E>` usually means some concrete `<expression>` and `<S>` usually means some concrete `<statement>`.

#### 4.2.2 `<E : T>`

When we add colon between the angle bracket like `<E : T>`. We means expression `<E>` has type `<T>`. It may be useful to explicity assign a type for the expression.

#### 4.2.3 Simple Types

Simple types (or atom types) are types that doesn't contains other types in itself. 

#### 4.2.4 Parameterized Type

Parameterized type (as its name suggest) can be provided with type parameters to form (intuitively) different types with different parameters.

#### 4.2.5 Instantiation / Instantiate

An instantiation of a type parameter is the process where type parameter is replaced by a specific type when the actual type of the expression is known somehow from the context.

#### 4.2.5 Direct Sub-expresion

A expression `<E>` is a direct sub-expressions of a statement or an expression `<R>` if it's the first `<expression>` appeared in some path while walking down the parsing tree rooted at `<R>`.

#### 4.2.6 Super-expression

Opposite to "sub-expression", if `<E'>` is a sub-expression to `<E>`, then `<E>` is a super-expression of `<E'>`.

#### 4.2.6 Implicitly Convertible

Type `T` is_ implicitly convertible_ to type `T'` when there are built-in ~~or user-provided~~ ways to transform an object of type `T` to another object of type `T'`. And intuitively the process could be done implicitly, with no additional code written.

### 4.3 Type Checking

#### 4.3.1 Well-typed Expression

The  _well-typed_ expression is recursively defined as follow:

- A literal of simple type is a well-typed expression
- An defined identifier is a well-typed expression
- Other expressions are well typed if all direct sub-expressions are well typed, and their types together satisfy the [_typing rules_](#4.4 Typing Rule) of that composite expression.

#### 4.3.2 Type Requirement

A composite expression `<E : T>` or a statement `<S>` may explicitly _requires_ some of their direct sub-expressions `<E'>` to have some certain possible types `T1', T2', ...` in their typing rules R. Then these variable together forms a type requirement, and `T1', T2', ...` are the  _required types_ by `<E : T>` or `<S>` on `<E'>` in that requirement.

Sometimes the required types by `<E>` or `<S>` on `<E'>`may also depend on the types of other direct sub-expressions of `<E>` or `<S>` due to the typing rule.

#### 4.3.3 Implicit Type Conversion

With `<E'>` being an direct sub-expression of `<E : T>` or `<S>`, and its actual type being `T'`, an _implicit type conversion_(or _coersion_) happens when `T'` isn't one of the required types in the corresponding type requirement. And `T'` is _implicitly convertible_ to exactly one type `T''` in the required types. Then we say _`<E' : T'>` successfully implicitly convert to `T''` in the requirement_.

If there're multiple implicitly convertible type in required types, then conversion is considered ambiguous, which results in a fail implict conversion.

#### 4.3.4 Satisfaction Condition

We say _`<E' : T'>` satisfy the requirement by `<E : T>` or `<S>` on `<E'>`_ if one of the followings is true:

- `T'` is a same type as one of the required types by `<E>` or `<S>` on `<E'>`.
- `<E' : T'>` successfully convert to `T` in the requirement.
- ~~There's only one type in the required types, and `T'` unifies with that type. ~~

#### 4.3.3 Type Correctness

A _type correct program_(i.e. a program that pass the type checking process) means that type requirements recursively generate by **all** statements and composite expressions are _satisfied_ by their corresponding direct sub-expressions.

### 4.4 Typing Rule

Typing rules, as mentioned in sections above, are the most important source of type requirements. And they usually come along with the semantic of the program. So they would be introduced together.

#### 4.4.1 Notation

We use a notation connected intimately with the notation that expressed syntax structure:

```
<expression : T> = <[optional-name :] T1> other tokens <T2>
# or
<statement> = <[optional-name :] T1> other tokens <T2 | T3>
```

where those enclosing in angle bracket are direct sub-expressions of the expression or statement with an optional name and type `T1, T2, T3, ...`. `T1, T2, T3, ...` may be specific types or just type pameters, either way, types with same name are the same type in the notation. Other tokens are no-longer enclosing in double or single quotes for clarity.

And what's on the left of the equal sign (`=`) of the rule is called the _head_ of the rule, the right is the _body_ of the rule.

#### 4.4.2 Type Deduction

Type deduction is performed **mostly bottom-up** and **always left-to-right** with respect to the abstract syntax tree. That is the type of the actual type of a super-expression is obtained by the type of  its sub-expressions and the applying typing rule.

#### 4.4.3 Requirement Generation

Whenever a type parameter in a rule is successfully deduced with a actual type `T`, all other type parameter with the same name (both in the _head_ and _body_) is instantiated with `T`.

A specific type `<E' : T'>` in the _body_, whenever it is instantiated, add `T'` to the required types by `<E : T>` on `<E'>`.  For example:

```
<E : T> = <E' : T'> <E'' : T'>
```

When `<E'>` is deduced with actual type `TA`, it instantiates the `T'` to `TA` in `<E''>`, and add `TA1` to the required types of `<E''>`. Then when `<E''>` is deduced with `TA2`, `<E'' : TA2>` is checked against the _satisfaction_ condition of the requirement with required type `TA1`.

### 4.5 Buit-in Types

#### 4.5.1 Simple Types

- `Unit`: "Trivial" type with only one value (`unit`)
- `Int`: Signed integer  _todo: specify the range of Int_
- `Bool`:  Boolean types with only two different values (`true` and `false`)
- `Char`: Character _todo: specify the range of Char_
- `String`: Character sequence.
- `Path`: Path may or may not pointing to a valid file in file system.
- `ExitCode`: Exit status of a process or command.
- `FD`: File descriptor is the logical handle of a opened file.

#### 4.5.2 Parameterized Types

- `Array [element]`: Linear sequence of values with type `[Element]`.
- `Map [Key] [Value]`: Associative structure that map a values of type `[Key]` to values of type `[Value]`
- `Function [Ret] [Param1] [Param2] ...`: Function with parameters of type `[Param1], [Param2]` and return type `[Ret]`.

#### 4.5.3 Implicit Conversion

- `ExitCode` is _implicitly convertible_ to `Bool`, with non-zero `ExitCode` to `false` (exit with error), zero `ExitCode` to `true` (success).
- `ExitCode` is _implicitly convertible_ to `Int`, converting the exit status code to the integer of same value.

### 4.6 User-defined Types

_Not (being planned to) implement yet_

### 4.7 Typing Rules in `sushiscript`

#### 4.7.1 Array Literal

```
<array literal : Array T> = [<T>, <T>, ...] | []
```

Array Literal is of type `Array T` when all of it's element are well typed and of same type `T`.

#### 4.7.2 Map Literal

```
<map literal : Map K V> = {<K> : <V>, <K> : <V>, ...} | {}
```

Map Literal is of type `Map K V` when all of it's keys and values are well typed and all keys are of the same type `K`, all values are of the same type `V`.

#### 4.7.3 Special Satisfaction Condition for Empty Array and Map

Empty array and map literals satisfy all requirement whose required types contains an array or map with any type parameter combination.

#### 4.7.4 Unary Operations

##### `+`

```
<abs : Int> : + <Int>
```

`+` as a unary operator turns an integer to its absolute value.

##### `-`

```
<negate : Int> = - <Int>
```

`-` as a unary operator turns an integer to its negative counter-part.

##### `not`

```
<logical negate : Bool> = not <Bool>
```

`not` negate boolean value to another different one.

#### 4.7.5 Binary Operations

##### `+`

```
<int plus : Int>         = <Int>     + <Int>
<string concat : String> = <String>  + <String>
<array concat : Array T> = <Array T> + <Array T>
<map merge : Map K V>    = <Map K V> + <Map K V>
```

- `<int plus>` performs integer addition
- `<string concat>` concatenates two strings producing a new string
- `<array concat>` concatenates two array of same element type producing a new array
- `<map merge>` combines two maps, with keys on the right overwrite the value of the same key on the left.

##### `-`

##### `*`

##### `//`

##### `%`

##### `==` `!=`

##### `>` `<`

##### `>=` `<=`

##### `or` `and`

#### 4.7.6 Indexing

#### 4.7.7 Function Call

#### 4.7.8 Redirection

#### 4.7.9 Definition

#### 4.7.10 Assignment

#### 4.7.11 Return

#### 4.7.12 If

#### 4.7.13 Switch

#### 4.7.14 Loop

## 5. Semantic

## 6. Translation

