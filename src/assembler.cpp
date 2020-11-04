#include "assembler.h"

Assembler::Assembler(string inputFile, string outputFile)
{

    this->inputFile.open(inputFile, ios::in);
    this->outputFile.open(outputFile, ios::out | ios::trunc);

    symbolTable = new SymbolTable();
    sectionTable = new SectionTable();
    relocationTable = new RelocationTable();
    tns = new TNSTable();

    IdSection idSection = sectionTable->insertSection("UND", 0, 0);
    IdSymbol idSymbol = symbolTable->insertSymbol(
        "UND",
        idSection,
        0,
        Scope::EXTERN,
        false
    );
    sectionTable->getEntryByID(idSection)->SymbolEntryNo = idSymbol;

}

Assembler::~Assembler() {

    delete tns;
    delete relocationTable;
    delete sectionTable;
    delete symbolTable;

    outputFile.close();
    inputFile.close();

    struct SymbolElement *prev = nullptr, *curr = nullptr;
    
    curr = externSymbolFirst;
    while (curr) {
        prev = curr;
        curr = curr->next;
        delete prev;
    }

    curr = globalSymbolFirst;
    while (curr) {
        prev = curr;
        curr = curr->next;
        delete prev;
    }

}

void Assembler::loadLocally() {

    string line;
    unsigned long lineCntr = 0;
    char* duplicate = nullptr;
    char* token = nullptr;

    while (std::getline(inputFile, line))
    {
        lineCntr++;

        if (line.find(COMMENT_SYMBOL) != string::npos)
            line = line.substr(0, line.find(COMMENT_SYMBOL));

        transform(line.begin(), line.end(), line.begin(), ::tolower);

        vector<string>* collector = new vector<string>();

        duplicate = strdup(line.c_str());
        token = strtok(duplicate, DELIMITER);

        while (token != NULL)
        {
            collector->push_back(string(token));
            token = strtok(NULL, DELIMITER);
        }

        delete duplicate;

        assembly.push_back(*collector);
        collector->clear();
        delete collector;

        if (collector->size() > 0 && (*collector)[0] == DIRECTIVE_END)
            break;

    }

    if (
        assembly.size() == 0 || 
        (assembly.at(assembly.size()-1).size() > 0 && Token::parse(assembly.at(assembly.size()-1).at(0), lineCntr, true).getType() != TokenType::END_OF_SECTIONS)
    )
        assembly.push_back({ DIRECTIVE_END });

}

void Assembler::writeToMachineCode(IdSection idSection, uint8_t byte) {

    if ((machineCode.find(idSection) == machineCode.end()))
        machineCode.insert({idSection, vector<uint8_t>()});

    machineCode[idSection].push_back(byte);

}

void Assembler::writeToMachineCode(IdSection idSection, Instruction instruction) {

    if ((machineCode.find(idSection) == machineCode.end()))
        machineCode.insert({idSection, vector<uint8_t>()});

    for (int i = 0; i < instruction.instructionSize; i++)
        machineCode[idSection].push_back(instruction.operationCode[i]);

}

void Assembler::generate() {

    loadLocally();

    oneAndOnlyPass();

    resolveSymbols();

    resolveTNSSymbols();

    backpatching();

    writeToOutputFile();

}

