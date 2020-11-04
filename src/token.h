#ifndef TOKEN_H
#define TOKEN_H

#define NUMBER_OF_PARSERS 13
#define ARITHMETIC_DELIMITER "+-"

#include <iostream>
#include <regex>
#include <string>
#include "enums.h"
using namespace std;

static const regex parsers[NUMBER_OF_PARSERS] = {

    regex("^\\.(global|extern)$"),

    regex("^([a-zA-Z][a-zA-Z0-9_]*):$"),

    regex("^\\.section$"),

    regex("^\\.(byte|equ|skip|word)$"),

    regex("^(halt|ret|iret|int|jmp|jeq|jne|jgt|call|(not|push|pop|xchg|mov|add|sub|mul|div|cmp|and|or|xor|test|shl|shr)(b|w){0,1})$"),	

    regex("^.end$"),
            
    regex("^(\\+|\\-){1}$"),

    regex("^[a-zA-Z][a-zA-Z0-9_]*$"), 

    regex("^(\\-|\\+){0,1}[0-9]+$"), 

    regex("^0x[0-9a-fA-F]{1,}$"),

    regex("^%r([0-7]|15)(h|l){0,1}$"),

    regex("^[a-zA-Z][a-zA-Z0-9_]*\\(%r7\\)$"),

    regex("^([a-zA-Z][a-zA-Z0-9_]*|(\\-|\\+){0,1}[0-9]+|0x[0-9a-fA-F]{1,}|)\\(%r([0-7]|15)(h|l){0,1}\\)$")

};

class Token {

public:

    Token() {};
    Token(TokenType type, string value) : type(type), value(value) {}

    TokenType getType() const;
    string getValue() const;

    static Token parse(string data, unsigned long line, bool recursive);

private:

    TokenType type;
    string value;

};

#endif