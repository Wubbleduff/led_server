FLAGS=-Wall -O2 -Wconversion -lstdc++ -o go.exe
SOURCE=./source/main.cpp

gnu :
	g++ -ldl $(FLAGS) $(SOURCE)


