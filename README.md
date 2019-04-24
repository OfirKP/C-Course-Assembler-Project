# C-Course-Assembler-Project
The final assignment (#14) of the C course (20465) at The Open University.  
It received a grade of 100.

This project is based on the **_two-pass assembler_** model.  
**Note:** the computer model for this project and the given assembly language are **imaginary**.

## Getting Started

The project was coded and compiled using Ubuntu, but it may run on all Linux versions.

### Usage

A step by step series of examples that tell you how to get a development env running

Use makefile to compile the project like this:
```
>   make
```

After preparing assembly files **with an `.as` extension**, open *terminal* and pass file names as arguments (without the file extensions) as following:

As for the files x.as, y.as, hello.as we will run:
```
>   assembler x y hello
```
The assembler will generate output files with the same filenames and the following extensions:  
- `.ob` - Object file
- `.ent` - Entries file
- `.ext` - Externals file

An example of input and output files can be found under the 'tests' folder.

## Hardware
- CPU
- RAM (including a stack), with the size of 256 *words*.
- A *word*'s size in memory is **10 bits**.
- Uses signed *2's complement* arithmetic for integers (with no support for real numbers).

   ### Registers
   The CPU has 9 registers: 8 general registers (named r1,r2,...,r8) and a register called *PSW* (program status word).  
   The size of each register is a *word* (10 bits).  

## Instruction Structure
Every software *instruction* is encoded into a few *words* in memory (max is 5 *words*).
The first word is of the following structure:

|  9&emsp;8&emsp;7&emsp;6  |  5&emsp;4  |  3&emsp;2  |  1&emsp;0  |
| ------------------------ | ---------- | ---------- | ---------- |
|          Opcode          | Source operand addressing method | Destination operand addressing method | E, R, A

Encoding of each instruction word is done using an **imaginary** base32 as defined here:
```
0: !   
1: @
2: #
3: $
4: %
5: ^
6: &
7: *
8: <
9: >
10: a
11: b
12: c
13: d
14: e
15: f
16: g
17: h
18: i
19: j
20: k
21: l
22: m
23: n
24: o
25: p
26: q
27: r
28: s
29: t
30: u
31: v
```
so that each *word* can be encoded as 2 digits in this base (with leading ! as needed).

## Commands
The commands allowed in bits 6-9 are:

| Opcode (decimal) | Command Name |
| ---------------- | ------------ |
|	0	|	mov	|
|	1	|	cmp	|
|	2	|	add	|
|	3	|	sub	|
|	4	|	not	|
|	5	|	clr	|
|	6	|	lea	|
|	7	|	inc	|
|	8	|	dec	|
|	9	|	jmp	|
|	10	|	bne	|
|	11	|	red	|
|	12	|	prn	|
|	13	|	jsr	|
|	14	|	rts	|
|	15	|	stop	|

## Directives
A **_directive_** line of the following structure:

1. An **optional** preceding *label*. e.g. `PLACE1: `.
2. A _directive_: `.data`, `.string`, `.struct`, `.entry` or `.extern`.
3. Operands according to the type of the *directive*.

   ### `.data`
   This directive allocates memory in the data image for storing received integers later in memory (also increases the _data counter_ and updates the _symbol table_).
   The parameters of `.data` are valid integers (separated by a comma).  
   e.g. `LABEL1: .data      +7, -56, 4, 9`.
   
   ### `.string`
   This direcive receives a string as an operand and stores it in the data image. It stores all characters by their order in the string, encoded ny their *ASCII* values.
   e.g. `STRING1: .string "abcdef"` is a valid directive.
   
   ### `.struct`
   This directive allocates memory for a *structure* consists of 2 fields: the first is an integer and the second is a string.
   e.g. `STRUCT4: .struct 4, "abc"` is a vaild directive.
   
   ### `.entry`
   This directive outputs a received name of a *label* to the *symbol table*, so that later it will be recognized by other assembly files (and they would be able to use it).
   e.g. 
   ```
   ; file1.as
   .entry HELLO
   HELLO: add #1, r1 
   ```
   ### `.extern`
   This directive receives a name of a *label* as a parameter and declares the *label* as being external (defined in another file) and that the current file shall use it.  
   This way, the directive `.extern HELLO` in `file2.as` will match the `.entry` directive in the previous example.
