#for compiling and executing the program.(N.B: specific for mac operating system only) 
all:
	g++ -std=c++17 gameLoop.cpp -o game -I $$PWD/src/include -L $$PWD/src/lib -lSDL2 -lSDL2_ttf && export DYLD_LIBRARY_PATH=$$PWD/src/lib:$$DYLD_LIBRARY_PATH && ./game
