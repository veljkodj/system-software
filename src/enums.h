#ifndef ENUMS_H
#define ENUMS_H

#define ASM_UNDEFINED -1

typedef unsigned long IdSymbol;
typedef unsigned long IdSection;
typedef unsigned long IdRelocation;

enum Scope : int
{
    LOCAL,
    GLOBAL,
    EXTERN
};

enum TokenType
{
    INVALID,
    ACCESS_MODIFIER,
    LABEL,
    SECTION,
    DIRECTIVE,
    INSTRUCTION,
    END_OF_SECTIONS,

    ASTERISK_SYMBOL, 
    IMMEDIATE_SYMBOL, 
    SYMBOL, //possible two addressing types
    ASTERISK_DECIMAL, 
    IMMEDIATE_DECIMAL, 
    DECIMAL, //possible two addressing types
    ASTERISK_HEXADECIMAL, 
    IMMEDIATE_HEXADECIMAL, 
    HEXADECIMAL, //possible two addressing types
    REGISTER_DIRECT, 
    PC_RELATIVE, 
    REGISTER_INDIRECT, 

    ARITHMETIC_OPERATOR,
    ARITHMETIC_EXPRESSION

};

enum OperandSize
{
    BYTE,
    WORD
};

enum RelocationType : int
{
    R_386_16,
    R_386_PC16
};

#endif