void Assembler::oneAndOnlyPass() {

    IdSection currentSection = START_SECTION;
    SymbolEntry* entry = nullptr;
    IdSymbol idSymbol = 0;
    unsigned long toWrite = 0;
    unsigned long padding = 0;
    unsigned long cntrLine = 0;
    unsigned long LC = 0;
    bool allLiterals = false;
    Token userDefinedSection;
    Token operand;

    for (vector<string> line : assembly) {

        cntrLine++;
        if (line.size() == 0) continue;

        queue<string> currentLineTokens;
        for (string s : line) currentLineTokens.push(s);

        Token currentToken = Token::parse(currentLineTokens.front(), cntrLine, true);
        currentLineTokens.pop();
        
        string labelName;
        if (currentToken.getType() == TokenType::LABEL)
        {
            labelName = currentToken.getValue();

            if (currentSection == START_SECTION)
                throw AssemblyException("Label '" + labelName + "' is defined outside of any section", cntrLine);

            if (symbolTable->getEntryByName(labelName) != nullptr && symbolTable->getEntryByName(labelName)->defined)
                throw AssemblyException("Label '" + labelName + "' is already defined", cntrLine);
            
            if (symbolTable->getEntryByName(labelName) != nullptr) {
                symbolTable->getEntryByName(labelName)->defined = true;
                symbolTable->getEntryByName(labelName)->value = LC;
            } else 
                idSymbol = symbolTable->insertSymbol(
                    labelName,
                    currentSection,
                    LC,
                    Scope::LOCAL,
                    true // defined = true;
                );

            if (currentLineTokens.empty())
                continue;
            
            currentToken = Token::parse(currentLineTokens.front(), cntrLine, true);
            currentLineTokens.pop();

        }

        switch (currentToken.getType()) {

            case TokenType::LABEL:
            {
            
            }	throw AssemblyException("Incorrect syntax after label '" + labelName + "'.", cntrLine);
            break;

            case TokenType::ACCESS_MODIFIER:
            {	
                if (currentToken.getValue() == MODIFIER_EXTERN)
                {
                    do
                    {
                        operand = Token::parse(currentLineTokens.front(), cntrLine, true);
                        currentLineTokens.pop();

                        if (operand.getType() != TokenType::SYMBOL)
                            throw AssemblyException("Directive '.extern' should be followed by symbol or list of symbols", cntrLine);

                        appendExternSymbolElem(operand.getValue());

                    } while (!currentLineTokens.empty());
                } 

                else if (currentToken.getValue() == MODIFIER_GLOBAL)
                {
                    do
                    {
                        operand = Token::parse(currentLineTokens.front(), cntrLine, true);
                        currentLineTokens.pop();

                        if (operand.getType() != TokenType::SYMBOL)
                            throw AssemblyException("Directive '.global' should be followed by symbol or list of symbols", cntrLine);

                        appendGlobalSymbolElem(operand.getValue());

                    } while (!currentLineTokens.empty());
                }
            }
            break;
            
            case TokenType::DIRECTIVE:

                if (currentSection == START_SECTION)
                    throw AssemblyException("Directive '" + currentToken.getValue() + "' is defined outside of any section", cntrLine);

                if (currentToken.getValue() == DIRECTIVE_BYTE)
                {

                    do {

                        operand = Token::parse(currentLineTokens.front(), cntrLine, true);
                        currentLineTokens.pop();

                        if ((operand.getType() != TokenType::DECIMAL) &&
                            (operand.getType() != TokenType::HEXADECIMAL) && 
                            (operand.getType() != TokenType::SYMBOL))
                            throw AssemblyException("Directive .byte should be followed by literal or symbol, or list of literals and symbols", cntrLine);

                        if (operand.getType() == TokenType::DECIMAL)
                            toWrite = stoi(operand.getValue());
                        else if (operand.getType() == TokenType::HEXADECIMAL)	
                            toWrite = stoul(operand.getValue(), nullptr, 16);
                        else // operand.getType() == TokenType::SYMBOL
                        {
                            toWrite = 0;
                            referencingSymbol(operand.getValue(), currentSection, LC, RelocationType::R_386_16, 0, true);
                        }

                        writeToMachineCode(currentSection, (uint8_t)toWrite);

                        LC++;

                    } while (!currentLineTokens.empty());

                }

                else if (currentToken.getValue() == DIRECTIVE_SKIP)
                {
                    operand = Token::parse(currentLineTokens.front(), cntrLine, true);
                    currentLineTokens.pop();

                    if (operand.getType() != TokenType::DECIMAL &&
                        operand.getType() != TokenType::HEXADECIMAL)
                        throw AssemblyException("Directive .skip should be followed by literal", cntrLine);

                    if (operand.getType() == TokenType::DECIMAL)
                        padding = stoi(operand.getValue());
                    else if (operand.getType() == TokenType::HEXADECIMAL)
                        padding = stoul(operand.getValue(), nullptr, 16);

                    for (int i = 0; i < padding; i++)
                        writeToMachineCode(currentSection, 0);
                    
                    LC += padding;
                }
                
                else if (currentToken.getValue() == DIRECTIVE_WORD)
                {

                    do
                    {
                        operand = Token::parse(currentLineTokens.front(), cntrLine, true);
                        currentLineTokens.pop();

                        if ((operand.getType() != TokenType::DECIMAL) &&
                            (operand.getType() != TokenType::HEXADECIMAL) && 
                            (operand.getType() != TokenType::SYMBOL))
                            throw AssemblyException("Directive .word should be followed by literal or symbol, or list of literals and symbols", cntrLine);

                        if (operand.getType() == TokenType::DECIMAL)
                            toWrite = stoi(operand.getValue());
                        else if (operand.getType() == TokenType::HEXADECIMAL)	
                            toWrite = stoul(operand.getValue(), nullptr, 16);
                        else // operand.getType() == SYMBOL
                        {
                            toWrite = 0;
                            referencingSymbol(operand.getValue(), currentSection, LC, RelocationType::R_386_16, 0, false);
                        }

                        writeToMachineCode(currentSection, (uint8_t)(toWrite & 0xFF));
                        writeToMachineCode(currentSection, (uint8_t)((toWrite >> 8) & 0xFF));

                        LC += 2;

                    } while (!currentLineTokens.empty());

                }
                
                else if (currentToken.getValue() == DIRECTIVE_EQU)
                {

                    operand = Token::parse(currentLineTokens.front(), cntrLine, false);
                    currentLineTokens.pop();

                    if (operand.getType() != TokenType::SYMBOL)
                        throw AssemblyException("Directive '.equ' requires label as first operand.", cntrLine);

                    string expression;
                    while (currentLineTokens.size())
                    {
                        expression += currentLineTokens.front();
                        currentLineTokens.pop();
                    }

                    allLiterals = true;
                    
                    vector<Token> arithmeticTokens = Arithmetic::tokenize(expression);
                    
                    for (const Token& t : arithmeticTokens)
                        if (
                            t.getType() != TokenType::ARITHMETIC_OPERATOR &&
                            t.getType() != TokenType::DECIMAL && 
                            t.getType() != TokenType::IMMEDIATE_DECIMAL &&
                            t.getType() != TokenType::HEXADECIMAL && 
                            t.getType() != TokenType::IMMEDIATE_HEXADECIMAL 
                        )
                        {
                            allLiterals = false;
                            break;
                        }

                    if (allLiterals) { // can be calculated right now

                        arithmeticTokens = Arithmetic::convertToPostfix(arithmeticTokens);

                        symbolTable->insertSymbol(
                            operand.getValue(),
                            currentSection,
                            Arithmetic::calculateSymbolValue(arithmeticTokens, symbolTable, currentSection),
                            Scope::LOCAL,
                            true
                        );

                    } else { // add to tns

                        symbolTable->insertSymbol(
                            operand.getValue(),
                            currentSection,
                            ASM_UNDEFINED,
                            Scope::LOCAL,
                            false
                        );

                        tns->insertSymbol(currentSection, operand.getValue(), expression, Scope::LOCAL);

                    }

                }
                
            break;

            case TokenType::SECTION:
            {
                if (currentSection != START_SECTION) 
                    sectionTable->getEntryByID(currentSection)->length = LC;
            
                if (currentLineTokens.size() == 0)
                    throw AssemblyException("Directive '.section' should be followed by the name of new section", cntrLine);

                userDefinedSection = Token::parse(currentLineTokens.front(), cntrLine, true);
                currentLineTokens.pop();

                if (userDefinedSection.getType() != TokenType::LABEL)
                    throw new AssemblyException("Directive '.section' should be followed by the name of new section", cntrLine);

                if (!currentLineTokens.empty())
                    throw AssemblyException("Incorrect syntax", cntrLine);

                currentSection = sectionTable->insertSection(userDefinedSection.getValue(), 0, cntrLine);
                idSymbol = symbolTable->insertSymbol(
                    userDefinedSection.getValue(),
                    currentSection,
                    0,
                    Scope::LOCAL,
                    true
                );
                sectionTable->getEntryByID(currentSection)->SymbolEntryNo = idSymbol;
                LC = 0;

            }
            break;

            case TokenType::END_OF_SECTIONS:
            {
                if (currentSection != START_SECTION)
                    sectionTable->getEntryByID(currentSection)->length = LC;
            }
            break;
            
            case TokenType::INSTRUCTION:
            {

                queue<Token> _instruction;
                _instruction.push(currentToken);

                while (!currentLineTokens.empty())
                {
                    operand = Token::parse(currentLineTokens.front(), cntrLine, true);
                    _instruction.push(operand);
                    currentLineTokens.pop();
                }

                Instruction instruction(
                    _instruction, 
                    cntrLine,
                    LC, // before instruction
                    currentSection,
                    this
                );

                LC += instruction.instructionSize;

                writeToMachineCode(currentSection, instruction);

            break;
            }
        
        }

    }

}

