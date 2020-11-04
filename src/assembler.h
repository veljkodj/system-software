#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#define BYTES_INLINE 8

#define START_SECTION -1

#define COMMENT_SYMBOL '#'
#define DELIMITER "\t\n, "

#define DIRECTIVE_END ".end"
#define DIRECTIVE_BYTE ".byte"
#define DIRECTIVE_WORD ".word"
#define DIRECTIVE_SKIP ".skip"
#define DIRECTIVE_EQU ".equ"
#define MODIFIER_EXTERN ".extern"
#define MODIFIER_GLOBAL ".global"
#define INSTRUCTION_SUB "sub"
#define INSTRUCTION_SHR "shr"
#define INSTRUCTION_POP "pop"

#include <iostream>
#include <fstream>
#include <queue>
#include <string>
#include <vector>
#include <algorithm>

#include "structures.h"
#include "enums.h"
#include "arithmetic.h"

using namespace std;

class Instruction;

class InstructionDetails
{
private:
    string mnemonic;
    uint8_t operationCode;
    uint8_t numberOfOperands;
public:
    InstructionDetails(string mnemonic, uint8_t operationCode, uint8_t numberOfOperands) :
        mnemonic(mnemonic), operationCode(operationCode), numberOfOperands(numberOfOperands) {}
    
    string getMnemonic() const { return mnemonic; };
    uint8_t getOperationCode() const { return operationCode; };
    uint8_t getNumberOfOperands() const { return numberOfOperands; };

};

static map<string, InstructionDetails> instructions = {
    {"halt", InstructionDetails("halt", 0, 0)},
    {"iret", InstructionDetails("iret", 1, 0)},
    {"ret", InstructionDetails("halt", 2, 0)},
    {"int", InstructionDetails("int", 3, 1)},
    {"call", InstructionDetails("call", 4, 1)},
    {"jmp", InstructionDetails("jmp", 5, 1)},
    {"jeq", InstructionDetails("jeq", 6, 1)},
    {"jne", InstructionDetails("jne", 7, 1)},
    {"jgt", InstructionDetails("jgt", 8, 1)},
    {"push", InstructionDetails("push", 9, 1)},
    {"pop", InstructionDetails("pop", 10, 1)},
    {"xchg", InstructionDetails("xchg", 11, 2)},
    {"mov", InstructionDetails("mov", 12, 2)},
    {"add", InstructionDetails("add", 13, 2)},
    {"sub", InstructionDetails("sub", 14, 2)},
    {"mul", InstructionDetails("mul", 15, 2)},
    {"div", InstructionDetails("div", 16, 2)},
    {"cmp", InstructionDetails("cmp", 17, 2)},
    {"not", InstructionDetails("not", 18, 2)},
    {"and", InstructionDetails("and", 19, 2)},
    {"or", InstructionDetails("or", 20, 2)},
    {"xor", InstructionDetails("xor", 21, 2)},
    {"test", InstructionDetails("test", 22, 2)},
    {"shl", InstructionDetails("shl", 23, 2)},
    {"shr", InstructionDetails("shr", 24, 2)}
};

class Assembler {
public:

    Assembler(string inputFile, string outputFile);
    void generate();
    ~Assembler();

private:

    void oneAndOnlyPass();
    void loadLocally();
    void backpatching();

    void writeToMachineCode(IdSection idSection, uint8_t byte);
    void writeToMachineCode(IdSection idSection, Instruction instruction);
    void writeToOutputFile();

    void referencingSymbol(
        string symbolString, 
        IdSection inSection, 
        unsigned long patch,
        RelocationType relocationType,
        unsigned long nextInstrToExecuteLC,
        bool modifyOneByte
    );

    void appendGlobalSymbolElem(string symbol);
    void appendExternSymbolElem(string symbol);
    void resolveSymbols();

    bool isClassificationIndexOk(string symbol, string expression);
    void resolveTNSSymbols();

    ifstream inputFile;
    vector<vector<string>> assembly;

    SymbolTable* symbolTable;
    SectionTable* sectionTable;
    RelocationTable* relocationTable;
    TNSTable* tns;
    
    map<IdSection, vector<uint8_t>> machineCode;
    ofstream outputFile;

    struct SymbolReference *symbolReferenceElemFirst = nullptr, *symbolReferenceElemLast = nullptr;
    struct SymbolElement *globalSymbolFirst = nullptr, *globalSymbolLast = nullptr;
    struct SymbolElement *externSymbolFirst = nullptr, *externSymbolLast = nullptr;

    friend class Instruction;

};

class Instruction {
public:

    Instruction(
        queue<Token> instruction, 
        unsigned long line, 
        unsigned long locationCounter, 
        IdSection currentSection,
        Assembler* assembler
    );
    
    static int getInstructionSize(unsigned long line, queue<Token> instruction);
    static bool isInstructionJump(string instruction);

    friend class Assembler;

private:

    uint8_t operationCode[7] = { 0,0,0,0,0,0,0 };
    uint8_t instructionSize = 0;

};

#endif