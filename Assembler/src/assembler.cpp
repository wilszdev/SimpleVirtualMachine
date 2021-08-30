#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <memory>
#include <cassert>
#include <functional>
#include <unordered_map>
#include "Lexer.h"
#include "Definitions.h"
#include "Instruction.h"

void PrintUsage(const char*);
std::vector<i32> compileForStackVM(const std::string& filecontents);
i32 mapToStackVmInstruction(const std::string& s);
std::vector<byte> compileForRegVM(const std::vector<std::string>& lines);
template<typename T>
void AppendToCode(std::vector<byte>* out, const T& op)
{
	if (sizeof(op) == 1)
	{
		out->push_back(AsType<byte>(op));
	}
	else if (sizeof(op) == 8)
	{
		const byte* mem = (byte*)&op;
		for (size_t i = 0; i < 8; ++i)
			out->push_back(mem[i]);
	}
	else
	{
		assert(false);
	}
}

template<typename T> void InsertInCode(std::vector<byte>* out, size_t offset, const T& op);
bool isInteger(const std::string& s);

int main(int argc, char** argv)
{
	const char* inputfile = nullptr; // <path>
	const char* outputfile = nullptr; // -o <path>
	const char* mode = nullptr; // -m <s/r>

#pragma warning(push)
#pragma warning(disable: 28182)
	// parse
	bool validArgs = true;
	for (int i = 1; i < argc; ++i)
	{
		if (strcmp(argv[i], "-o") == 0)
		{
			/* valid if:
			* - not already set
			* - haven't reached end of args
			* - previous arg doesnt start with tack
			*/
			if (outputfile == nullptr && i + 1 < argc && argv[i - 1][0] != '-')
			{
				outputfile = argv[i + 1];
				i++; // skip
			}
			else
			{
				validArgs = false;
				break;
			}
		}
		else if (strcmp(argv[i], "-m") == 0)
		{
			if (mode == nullptr && i + 1 < argc && argv[i - 1][0] != '-')
			{
				mode = argv[i + 1];
				i++;
			}
			else
			{
				validArgs = false;
				break;
			}
		}
		else if (inputfile == nullptr && argv[i - 1][0] != '-')
		{
			inputfile = argv[i];
		}
		else
		{
			std::cout << "invalid argument '" << argv[i] << "'" << std::endl;
			validArgs = false;
			break;
		}
	}
#pragma warning(pop)
	// default value
	if (!outputfile) outputfile = "out.bin";
	// validate
	if (!validArgs || !mode || !inputfile || !outputfile)
	{
		PrintUsage(argv[0]);
		return -1;
	}
	// read input file
	std::ifstream infile(inputfile);
	if (!infile.is_open())
	{
		std::cout << "error: unable to open file [" << inputfile << "]" << std::endl;
		return -1;
	}
	std::string line;
	std::string contents;
	std::vector<std::string> lines;
	while (std::getline(infile, line)) { contents += line + '\n'; lines.push_back(line); }
	infile.close();
	// compile
	if (*mode == 's')
	{
		std::vector<i32> instructions = compileForStackVM(contents);
		// write to file
		std::ofstream ofile(outputfile, std::ios::binary);
		if (!ofile.is_open())
		{
			std::cout << "error: unable to create output file [" << outputfile << "]" << std::endl;
			return -1;
		}
		for (size_t i = 0; i < instructions.size(); i++)
			ofile.write(reinterpret_cast<char*>(&instructions[i]), sizeof(i32));
		ofile.close();
	}
	else if (*mode == 'r')
	{
		std::vector<byte> instructions = compileForRegVM(lines);
		// write to file
		std::ofstream ofile(outputfile, std::ios::binary);
		if (!ofile.is_open())
		{
			std::cout << "error: unable to create output file [" << outputfile << "]" << std::endl;
			return -1;
		}
		for (size_t i = 0; i < instructions.size(); i++)
			ofile.write(reinterpret_cast<char*>(&instructions[i]), sizeof(byte));
		ofile.close();
	}
	else
	{
		std::cout << "error: invalid mode '" << mode << "'" << std::endl;
		return -1;
	}
	// done
	return 0;
}

void PrintUsage(const char* argv0)
{
	std::cout << "usage: " << argv0 << " <input file> -m <s/r> [-o <output file>]" << std::endl;
}

