#include "arithmetic.h"

#include "enums.h"
#include "exceptions.h"

bool Arithmetic::isOperator(char c)
{
    return c == '+' || c == '-';
}

vector<Token> Arithmetic::convertToPostfix(vector<Token> input)
{

    vector<Token> result;
    stack<Token> stack;

    int rank = 0;

    for (Token next : input)
        if (next.getType() != TokenType::ARITHMETIC_OPERATOR) {
            result.push_back(next);
            rank++;
            if (!stack.empty()) {
                result.push_back(stack.top());
                stack.pop();
                rank--;
            }
        } else
            stack.push(next);

    if (rank != 1)
        throw AssemblyException("Can't process arithmetic expression");

    return result;

}

Token Arithmetic::returnToken(string data)
{
    Token t = Token::parse(data, 0, true);
    
    if (t.getType() == TokenType::ARITHMETIC_OPERATOR || 
        t.getType() == TokenType::IMMEDIATE_SYMBOL ||
        t.getType() == TokenType::IMMEDIATE_DECIMAL ||
        t.getType() == TokenType::IMMEDIATE_HEXADECIMAL ||
        t.getType() == TokenType::SYMBOL ||
        t.getType() == TokenType::DECIMAL ||
        t.getType() == TokenType::HEXADECIMAL
    )
        return t;

    throw AssemblyException("Can't process arithmetic expression");
}

vector<Token> Arithmetic::tokenize(string expression)
{
    vector<Token> result;
    string temp;

    if (isOperator(expression[0]))
        result.push_back(Token(TokenType::DECIMAL, "0"));

    for (char c : expression) {
        if (isOperator(c)) {

            if (temp != "")
                result.push_back(returnToken(temp));

            result.push_back(returnToken(string(1, c)));
            temp = "";

        } else
            temp += c;
    }

    if (temp != "")
        result.push_back(returnToken(temp));

    if (result.size() == 1) {
        result.push_back(Token(TokenType::ARITHMETIC_OPERATOR, "+"));
        result.push_back(Token(TokenType::DECIMAL, "0"));
    }
    
    return result;

}

unsigned long Arithmetic::calculateSymbolValue(vector<Token> tokens, SymbolTable* symbolTable, unsigned long section)
{
    char buffer[256];
    stack<Token> tmp;
    unsigned long rez = 0;

    for (Token& t : tokens)
    {
        
        if (t.getType() == TokenType::ARITHMETIC_OPERATOR)
        {
            Token op2 = tmp.top();
            tmp.pop();
            Token op1 = tmp.top();
            tmp.pop();

            const SymbolEntry* s2 = symbolTable->getEntryByName(op2.getValue());
            const SymbolEntry* s1 = symbolTable->getEntryByName(op1.getValue());

            if (
                (op1.getType() == TokenType::IMMEDIATE_SYMBOL && s1 == nullptr) ||
                (op2.getType() == TokenType::IMMEDIATE_SYMBOL && s2 == nullptr) ||
                (op1.getType() == TokenType::SYMBOL && s1 == nullptr) ||
                (op2.getType() == TokenType::SYMBOL && s2 == nullptr)
            )
                throw AssemblyException("Symbol is not found in symbol table when calculating symbol value");

            if (
                (s1 != nullptr && (s1->scope == Scope::LOCAL || s1->scope == Scope::GLOBAL) && !s1->defined) ||
                (s2 != nullptr && (s2->scope == Scope::LOCAL || s2->scope == Scope::GLOBAL) && !s2->defined) 
            )
                throw exception();

            unsigned long v2;
            if (s2)
                v2 = s2->value;
            else
                v2 = strtoul(op2.getValue().c_str(), NULL, 0);

            unsigned long v1;
            if (s1)
                v1 = s1->value;
            else
                v1 = strtoul(op1.getValue().c_str(), NULL, 0);

            if (t.getValue() == "-")
                rez = v1 - v2;
            else
                rez = v1 + v2;

            sprintf(buffer, "%lu", rez);

            tmp.push(Token(TokenType::IMMEDIATE_DECIMAL, string(buffer)));

        } else if (
            t.getType() == TokenType::IMMEDIATE_SYMBOL || 
            t.getType() == TokenType::IMMEDIATE_DECIMAL ||
            t.getType() == TokenType::IMMEDIATE_HEXADECIMAL ||
            t.getType() == TokenType::SYMBOL || 
            t.getType() == TokenType::DECIMAL ||
            t.getType() == TokenType::HEXADECIMAL
        )
            tmp.push(t);
    }

    rez = strtoul(tmp.top().getValue().c_str(), NULL, 0);
    tmp.pop();

    if (tmp.empty())
        return rez;
    else
        throw AssemblyException("Can't process given postfix expression.");

}