void Assembler::writeToOutputFile() {

    /* write relevant tables */

    outputFile << "<--Symbol table-->" << endl;
    outputFile << symbolTable->generateTextualSymbolTable().str() << endl << endl;

    outputFile << "<--Section table-->" << endl;
    outputFile << sectionTable->generateTextualSectionTable().str() << endl << endl;

    /* write machine code */

    int currentBytesInline;
    map<IdSection, vector<uint8_t>>::iterator it;

    for (it = machineCode.begin(); it != machineCode.end(); it++) {

        outputFile << "<--Section '" <<  sectionTable->getEntryByID(it->first)->name << "'-->" << endl << endl;
        outputFile << relocationTable->generateTextualRelocationTable(it->first).str() << endl;

        currentBytesInline = 0;

        for (int i = 0; i < it->second.size(); i++) {

            outputFile << hex << ((it->second[i]  >> 4) & 0xF);
            outputFile << hex << (it->second[i]  & 0xF);
        
            if (++currentBytesInline == BYTES_INLINE)
            {
                currentBytesInline = 0;
                outputFile << endl;
            }
            else
                outputFile << " ";
        }

        outputFile << endl << endl << endl;

    }


}

void Assembler::appendGlobalSymbolElem(string symbol) {

    struct SymbolElement* temp = new SymbolElement(symbol, nullptr);

    if (globalSymbolFirst == nullptr)
        globalSymbolLast = globalSymbolFirst = temp;
    else
        globalSymbolLast = globalSymbolLast->next = temp;

}

