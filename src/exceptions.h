#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <iostream>
#include <string.h>
#include <sstream>

using namespace std;

class AssemblyException : public exception
{
public:
    
    AssemblyException(string message) noexcept : exception(), message(message), line(-1) {}
    AssemblyException(string message, unsigned long line) noexcept : exception(), message(message), line(line) {}

    ~AssemblyException()
    {
        delete[] a[0];
        delete a;
    }

    const char* what() const noexcept override
    {

        ostringstream temp;

        temp << "Error: ";
        
        if (line != -1)
            temp << " on line " << line << ": ";
        else
            temp << ": ";

        temp << message;

        string errorDescription = temp.str();

        char *cstr = new char[errorDescription.length() + 1];
        strcpy(cstr, errorDescription.c_str());
        a[0] = cstr;

        return cstr;
    
    }

private:

    string message;
    unsigned long line;

    char** a = new char*[1];
    
};

#endif