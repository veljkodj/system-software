#include "token.h"

#include <string.h>
#include <assert.h> 

#include "exceptions.h"

TokenType Token::getType() const {
    return type;
}

string Token::getValue() const {
    return value;
}

Token Token::parse(string str, unsigned long line, bool recursive = true) {

    if (str.size() == 0)
        return Token(TokenType::INVALID, 0);

    string data = str;

    data = regex_replace(data, regex("%sp"), "%r6");
    data = regex_replace(data, regex("%pc"), "%r7");
    data = regex_replace(data, regex("%psw"), "%r15");

    bool isImmediate = false;
    bool isAsterisk = false;

    if (data.at(0) == '*')
    {
        data = data.substr(1, data.size() - 1);
        isAsterisk = true;
    }
    else if (data.at(0) == '$')
    {
        data = data.substr(1, data.size() - 1);
        isImmediate = true;
    }

    int i = 0;
    TokenType r1;
    string r2;

    while (i < NUMBER_OF_PARSERS) {

        if (regex_match(data, parsers[i])) {
        
            switch (i) {

                // access modifier
                case 0: 
                    r1 = TokenType::ACCESS_MODIFIER;
                    r2 = data;
                break;

                // label; remove ":" at the end
                case 1:
                    r1 = TokenType::LABEL;
                    r2 = data.substr(0, data.length() - 1);
                break;

                // section
                case 2:
                    r1 = TokenType::SECTION;
                    r2 = data;
                break;

                // directive
                case 3:
                    r1 = TokenType::DIRECTIVE;
                    r2 = data;
                break;

                // instruction
                case 4:
                    r1 = TokenType::INSTRUCTION;
                    r2 = data;
                break;

                // end of file
                case 5:
                    r1 = TokenType::END_OF_SECTIONS;
                    r2 = data;
                break;

                // arithmetic operator
                case 6:
                    r1 = TokenType::ARITHMETIC_OPERATOR;
                    r2 = data;
                break;

                // symbol
                case 7:
                    if (isAsterisk)
                        r1 = TokenType::ASTERISK_SYMBOL;
                    else if (isImmediate)
                        r1 = TokenType::IMMEDIATE_SYMBOL;
                    else
                        r1 = TokenType::SYMBOL;
                    r2 = data;
                break;

                // literal decimal
                case 8:
                    if (isAsterisk)
                        r1 = TokenType::ASTERISK_DECIMAL;
                    else if (isImmediate)
                        r1 = TokenType::IMMEDIATE_DECIMAL;
                    else
                        r1 = TokenType::DECIMAL;
                    r2 = data;
                break;

                // literal hexadecimal
                case 9:
                    if (isAsterisk)
                        r1 = TokenType::ASTERISK_HEXADECIMAL;
                    else if (isImmediate)
                        r1 = TokenType::IMMEDIATE_HEXADECIMAL;
                    else
                        r1 = TokenType::HEXADECIMAL;
                    r2 = data;
                break;

                // register direct
                case 10:
                    r1 = TokenType::REGISTER_DIRECT;
                    r2 = data;
                break;

                // pc relative
                case 11:
                    r1 = TokenType::PC_RELATIVE;
                    r2 = data;
                break;	

                // register indirect
                case 12:
                    r1 = TokenType::REGISTER_INDIRECT;
                    r2 = data;
                break;

            }

            return Token(r1, r2);

        }

        i++;

    }

    if (recursive)
    {
        char* duplicate = strdup(str.c_str());
        char* token = strtok(duplicate, ARITHMETIC_DELIMITER);

        while (token != NULL)
        {
            Token t = parse(string(token), line, false);

            if (
                t.getType() != TokenType::IMMEDIATE_DECIMAL &&
                t.getType() != TokenType::DECIMAL &&
                t.getType() != TokenType::IMMEDIATE_HEXADECIMAL && 
                t.getType() != TokenType::HEXADECIMAL && 
                t.getType() != TokenType::IMMEDIATE_SYMBOL &&
                t.getType() != TokenType::SYMBOL
            )
                throw AssemblyException("Unable to parse '" + str + "'.", line);

            token = strtok(NULL, ARITHMETIC_DELIMITER);

        }

        delete duplicate;

        return Token(TokenType::ARITHMETIC_EXPRESSION, str);
        
    }

    throw AssemblyException("Unable to parse '" + str + "'.", line);
    
}