void Assembler::appendExternSymbolElem(string symbol) {

    struct SymbolElement* temp = new SymbolElement(symbol, nullptr);

    if (externSymbolFirst == nullptr)
        externSymbolLast = externSymbolFirst = temp;
    else
        externSymbolLast = externSymbolLast->next = temp;

}

void Assembler::resolveSymbols() {

    struct SymbolElement *curr = nullptr;
    SymbolEntry *symbol = nullptr;

    // resolve global symbols
    curr = globalSymbolFirst;
    while (curr) {

        symbol = symbolTable->getEntryByName(curr->symbol);

        if (symbol == nullptr || !symbol->defined)
            throw AssemblyException("Symbol '" + curr->symbol + "' is declared as global, but isn't defined");
        
        symbol->scope = Scope::GLOBAL;
        
        curr = curr->next;
    
    }

    // resolve extern symbols
    curr = externSymbolFirst;
    while (curr) {

        if (symbolTable->getEntryByName(curr->symbol) != nullptr)
            throw AssemblyException("Symbol '" + curr->symbol + "' is declared as extern, but is defined");
        
        symbol = symbolTable->getEntryByID(symbolTable->insertSymbol(curr->symbol, 0, 0, Scope::EXTERN, false));

        curr = curr->next;

    }

}

void Assembler::backpatching() {

    struct SymbolReference *prev = nullptr, *curr = symbolReferenceElemFirst;
    SymbolEntry *symbol = nullptr;
    uint16_t t = 0;

    while (curr) {

        t = 0;

        symbol = symbolTable->getEntryByName(curr->symbol);

        if (symbol == nullptr)
            throw AssemblyException("Unsuccessful backpatching - symbol '" + curr->symbol + "' is not defined.");

        if ((symbol->scope == Scope::LOCAL || symbol->scope == Scope::GLOBAL) && !symbol->defined)
            throw AssemblyException("Unsuccessful backpatching - symbol '" + symbol->name + "' is not defined.");

        if (curr->relocationType == RelocationType::R_386_PC16) {

            if (symbol->section == curr->inSection)
                t = symbol->value - curr->nextInstructionLC;
            else if (symbol->scope == Scope::LOCAL) {
                t = symbol->value - 2;
                relocationTable->insertRelocation(
                    curr->inSection,
                    curr->patch, 
                    RelocationType::R_386_PC16, 
                    sectionTable->getEntryByID(symbol->section)->SymbolEntryNo
                );
            } else { // Scope::GLOBAL || Scope::EXTERN
                t = -2;
                relocationTable->insertRelocation(
                    curr->inSection,
                    curr->patch,
                    RelocationType::R_386_PC16,
                    symbol->entryNo
                );
            }

        } else { // RelocationType::R_386_16
        
            if (symbol->scope == Scope::LOCAL) {
                t = symbol->value;
                relocationTable->insertRelocation(
                    curr->inSection,
                    curr->patch,
                    RelocationType::R_386_16,
                    sectionTable->getEntryByID(symbol->section)->SymbolEntryNo
                );
            } else { // Scope::GLOBAL || Scope::EXTERN
                t = 0;
                relocationTable->insertRelocation(
                    curr->inSection,
                    curr->patch,
                    RelocationType::R_386_16,
                    symbol->entryNo
                );
            }

        }

        if (curr->modifyOneByte) {
            machineCode[curr->inSection][curr->patch] = ((uint8_t)t & 0xFF);
        } else {
            machineCode[curr->inSection][curr->patch] = t & 0xFF;
            machineCode[curr->inSection][curr->patch + 1] = (t >> 8) & 0xFF;
        }

        prev = curr;
        curr = curr->next;
        delete prev;

    }

}

