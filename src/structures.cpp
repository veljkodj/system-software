#include "structures.h"

IdSymbol SymbolTable::insertSymbol(string name, unsigned long sectionNumber, unsigned long value, Scope Scope, bool defined)
{

    map<IdSymbol, SymbolEntry>::iterator it;

    for (it = table.begin(); it != table.end(); it++)
        if (it->second.name == name)
            throw AssemblyException("Symbol '" + name + "' is already declared.");	


    SymbolEntry entry(cntr, name, sectionNumber, value, Scope, defined);

    table.insert({ cntr, entry });

    return cntr++;
}

SymbolEntry* SymbolTable::getEntryByID(IdSymbol id)
{
    if (table.find(id) != table.end())
        return &table.at(id);
    else
        return nullptr;
}

SymbolEntry* SymbolTable::getEntryByName(string name)
{
    map<IdSymbol, SymbolEntry>::iterator it;

    for (it = table.begin(); it != table.end(); it++)
        if (it->second.name == name)
            return &it->second;

    return nullptr;
}

stringstream SymbolTable::generateTextualSymbolTable()
{
    stringstream output;

    output << left;
    output << setw(15) << "EntryNumber";
    output << setw(15) << "Name";
    output << setw(15) << "SectionNumber";
    output << setw(15) << "Value";
    output << setw(15) << "Scope";
    output << endl;

    map<IdSymbol, SymbolEntry>::iterator it;

    for (it = table.begin(); it != table.end(); it++)
    {
        output << left;

        output << setw(15) << hex << it->second.entryNo;

        output << setw(15) << it->second.name;

        if (it->second.section != ASM_UNDEFINED)
            output << setw(15) << hex << it->second.section;
        else
            output << setw(15) << "N/A";

        output << setw(15) << hex << it->second.value;

        if (it->second.scope == Scope::GLOBAL)
            output << setw(15) << "GLOBAL";
        else if (it->second.scope == Scope::EXTERN)
            output << setw(15) << "EXTERN";
        else
            output << setw(15) << "LOCAL";

        output << endl;
    }

    return output;
}

IdSection SectionTable::insertSection(string name, unsigned long length, unsigned long lineNumber)
{
    map<IdSection, SectionEntry>::iterator it;

    for (it = table.begin(); it != table.end(); it++)
        if (it->second.name == name)
            throw AssemblyException("Section '" + name + "' is already declared.");	

    
    SectionEntry entry(cntr, name, length);
    table.insert({ cntr, entry });

    return cntr++;;
}

SectionEntry* SectionTable::getEntryByID(IdSection id)
{
    if (table.find(id) != table.end())
        return &table.at(id);
    else
        return nullptr;
}

SectionEntry* SectionTable::getEntryByName(string name)
{
    map<IdSection, SectionEntry>::iterator it;

    for (it = table.begin(); it != table.end(); it++)
        if (it->second.name == name)
            return &it->second;

    return nullptr;
}

stringstream SectionTable::generateTextualSectionTable()
{
    stringstream output;

    output << left;
    output << setw(15) << "EntryNumber";
    output << setw(15) << "Name";
    output << setw(15) << "Length";
    output << setw(15) << "SymbolEntryNumber";
    output << endl;

    map<IdSection, SectionEntry>::iterator it;

    for (it = table.begin(); it != table.end(); it++)
    {
        output << left;
        output << setw(15) << hex << it->second.entryNo;
        output << setw(15) << it->second.name;
        output << setw(15) << hex << it->second.length;
        output << setw(15) << hex << it->second.SymbolEntryNo;
        output << endl;
    }

    return output;

}

void RelocationTable::insertRelocation(unsigned long section, unsigned long offset, RelocationType relocationType, unsigned long value)
{
    RelocationEntry entry(section, offset, relocationType, value);
    table.push_back(entry);
}

stringstream RelocationTable::generateTextualRelocationTable(IdSection idSection)
{
    stringstream output;

    output << left;
    output << setw(15) << "Offset";
    output << setw(15) << "RelocationType";
    output << setw(15) << "Value";
    output << endl;

    vector<RelocationEntry>::iterator it;

    for (it = table.begin(); it != table.end(); it++)
    {
        if (it->section == idSection) {
            output << left;
            output << setw(15) << hex << it->offset;
            output << setw(15) << (it->relocationType == RelocationType::R_386_PC16 ? "R_386_PC16" : "R_386_16");
            output << setw(15) << hex << it->value;
            output << endl;
        }
    }

    return output;
}

void TNSTable::insertSymbol(IdSection section, string name,string expression, Scope scope)
{
    vector<TNSEntry>::iterator it;

    TNSEntry entry(section, name, expression, scope);

    for (it = table.begin(); it != table.end(); it++)
        if (it->name == name)
            throw AssemblyException("TNS symbol '" + name + "' is already declared.");	

    table.push_back(entry);
}

TNSEntry* TNSTable::getEntryByID(unsigned id)
{	
    return &table.at(id);
}

TNSEntry* TNSTable::getEntryByName(string name)
{
    for (size_t i = 0; i < table.size(); i++)
        if (table.at(i).name == name)
            return &table.at(i);

    return nullptr;
}

void TNSTable::deleteEntryByName(string name)
{
    for (size_t i = 0; i < table.size(); i++)
        if (table.at(i).name == name)
            table.erase(table.begin() + i);
}

/*
stringstream TNSTable::generateTextualTNSTable()
{
    stringstream output;

    output << left;
    output << setw(15) << "Section";
    output << setw(15) << "Name";
    output << setw(15) << "Expression";
    output << setw(15) << "Scope";
    output << endl;
    
    for (size_t i = 0; i < table.size(); i++)
    {
        TNSEntry& entry = table.at(i);

        output << left;
        output << setw(15) << hex << entry.section;
        output << setw(15) << entry.name;
        output << setw(15) << hex << entry.expression;
        output << setw(15) << entry.scope;
        output << endl;
    }

    return output;
}
*/
