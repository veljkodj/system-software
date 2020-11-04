#ifndef ARITHMETIC_H
#define ARITHMETIC_H

#include "token.h"
#include "structures.h"
#include <iostream>
#include <stack>
#include <string>
#include <vector>
using namespace std;

class Arithmetic
{
public:

    static bool isOperator(char c);
    static Token returnToken(string data);

    static vector<Token> convertToPostfix(vector<Token> input);
    static vector<Token> tokenize(string expression);
    static unsigned long calculateSymbolValue(vector<Token> tokens, SymbolTable* symbolTable, unsigned long section = -1);

};

#endif