void Assembler::referencingSymbol(
    string symbolString, 
    IdSection inSection, 
    unsigned long patch,
    RelocationType relocationType,
    unsigned long nextInstrToExecuteLC,
    bool modifyOneByte
) {
    SymbolReference* temp = new SymbolReference(symbolString, inSection, patch, relocationType, nextInstrToExecuteLC, modifyOneByte);
    if (symbolReferenceElemFirst == nullptr)
        symbolReferenceElemLast = symbolReferenceElemFirst = temp;
    else
        symbolReferenceElemLast = symbolReferenceElemLast->next = temp;
}

bool Assembler::isClassificationIndexOk(string symbol, string expression) {
    
    map<IdSection, unsigned long> hashMap;

    for (IdSection idSection = 0; idSection < sectionTable->cntr; idSection++)
        hashMap.insert({idSection, 0});

    bool plus = false, minus = false;

    vector<Token> arithmeticTokens = Arithmetic::tokenize(expression);

    for (Token& t: arithmeticTokens) {

        if (
            t.getType() == TokenType::DECIMAL ||
            t.getType() == TokenType::HEXADECIMAL ||
            t.getType() == TokenType::IMMEDIATE_DECIMAL ||
            t.getType() == TokenType::IMMEDIATE_HEXADECIMAL
        )  {
            plus = false, minus = false;
            continue;
        }	
        else if (t.getType() == TokenType::ARITHMETIC_OPERATOR) {

            if (t.getValue() == "+") 
                plus = true;
            else 
                minus = true;
        
        } else { // TokenType::IMMEDIATE_SYMBOL || TokenType::SYMBOL

            if (symbolTable->getEntryByName(t.getValue()) == nullptr)
                throw AssemblyException("Symbol '" + t.getValue() + "' is used in .equ directive, but is not defined");

            if (symbolTable->getEntryByName(t.getValue())->section == 0) { // extern symbol
                hashMap[0]++;
                plus = false, minus = false;
            } else if (plus) {
                hashMap[symbolTable->getEntryByName(t.getValue())->section]++;
                plus = false;
            } else if (minus) {
                hashMap[symbolTable->getEntryByName(t.getValue())->section]--;
                minus = false;
            } else // first token in this expression is symbol without sign before it
                hashMap[symbolTable->getEntryByName(t.getValue())->section]++;

        }

    }

    bool flagIsOk = true;

    for (IdSection idSection = 0; idSection < sectionTable->cntr; idSection++) {

        if (hashMap[idSection] == 0)
            continue;
        else if (hashMap[idSection] == 1) {

            if (!flagIsOk)
                throw AssemblyException("Incorrect classification index for symbol '" + symbol + "'");

            flagIsOk = false;

        } else
            throw AssemblyException("Incorrect classification index for symbol '" + symbol + "'");

    }
    
    return true;
}

void Assembler::resolveTNSSymbols() {

    for (TNSEntry& entry: tns->table)
        isClassificationIndexOk(entry.name, entry.expression);

    bool end;

    do {

        end = true;

        for (size_t i = 0; i < tns->getSize(); i++)
        {
            try {
                
                TNSEntry* entry = tns->getEntryByID(unsigned(i));
                vector<Token> arithmeticTokens = Arithmetic::convertToPostfix(Arithmetic::tokenize(entry->expression));

                unsigned long v = Arithmetic::calculateSymbolValue(arithmeticTokens, symbolTable, entry->section);

                symbolTable->getEntryByName(entry->name)->value = v;
                symbolTable->getEntryByName(entry->name)->defined = true;

                for (Token& t: arithmeticTokens)
                    if ((t.getType() == TokenType::SYMBOL || t.getType() == TokenType::IMMEDIATE_SYMBOL) &&
                        symbolTable->getEntryByName(t.getValue())->scope == Scope::EXTERN
                    ) {
                        symbolTable->getEntryByName(entry->name)->scope = Scope::EXTERN;
                        break;
                    }

                tns->deleteEntryByName(entry->name);
                end = false;
                break;

            } 
            catch (const exception& ex)
            {
                ex.what();
            }
        
        }

        if (end && tns->getSize() > 0)
            throw AssemblyException("Possible circular dependency between TNS symbols");

    } while (!end);

}

bool Instruction::isInstructionJump(string instr) {
    return instr == "jmp" || instr == "jeq" || instr == "jne" || instr == "jgt";
};