std::vector<i32> compileForStackVM(const std::string& filecontents)
{
	Lexer lexer;
	std::vector<std::string> s = lexer.lex(filecontents);
	std::vector<i32> instructions;
	for (size_t i = 0; i < s.size(); i++)
	{
		i32 instruction = mapToStackVmInstruction(s[i]);
		if (instruction == -1)
		{
			std::cout << "error: invalid instruction [" << s[i] << "]" << std::endl;
		}
		else
		{
			instructions.push_back(instruction);
		}
	}
	instructions.push_back(Instruction::Create(VMs::Stack::Opcode::HALT));
	return instructions;
}

i32 mapToStackVmInstruction(const std::string& s)
{
	using op = VMs::Stack::Opcode;
	if (isInteger(s))
	{
		return Instruction::Create(op::PUSH, std::stoi(s));
	}
	else if (s == "+")
	{
		return Instruction::Create(op::ADD);
	}
	else if (s == "-")
	{
		return Instruction::Create(op::SUB);
	}
	else if (s == "*")
	{
		return Instruction::Create(op::MUL);
	}
	else if (s == "/")
	{
		return Instruction::Create(op::DIV);
	}
	return -1;
}

template<typename T>
void InsertInCode(std::vector<byte>* out, size_t offset, const T& op)
{
	if (sizeof(op) == 1)
	{
		(*out)[offset] = AsType<byte>(op);
	}
	else if (sizeof(op) == 8)
	{
		const byte* mem = (byte*)&op;
		for (size_t i = 0; i < 8; ++i)
			(*out)[offset + i] = mem[i];
	}
	else
	{
		assert(false);
	}
}

#pragma region macros

#define APP(x) AppendToCode(&code, (x))
#define INS(x, offset) InsertInCode(&code, (offset), (x))

#define IS_REG(string) ( string == "a" || string == "b" || string == "c" || string == "ip" || string == "sp" || string == "f" )
#define APP_REG(string) { \
	if (string == "a") {APP(VMs::Reg::Regcode::A);		}	\
	else if (string == "b") {APP(VMs::Reg::Regcode::B);	}	\
	else if (string == "c")	{APP(VMs::Reg::Regcode::C);	}	\
	else if (string == "ip") {APP(VMs::Reg::Regcode::IP);}	\
	else if (string == "sp") {APP(VMs::Reg::Regcode::SP);}	\
	else if (string == "f")	{APP(VMs::Reg::Regcode::F);	 } }
#define APP_REG_CHECK(string) {if (IS_REG(string)) {APP_REG(string);} else { ss.str(""); ss.clear(); ss << "invalid register identifier [" << string << "]"; errors.push_back({ss.str(), i}); } }

#define CHECK_N_TOK(n) {if (tokens.size() != n) { ss.str(""); ss.clear(); ss << "instruction requires " << n << " tokens"; errors.push_back({ss.str(), i}); continue; }}

#define APP_JMP_TARGET(token) {if (isInteger(token)) {APP(std::stoull(token));} else { bool found = false; for (const auto& l : labels)	{ if (l.name == token) { found = true; APP(l.addr); break;}} if (!found) { undefinedLabels[token].push_back(code.size()); APP((u64)-1); } }}
#define APP_JMP(opcode) {CHECK_N_TOK(2); APP(opcode); APP_JMP_TARGET(tokens[1]);}

#define APP_ARITH_2(opcode) {CHECK_N_TOK(3); APP(opcode); APP_REG_CHECK(tokens[1]); APP_REG_CHECK(tokens[2]);}
#define APP_ARITH_1(opcode) {CHECK_N_TOK(2); APP(opcode); APP_REG_CHECK(tokens[1]);}

#define PUSH_INVALID_TOKEN_ERR(token) {ss.str(""); ss.clear(); ss << "invalid token [" << token << "]"; errors.push_back({ ss.str(), i });}

#pragma endregion

struct symbol
{
	std::string name;
	u64 addr;
	symbol(const std::string& nam, u64 address) : name(nam), addr(address) {}
};

struct error
{
	std::string description;
	size_t lineIndex;
};

