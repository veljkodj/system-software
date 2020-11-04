#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <iomanip>

#include "enums.h"
#include "exceptions.h"
#include "token.h"

using namespace std;

struct SymbolElement {
    string symbol;
    struct SymbolElement* next;
    SymbolElement(string symbol, SymbolElement* next) : symbol(symbol), next(next) {}
};

struct SymbolReference {

    string symbol;
    IdSection inSection;
    unsigned long patch;
    RelocationType relocationType;
    unsigned long nextInstructionLC;
    bool modifyOneByte;
    SymbolReference *next = nullptr;

    SymbolReference(
        string symbol, 
        IdSection inSection, 
        unsigned long patch,
        RelocationType relocationType,
        unsigned long nextInstructionLC,
        bool modifyOneByte
    ) : symbol(symbol), 
        inSection(inSection), 
        patch(patch),
        relocationType(relocationType),
        nextInstructionLC(nextInstructionLC),
        modifyOneByte(modifyOneByte)
    {}

};

struct SymbolEntry
{
    IdSymbol entryNo;
    string name;
    IdSection section;		
    unsigned long value;
    Scope scope;
    bool defined;

    SymbolEntry(
        IdSymbol entryNo, 
        string name, 
        unsigned long section, 
        unsigned long value, 
        Scope scope, 
        bool defined
    ) : name(name), 
        section(section), 
        value(value), 
        scope(scope), 
        entryNo(entryNo), defined(defined) 
    {}
};

class SymbolTable
{
public:

    IdSymbol insertSymbol(string name, unsigned long section, unsigned long value, Scope Scope, bool defined);

    SymbolEntry* getEntryByID(IdSymbol id);
    SymbolEntry* getEntryByName(string name);

    stringstream generateTextualSymbolTable();
    size_t getSize() { return table.size(); }

    void deleteSymbol(const IdSymbol& id) { table.erase(id); }

private:

    map<IdSymbol, SymbolEntry> table;
    unsigned long cntr = 0;

};

struct SectionEntry
{
    IdSection entryNo;
    string name;
    unsigned long length;
    IdSymbol SymbolEntryNo = -1;

    SectionEntry() {}
    SectionEntry(IdSection entryNo, string name, unsigned long length) :
        name(name), length(length), entryNo(entryNo) {}
};

class SectionTable
{
public:

    IdSection insertSection(string name, unsigned long length, unsigned long lineNumber);

    SectionEntry* getEntryByID(IdSection id);
    SectionEntry* getEntryByName(string name);

    stringstream generateTextualSectionTable();

    size_t GetSize() { return table.size(); }

    friend class Assembler;

private:

    map<IdSection, SectionEntry> table;
    IdSection cntr = 0;

};

struct RelocationEntry
{
    unsigned long section;
    unsigned long offset;
    RelocationType relocationType;
    unsigned long value;

    RelocationEntry(
        unsigned long section, 
        unsigned long offset, 
        RelocationType relocationType, 
        unsigned long value
    ) : section(section), 
        value(value), 
        offset(offset), 
        relocationType(relocationType) 
    {}
};

class RelocationTable
{
public:

    void insertRelocation(unsigned long section, unsigned long offset, RelocationType relocationType, unsigned long value);

    RelocationEntry* getEntryByID(int id) { return &table.at(id); }

    stringstream generateTextualRelocationTable(IdSection idSection);
    
    size_t getSize() { return table.size(); }

    friend class Assembler;

private:

    vector<RelocationEntry> table;
    IdSection cntr = 0;

};

struct TNSEntry
{
    IdSection section;
    string name;
    string expression;
    Scope scope;

    TNSEntry() {}
    TNSEntry(IdSection section, string name, string expression, Scope Scope) :
        name(name), section(section), expression(expression), scope(Scope) {}
};

class TNSTable
{
public:

    void insertSymbol(IdSection section, string name, string expression, Scope scope);

    TNSEntry* getEntryByID(unsigned id);
    TNSEntry* getEntryByName(string name);

    void deleteEntryByName(string name);
    /*stringstream generateTextualTNSTable();*/

    size_t getSize() { return table.size(); }

    friend class Assembler;
private:

    vector<TNSEntry> table;

};

#endif