Instruction::Instruction (
    queue<Token> instruction, 
        unsigned long line, 
        unsigned long locationCounter, 
        IdSection currentSection,
        Assembler* assembler
) {

    unsigned long sizeInBytes = Instruction::getInstructionSize(line, instruction);

    Token mnemonic = instruction.front(); 
    string mnemomicString = mnemonic.getValue();

    queue<Token> params = instruction; params.pop();
    int iterations = params.size();

    OperandSize size = OperandSize::WORD;
    if (mnemomicString[mnemomicString.size() - 1] == 'w') {
        mnemomicString = mnemomicString.substr(0, mnemomicString.size() - 1);
    } else if ((mnemomicString[mnemomicString.size() - 1] == 'b') && (mnemomicString != INSTRUCTION_SUB)) {
        size = OperandSize::BYTE;
        mnemomicString = mnemomicString.substr(0, mnemomicString.size() - 1);
    }

    map<string, InstructionDetails>::iterator it = instructions.find(mnemomicString);

    if (it == instructions.end())
        throw AssemblyException("Inctruction '" + mnemomicString + "' does not exist", line);
    
    if (instructions.at(mnemomicString).getNumberOfOperands() != params.size())
        throw AssemblyException("Wrong number of operands in instruction '" + mnemomicString + "'", line);
    
    operationCode[0] = instructions.at(mnemomicString).getOperationCode() << 3;
    operationCode[0] |= (size << 2);
    instructionSize++;

    int toWrite = 1;

    bool destination = false;

    if (params.size() == 2 && mnemomicString == INSTRUCTION_SHR)
        destination = true;
    else if (params.size() == 2)
        destination = false;
    else if (params.size() == 1 && mnemomicString == INSTRUCTION_POP)
        destination = true;
    else if (params.size() == 1)
        destination = false;
    
    string offsetString;
    string registerNumber;
    Token offset;
    long valueToWrite = 0;
    int c = 0;
    int mode = 0;
    bool noOffset = false;
    SymbolEntry *entry = nullptr;

    int i = 0;

    while (i++ < iterations) {

        Token operand = params.front();
        params.pop();

        switch (operand.getType()) {

            case TokenType::SYMBOL:
            case TokenType::DECIMAL:
            case TokenType::HEXADECIMAL:
            {

                if (isInstructionJump(mnemomicString)) // immediate
                    operationCode[toWrite++] = 0;
                else // memory direct
                    operationCode[toWrite++] = 4 << 5;
                instructionSize++;

                valueToWrite = 0;

                if (operand.getType() == TokenType::HEXADECIMAL)
                    valueToWrite = (unsigned long)strtol(operand.getValue().c_str(), 0, 16);
                else if (operand.getType() == TokenType::DECIMAL)
                    valueToWrite = stoul(operand.getValue());
                else // TokenType::SYMBOL
                {
                    valueToWrite = 0;
                    assembler->referencingSymbol(
                        operand.getValue(),
                        currentSection,
                        locationCounter + toWrite,
                        RelocationType::R_386_16,
                        locationCounter + sizeInBytes,
                        false
                    );
                }

                operationCode[toWrite++] = (uint8_t)(valueToWrite & 0xFF);
                operationCode[toWrite++] = (uint8_t)((valueToWrite >> 8) & 0xFF);
                instructionSize += 2;
            }
            break;

            case TokenType::REGISTER_INDIRECT:
            {	
                noOffset = false;

                if (operand.getValue().find('(') == 0) 
                    noOffset = true;
                else {
                    offsetString = string(operand.getValue().substr(0, operand.getValue().find('(')));
                    offset = Token::parse(offsetString, line, false);
                }

                registerNumber = operand.getValue().substr(operand.getValue().find("(") + 3, string::npos);
                registerNumber = string(registerNumber.substr(0, registerNumber.size()-1));
                if (registerNumber.at(registerNumber.size()-1) == 'h' || registerNumber.at(registerNumber.size()-1) == 'l')
                    registerNumber = string(registerNumber.substr(0, registerNumber.size()-1));
                
                c = stoi(registerNumber);

                if (c == 15)
                    throw AssemblyException("Using PSW register in indirect addressing mode is not allowed", line);
                
                if (c >= 8 || c < 0)
                    throw AssemblyException("Specified register is not supported in this architecture", line);
                
                valueToWrite = 0;

                if (offset.getType() == TokenType::HEXADECIMAL)
                    valueToWrite = (unsigned long)strtol(offset.getValue().c_str(), 0, 16);
                else if (offset.getType() == TokenType::DECIMAL)
                    valueToWrite = stoul(offset.getValue());
                else if (offset.getType() == TokenType::SYMBOL)
                {
                    valueToWrite = 0;
                    assembler->referencingSymbol(
                        offset.getValue(),
                        currentSection,
                        locationCounter + toWrite + 1,
                        RelocationType::R_386_16,
                        locationCounter + sizeInBytes,
                        false
                    );
                }

                if ((valueToWrite == 0 && offset.getType() != TokenType::SYMBOL) || noOffset)
                {
                    operationCode[toWrite++] = (2 << 5) | (c << 1);
                    instructionSize++;
                }
                else
                {
                    operationCode[toWrite++] = (3 << 5) | (c << 1);
                    instructionSize++;

                    operationCode[toWrite++] = (uint8_t)(valueToWrite & 0xFF);
                    operationCode[toWrite++] = (uint8_t)((valueToWrite >> 8) & 0xFF);
                    
                    instructionSize += 2;
                }
            }
            break;

            case TokenType::ASTERISK_DECIMAL:
            case TokenType::ASTERISK_HEXADECIMAL:
            case TokenType::ASTERISK_SYMBOL:
            {
                operationCode[toWrite++] = 4 << 5;
                instructionSize++;

                offsetString = operand.getValue();
                offset = Token::parse(offsetString, line, false);

                if (offset.getType() == TokenType::HEXADECIMAL)
                    valueToWrite = (unsigned long)strtol(offsetString.c_str(), 0, 16);
                else if (offset.getType() == TokenType::DECIMAL)
                    valueToWrite = stoul(offsetString);
                else if (operand.getType() == TokenType::SYMBOL)
                {
                    valueToWrite = 0;
                    assembler->referencingSymbol(
                        operand.getValue(),
                        currentSection,
                        locationCounter + toWrite,
                        RelocationType::R_386_16,
                        locationCounter + sizeInBytes,
                        false
                    );
                }

                operationCode[toWrite++] = (uint8_t)(valueToWrite & 0xFF);
                operationCode[toWrite++] = (uint8_t)((valueToWrite >> 8) & 0xFF);

                instructionSize += 2;
            }
            break;

            case TokenType::REGISTER_DIRECT:
            {
                registerNumber = string(operand.getValue()).substr(2, string::npos);
                if (registerNumber.at(registerNumber.size()-1) == 'h' || registerNumber.at(registerNumber.size()-1) == 'l')
                    registerNumber = string(registerNumber.substr(0, registerNumber.size()-1));

                c = stoi(registerNumber);
                mode = 0;

                if (destination && c == 15)
                    throw AssemblyException("Writing to PSW register is not allowed", line);

                if (size == OperandSize::BYTE)
                {
                    if (operand.getValue().at(operand.getValue().size()-1) != 'h' &&
                        operand.getValue().at(operand.getValue().size()-1) != 'l'
                    )
                        throw AssemblyException("Specify which byte you want to use (higer or lower byte)", line);
                    
                    if (operand.getValue().at(operand.getValue().size()-1) == 'h')
                        mode = 1;
                    else if (operand.getValue().at(operand.getValue().size()-1) == 'l')
                        mode = 0;
                }
                else
                {
                
                    if (operand.getValue().at(operand.getValue().size()-1) == 'h' &&
                        operand.getValue().at(operand.getValue().size()-1) == 'l'
                    )
                        throw AssemblyException("If you use word as an operand, you should not specify which byte in that word you want to use", line);

                }

                if ((c >= 8 || c < 0) && c != 15)
                    throw AssemblyException("Specified register is not supported in this architecture", line);

                operationCode[toWrite++] = (1 << 5) | (c << 1) | mode;
                instructionSize++;

            }
            break;

            case TokenType::IMMEDIATE_DECIMAL:
            case TokenType::IMMEDIATE_HEXADECIMAL:
            case TokenType::IMMEDIATE_SYMBOL:
            {
                if (destination)
                    throw AssemblyException("Immediate value is specified as destination operand", line);

                operationCode[toWrite++] = 0;
                instructionSize++;

                valueToWrite = 0;

                if (operand.getType() == TokenType::IMMEDIATE_HEXADECIMAL)
                    valueToWrite = (unsigned long)strtol(operand.getValue().c_str(), 0, 16);
                else if (operand.getType() == TokenType::IMMEDIATE_DECIMAL)
                    valueToWrite = stoul(operand.getValue());
                else if (operand.getType() == IMMEDIATE_SYMBOL)
                {
                    valueToWrite = 0;
                    assembler->referencingSymbol(
                        operand.getValue(),
                        currentSection,
                        locationCounter + toWrite,
                        RelocationType::R_386_16,
                        locationCounter + sizeInBytes,
                        size == OperandSize::BYTE ? true : false
                    );
                }

                if (size == OperandSize::BYTE)
                {
                    operationCode[toWrite++] = (uint8_t)valueToWrite;
                    instructionSize++;
                }
                else
                {
                    operationCode[toWrite++] = (uint8_t)(valueToWrite & 0xFF);
                    operationCode[toWrite++] = (uint8_t)((valueToWrite >> 8) & 0xFF);
                    instructionSize += 2;
                }
            }	
            break;

            case TokenType::PC_RELATIVE:
            {

                offsetString = operand.getValue().substr(0, operand.getValue().find('('));

                operationCode[toWrite++] = ((3 << 5) | (7 << 1)); 
                instructionSize++;

                valueToWrite = 0;

                assembler->referencingSymbol(
                    offsetString,
                    currentSection,
                    locationCounter + toWrite,
                    RelocationType::R_386_PC16,
                    locationCounter + sizeInBytes,
                    false
                );

                operationCode[toWrite++] = (uint8_t)(valueToWrite & 0xFF);
                operationCode[toWrite++] = (uint8_t)((valueToWrite >> 8) & 0xFF);

                instructionSize += 2;
            }
            break;

            default:
                throw AssemblyException("Cannot code given instruction");
            break;

        }

        destination = !destination;

    }

}