std::vector<byte> compileForRegVM(const std::vector<std::string>& lines)
{
	Lexer lexer;
	std::vector<byte> code;
	std::vector<error> errors;
	std::vector<symbol> procs;
	size_t endpCount = 0;
	std::vector<symbol> labels;
	std::unordered_map<std::string, std::vector<size_t>> undefinedProcs;
	std::unordered_map<std::string, std::vector<size_t>> undefinedLabels;

	std::stringstream ss;

	// call the main function
	APP(VMs::Reg::Opcode::CALLI);		// call immediate
	APP((u64)-1);						// placeholder address
	APP(VMs::Reg::Opcode::HALT);		// halt after main returns
	undefinedProcs["main"] = { 0x01 };	// so that linker will replace the placeholder address

	std::string currentProc = "";
	for (size_t i = 0; i < lines.size(); ++i)
	{
		auto tokens = lexer.lex(lines[i]);
		if (tokens.size() == 0) continue;
		/* symbols */
		if (tokens[0] == "proc")
		{
			CHECK_N_TOK(2);
			const std::string& name = tokens[1];
			currentProc = name;
			procs.push_back(symbol(name, code.size()));
		}
		else if (tokens[0] == "endp")
		{
			CHECK_N_TOK(1);
			// replace undefined labels with the correct addresses
			for (const auto& entry : undefinedLabels)
			{
				bool found = false;
				for (const auto& p : labels)
				{
					if (entry.first == p.name)
					{
						found = true;
						for (const auto& offset : entry.second)
						{
							INS(p.addr, offset);
						}
						break;
					}
				}
				if (!found)
				{
					ss.str(""); ss.clear();
					ss << "unresolved symbol [" << entry.first << "] in proc [" << currentProc << "]";
					errors.push_back({ ss.str(), i });
				}
			}
			std::vector<symbol>().swap(labels);
			undefinedLabels.clear();
			++endpCount;
		}
		else if (tokens.size() == 2 && tokens[1] == ":") {
			labels.push_back(
				symbol(
					tokens[0],
					code.size()
				)
			);
		}
		/* calling */
		else if (tokens[0] == "call")
		{
			CHECK_N_TOK(2);
			if (isInteger(tokens[1]))
			{
				// calli
				APP(VMs::Reg::Opcode::CALLI);
				APP(std::stoull(tokens[1]));
			}
			else if (IS_REG(tokens[1]))
			{
				// callr
				APP(VMs::Reg::Opcode::CALLR);
				APP_REG(tokens[1]);
			}
			else
			{
				// calli a proc
				APP(VMs::Reg::Opcode::CALLI);
				bool found = false;
				for (const auto& p : procs)
				{
					if (p.name == tokens[1])
					{
						found = true;
						APP(p.addr);
						break;
					}
				}
				if (!found)
				{
					undefinedProcs[tokens[1]].push_back(code.size());
					APP((u64)-1);
				}
			}
		}
		else if (tokens[0] == "ret") { CHECK_N_TOK(1); APP(VMs::Reg::Opcode::RET); }
		/* jumping */
		else if (tokens[0] == "jmp") { APP_JMP(VMs::Reg::Opcode::JMP); }
		else if (tokens[0] == "je") { APP_JMP(VMs::Reg::Opcode::JE); }
		else if (tokens[0] == "jz") { APP_JMP(VMs::Reg::Opcode::JZ); }
		else if (tokens[0] == "jne") { APP_JMP(VMs::Reg::Opcode::JNE); }
		else if (tokens[0] == "jnz") { APP_JMP(VMs::Reg::Opcode::JNZ); }
		else if (tokens[0] == "jgt") { APP_JMP(VMs::Reg::Opcode::JGT); }
		else if (tokens[0] == "jlt") { APP_JMP(VMs::Reg::Opcode::JLT); }
		else if (tokens[0] == "jge") { APP_JMP(VMs::Reg::Opcode::JGE); }
		else if (tokens[0] == "jle") { APP_JMP(VMs::Reg::Opcode::JLE); }
		/* arithmetic */
		else if (tokens[0] == "add") { APP_ARITH_2(VMs::Reg::Opcode::ADD); } // double arg
		else if (tokens[0] == "sub") { APP_ARITH_2(VMs::Reg::Opcode::SUB); }
		else if (tokens[0] == "mul") { APP_ARITH_2(VMs::Reg::Opcode::MUL); }
		else if (tokens[0] == "div") { APP_ARITH_2(VMs::Reg::Opcode::DIV); }
		else if (tokens[0] == "mod") { APP_ARITH_2(VMs::Reg::Opcode::MOD); }
		else if (tokens[0] == "cmp") { APP_ARITH_2(VMs::Reg::Opcode::CMP); }
		else if (tokens[0] == "and") { APP_ARITH_2(VMs::Reg::Opcode::AND); }
		else if (tokens[0] == "xor") { APP_ARITH_2(VMs::Reg::Opcode::XOR); }
		else if (tokens[0] == "not") { APP_ARITH_2(VMs::Reg::Opcode::NOT); }
		else if (tokens[0] == "shr") { APP_ARITH_2(VMs::Reg::Opcode::SHR); }
		else if (tokens[0] == "shl") { APP_ARITH_2(VMs::Reg::Opcode::SHL); }
		else if (tokens[0] == "inc") { APP_ARITH_1(VMs::Reg::Opcode::INC); } // single arg
		else if (tokens[0] == "dec") { APP_ARITH_1(VMs::Reg::Opcode::DEC); }
		else if (tokens[0] == "or ") { APP_ARITH_1(VMs::Reg::Opcode::OR); }
		/* registers */
		else if (tokens[0] == "clf") { CHECK_N_TOK(1); APP(VMs::Reg::Opcode::CLF); }
		else if (tokens[0] == "mov")
		{
			// MOV	REG,	REG
			// MOVI	REG,	IMM
			// MOVF	REG,	ADDR
			// MOVT	ADDR,	REG
			CHECK_N_TOK(3);
			if (IS_REG(tokens[1]))
			{
				if (IS_REG(tokens[2]))
				{
					APP(VMs::Reg::Opcode::MOV);
					APP_REG(tokens[1]);
					APP_REG(tokens[2]);
				}
				else if (isInteger(tokens[2]))
				{
					APP(VMs::Reg::Opcode::MOVI);
					APP_REG(tokens[1]);
					APP(std::stoll(tokens[2]));
				}
				else if (0)
				{
					// TODO: square brackets == address
					APP(VMs::Reg::Opcode::MOVF);
					APP_REG(tokens[1]);
					APP(std::stoll(tokens[2]));
				}
				else { PUSH_INVALID_TOKEN_ERR(tokens[2]); }
			}
			// todo: square bracket == addr
			else if (isInteger(tokens[1]) && IS_REG(tokens[2]))
			{
				APP(std::stoull(tokens[1]));
			}
			else { PUSH_INVALID_TOKEN_ERR(tokens[1]); }
		}
		else if (tokens[0] == "push")
		{
			// PUSH REG
			// PUSH IMM
			CHECK_N_TOK(2);
			if (isInteger(tokens[1]))
			{
				APP(VMs::Reg::Opcode::PUSHI);
				APP(std::stoull(tokens[1]));
			}
			else if (IS_REG(tokens[1]))
			{
				APP(VMs::Reg::Opcode::PUSH);
				APP_REG(tokens[1]);
			}
			else
			{
				PUSH_INVALID_TOKEN_ERR(tokens[1]);
			}
		}
		else if (tokens[0] == "pushf") { CHECK_N_TOK(1); APP(VMs::Reg::Opcode::PUSHF); }
		else if (tokens[0] == "pop")
		{
			// POP REG
			// POP ADDR
			CHECK_N_TOK(2);
			if (isInteger(tokens[1]))
			{
				APP(VMs::Reg::Opcode::POPTO);
				APP(std::stoull(tokens[1]));
			}
			else if (IS_REG(tokens[1]))
			{
				APP(VMs::Reg::Opcode::POP);
				APP_REG(tokens[1]);
			}
			else
			{
				PUSH_INVALID_TOKEN_ERR(tokens[1]);
			}
		}
		else if (tokens[0] == "popf") { CHECK_N_TOK(1); APP(VMs::Reg::Opcode::POPF); }
		/* misc */
		else if (tokens[0] == "nop") { CHECK_N_TOK(1); APP(VMs::Reg::Opcode::NOP); }
		else if (tokens[0] == "int") { CHECK_N_TOK(1); APP(VMs::Reg::Opcode::INT); }
		else if (tokens[0] == "halt") { CHECK_N_TOK(1); APP(VMs::Reg::Opcode::HALT); }
		/* invalid */
		else { PUSH_INVALID_TOKEN_ERR(tokens[0]); }
	}
	// go through undefinedprocs and replace code at given locations with addr of proc
	for (const auto& entry : undefinedProcs)
	{
		bool found = false;
		for (const auto& p : procs)
		{
			if (entry.first == p.name)
			{
				found = true;
				for (const auto& offset : entry.second)
				{
					INS(p.addr, offset);
				}
				break;
			}
		}
		if (!found)
		{
			ss.str(""); ss.clear();
			ss << "unresolved symbol [" << entry.first << "]";
			errors.push_back({ ss.str(), (size_t)-1 });
		}
	}

	// print any errors
	if (errors.size() > 0)
	{
		std::cout << errors.size() << " compilation errors occurred:\n--------------------\n";
		for (const auto& e : errors)
		{
			std::cout << "on line " << e.lineIndex + 1 << ":\t" << e.description << std::endl;
		}
		return {};
	}
	else
		return code;
}

bool isInteger(const std::string& s)
{
	size_t i = 0;
	if (s[0] == '-' || s[0] == '+') i = 1; // skip
	for (i; i < s.size(); ++i)
		if (!std::isdigit(s[i]))
			return false;
	return true;
}