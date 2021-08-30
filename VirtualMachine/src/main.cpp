#include <iostream>
#include <fstream>
#include "StackVM.h"
#include "RegVM.h"
#include "Instruction.h"

int main(int argc, char** argv)
{
	// check args
	if (argc != 3)
	{
		std::cout << "usage: " << argv[0] << " <program file> <mode>\n\tmodes:\n\t\tr: register vm\n\t\ts: stack vm" << std::endl;
		return -1;
	}
	if (strlen(argv[2]) != 1)
	{
		std::cout << "invalid argument: " << argv[2] << std::endl;
		return -1;
	}
	// create vm 
	VM* vm = nullptr;
	if (*argv[2] == 's')
	{
		vm = new StackVM();
	}
	else if (*argv[2] == 'r')
	{
		vm = new RegVM();
	}
	else
	{
		std::cout << "error: invalid mode" << std::endl;
		return -1;
	}
	// open file
	std::ifstream infile(argv[1], std::ios::binary);
	if (!infile.is_open())
	{
		std::cout << "error opening program file [" << argv[1] << "]" << std::endl;
		return -1;
	}
	// find file size
	infile.seekg(0, std::ios::end);
	int rawSize = static_cast<int>(infile.tellg());
	// reset to beginning
	infile.seekg(0, std::ios::beg);
	// allocate memory and read program from file
	void* program = new byte[rawSize];
	infile.read((char*)program, rawSize);
	infile.close();
	// load and run program
	vm->LoadProgram(program, rawSize);
	delete[] program;

	vm->Run();

	delete vm;
	return 0;
}