int Instruction::getInstructionSize(unsigned long line, queue<Token> _instruction)
{

    int result = 0;

    Token instrToken = _instruction.front();
    _instruction.pop();
    int iters = _instruction.size();

    string instruction = instrToken.getValue();

    OperandSize size = OperandSize::WORD;

    if (instruction[instruction.size() - 1] == 'w') {
        instruction = instruction.substr(0, instruction.size() - 1);
    } else if ((instruction[instruction.size() - 1] == 'b') && (instruction != "sub")) {
        size = OperandSize::BYTE;
        instruction = instruction.substr(0, instruction.size() - 1);
    }

    map<string, InstructionDetails>::iterator it = instructions.find(instruction);

    if (it == instructions.end())
        throw AssemblyException("Instruction '" + instruction + "' does not exist", line);
    
    if (instructions.at(instruction).getNumberOfOperands() != _instruction.size())
        throw AssemblyException("Wrong number of operands in instruction '" + instruction + "'", line);

    result++;
    
    string offset;
    Token tokenOffset;
    long toWrite;

    while (iters > 0)
    {
        Token operand = _instruction.front();
        _instruction.pop();

        switch (operand.getType())
        {
            case TokenType::REGISTER_DIRECT:
            {
                result++;
                break;
            }

            case TokenType::IMMEDIATE_SYMBOL:
            {
                result++;
                result += 2;
                break;
            }

            case TokenType::IMMEDIATE_DECIMAL:
            case TokenType::IMMEDIATE_HEXADECIMAL:
            {
                result++;

                if (size == OperandSize::BYTE)
                    result++;
                else
                    result += 2;
                break;
            }

            case TokenType::REGISTER_INDIRECT:
            {

                bool noOffset = false;

                if (operand.getValue().find('(') == 0) {
                    noOffset = true;
                } else {
                    offset = operand.getValue().substr(0, operand.getValue().find('('));
                    tokenOffset = Token::parse(offset, line, false);
                }

                toWrite = -1;

                if (tokenOffset.getType() == TokenType::HEXADECIMAL)
                    toWrite = (unsigned long)strtol(tokenOffset.getValue().c_str(), 0, 16);
                else if (tokenOffset.getType() == TokenType::DECIMAL)
                    toWrite = stoul(tokenOffset.getValue());


                if (noOffset ||
                    ((tokenOffset.getType() == TokenType::DECIMAL || 
                    tokenOffset.getType() == TokenType::HEXADECIMAL) 
                    && toWrite == 0)
                )
                {
                    result++;
                }
                else
                {
                    result++;
                    result += 2;
                }

                break;

            }
            case TokenType::PC_RELATIVE:
            {
                result++;
                result += 2;
                break;
            }
            case TokenType::ASTERISK_DECIMAL:
            case TokenType::ASTERISK_HEXADECIMAL:
            case TokenType::ASTERISK_SYMBOL:
            {
                result++;
                result += 2;
                break;
            }

            case TokenType::DECIMAL:
            case TokenType::HEXADECIMAL:
            case TokenType::SYMBOL:
            {

                result++;
                result += 2;

                break;
            }		

            default:
            {
                throw AssemblyException("Non-existent addressing mode", line);
            }

        }

        iters--;

    }

    return result;
}
