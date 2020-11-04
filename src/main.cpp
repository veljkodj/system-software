#include <iostream>

#include "token.h"
#include "structures.h"
#include "enums.h"
#include "arithmetic.h"
#include "exceptions.h"
#include "assembler.h"

using namespace std;

int main(int argc, char** argv) {

    if (argc < 4)
    {
        cout << "Invalid call parameters. Syntax is assembler -o output_file input_file" << endl;
        return -1;
    }

    try
    {
        if (string(argv[1]) == "-o")
        {

            Assembler* assembler = new Assembler(argv[3], argv[2]);

            assembler->generate();

            delete assembler;

            cout << "Output file is generated." << endl;
        
        }

        return 0;
    
    }
    catch (AssemblyException& ex)
    {
        cout << ex.what() << endl;
    }
    catch (exception& ex)
    {
        cout << ex.what() << endl;
    }

    return 0;

}