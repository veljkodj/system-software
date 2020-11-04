final: assembler clear 
	
assembler: arithmetic.o assembler.o main.o structures.o token.o
	g++ -o assembler arithmetic.o assembler.o main.o structures.o token.o

arithmetic.o: ../src/arithmetic.h ../src/arithmetic.cpp
	g++ -c ../src/arithmetic.cpp

assembler.o: ../src/assembler.h ../src/assembler.cpp
	g++ -c ../src/assembler.cpp

main.o: ../src/main.cpp
	g++ -c ../src/main.cpp

structures.o: ../src/structures.h ../src/structures.cpp
	g++ -c ../src/structures.cpp

token.o: ../src/token.h ../src/token.cpp
	g++ -c ../src/token.cpp

clear:
	rm *.o