#pragma once

#include <cstdio>
#include <vector>

#include "VM.h"
#include "Definitions.h"
#include "Instruction.h"

/* INSTRUCTIONS
	8-bit opcodes
	64-bit values
	produces 1 and 9 byte instructions
*/
/* FLAGS REGISTER
	starting from LSB:
	0: zero
	1: sign
	2: overflow
*/

class RegVM final : public VM
{
public:
	typedef VMs::Reg::Opcode op;
	typedef VMs::Reg::Regcode reg;
	struct Context
	{
		i64 r[reg::REG_END] = {};
		byte* mem = nullptr;
		bool running = false;
	};
	typedef void(*opHandler)(Context*);
private:
	Context m_context;
	opHandler m_opTable[256];
	//size_t m_opSizeTable[256];
public:
	RegVM();
	~RegVM();
	void LoadProgram(const void* mem, size_t size) override;
	void Run() override;
	void PrintState();
	//size_t GetInstructionSize(byte opcode);
private:
	void Reset();
	void Configure();
};

inline void SetArithmeticFlags(i64 value, RegVM::Context* c)
{
	c->r[RegVM::reg::F] = SetBit(c->r[RegVM::reg::F], 0, value == 0); // zero flag
	c->r[RegVM::reg::F] = SetBit(c->r[RegVM::reg::F], 1, value < 0);  // sign flag
}

class OpImpl
{
	using reg = RegVM::reg;
public:
#pragma region misc
	static void _halt(RegVM::Context* c)
	{
		c->running = false;
	}

	static void _nop(RegVM::Context* c)
	{
		// do nothing
	}

	static void _int(RegVM::Context* c)
	{
		printf("REGISTERS:\n------------\n"
			"a:\t0x%016llx ( %lld )\n"
			"b:\t0x%016llx ( %lld )\n"
			"c:\t0x%016llx ( %lld )\n"
			"ip:\t0x%016llx ( %lld )\n"
			"sp:\t0x%016llx ( %lld )\n"
			"f:\t0x%016llx ( %lld )\n",
			c->r[0], c->r[0],
			c->r[1], c->r[1],
			c->r[2], c->r[2],
			c->r[3], c->r[3],
			c->r[4], c->r[4],
			c->r[5], c->r[5]
		);
	}
#pragma endregion

#pragma region registers
	static void _clf(RegVM::Context* c)
	{
		c->r[reg::F] = 0;
	}

	static void _movi(RegVM::Context* c)
	{
		// read args
		u64 regcode = AsType<u64>(c->mem[c->r[reg::IP] + 1]);
		i64 value = AsType<i64>(c->mem[c->r[reg::IP] + 1 + 8]);
		// do stuff
		memcpy(&c->r[regcode], &value, 8);
		// avoid reading the 2 arguments as opcodes
		c->r[reg::IP] += 16;
	}

	static void _movf(RegVM::Context* c)
	{
		// read args
		u64 regcode = AsType<u64>(c->mem[c->r[reg::IP] + 1]);
		u64 address = AsType<u64>(c->mem[c->r[reg::IP] + 1 + 8]);
		// do stuff
		memcpy(&c->r[regcode], &c->mem[address], 8);
		// avoid reading the 2 arguments as opcodes
		c->r[reg::IP] += 16;
	}

	static void _movt(RegVM::Context* c)
	{
		// read args
		u64 address = AsType<u64>(c->mem[c->r[reg::IP] + 1]);
		u64 regcode = AsType<u64>(c->mem[c->r[reg::IP] + 1 + 8]);
		// do stuff
		memcpy(&c->mem[address], &c->r[regcode], 8);
		// avoid reading the 2 arguments as opcodes
		c->r[reg::IP] += 16;
	}

	static void _mov(RegVM::Context* c)
	{
		// read args
		u64 reg1 = AsType<u64>(c->mem[c->r[reg::IP] + 1]);
		u64 reg2 = AsType<u64>(c->mem[c->r[reg::IP] + 1 + 8]);
		// move value
		memcpy(&c->r[reg1], &c->r[reg2], 8);
		// avoid reading args as opcodes
		c->r[reg::IP] += 16;
	}

	static void _push(RegVM::Context* c)
	{
		// read arg
		u64 regcode = AsType<u64>(c->mem[c->r[reg::IP] + 1]);
		// grow stack
		c->r[reg::SP] -= 8;
		// place value on stack
		memcpy(&c->mem[c->r[reg::SP]], &c->r[regcode], 8);
		// avoid reading args as opcode
		c->r[reg::IP] += 8;
#ifndef NDEBUG
		printf("pushed value %lld from register %llu onto stack\n", c->mem[c->r[reg::SP]], regcode);
#endif
	}

	static void _pushf(RegVM::Context* c)
	{
		c->r[reg::SP] -= 8;
		memcpy(&c->mem[c->r[reg::SP]], &c->r[reg::F], 8);
	}

	static void _pushi(RegVM::Context* c)
	{
		// grow stack
		c->r[reg::SP] -= 8;
		// place value on stack. read argument in this step too
		memcpy(&c->mem[c->r[reg::SP]], &c->mem[c->r[reg::IP] + 1], 8);
		// avoid reading args as opcode
		c->r[reg::IP] += 8;
#ifndef NDEBUG
		printf("pushed value %lld onto the stack\n", c->mem[c->r[reg::SP]]);
#endif
	}

	static void _pop(RegVM::Context* c)
	{
		// read arg
		u64 regcode = AsType<u64>(c->mem[c->r[reg::IP] + 1]);
#ifndef NDEBUG
		
		printf("popping a value into register %llu\n", regcode);
#endif
		// get value from stack
		memcpy(&c->r[regcode], &c->mem[c->r[reg::SP]], 8);
		// shrink stack
		c->r[reg::SP] += 8;
		// avoid reading args as opcode
		c->r[reg::IP] += 8;
#ifndef NDEBUG
		printf("popped value %lld into register %llu\n", c->r[regcode], regcode);
#endif
	}

	static void _popf(RegVM::Context* c)
	{
		memcpy(&c->r[reg::F], &c->mem[c->r[reg::SP]], 8);
		c->r[reg::SP] += 8;
	}

#pragma endregion

#pragma region arithmetic

public:

	static void _add(RegVM::Context* c)
	{
		u64 r1 = AsType<u64>(c->mem[c->r[reg::IP] + 1]);
		u64 r2 = AsType<u64>(c->mem[c->r[reg::IP] + 1 + 8]);
		c->r[r1] = c->r[r1] + c->r[r2];
		SetArithmeticFlags(c->r[r1], c);
		c->r[reg::IP] += 16;
	}

	static void _cmp(RegVM::Context* c)
	{
		u64 r1 = AsType<u64>(c->mem[c->r[reg::IP] + 1]);
		u64 r2 = AsType<u64>(c->mem[c->r[reg::IP] + 1 + 8]);
		i64 res = c->r[r1] - c->r[r2];
		SetArithmeticFlags(res, c);
		c->r[reg::IP] += 16;
#ifndef NDEBUG
		printf("comparing registers %llu and %llu. got result %lld.\n", r1, r2, res);
#endif
	}

	static void _sub(RegVM::Context* c)
	{
		u64 r1 = AsType<u64>(c->mem[c->r[reg::IP] + 1]);
		u64 r2 = AsType<u64>(c->mem[c->r[reg::IP] + 1 + 8]);
		c->r[r1] = c->r[r1] - c->r[r2];
		SetArithmeticFlags(c->r[r1], c);
		c->r[reg::IP] += 16;
	}

	static void _mul(RegVM::Context* c)
	{
		u64 r1 = AsType<u64>(c->mem[c->r[reg::IP] + 1]);
		u64 r2 = AsType<u64>(c->mem[c->r[reg::IP] + 1 + 8]);
		c->r[r1] = c->r[r1] * c->r[r2];
		SetArithmeticFlags(c->r[r1], c);
		c->r[reg::IP] += 16;
	}

	static void _div(RegVM::Context* c)
	{
		u64 r1 = AsType<u64>(c->mem[c->r[reg::IP] + 1]);
		u64 r2 = AsType<u64>(c->mem[c->r[reg::IP] + 1 + 8]);
		c->r[r1] = c->r[r1] / c->r[r2];
		SetArithmeticFlags(c->r[r1], c);
		c->r[reg::IP] += 16;
	}

	static void _mod(RegVM::Context* c)
	{
		u64 r1 = AsType<u64>(c->mem[c->r[reg::IP] + 1]);
		u64 r2 = AsType<u64>(c->mem[c->r[reg::IP] + 1 + 8]);
		c->r[r1] = c->r[r1] % c->r[r2];
		SetArithmeticFlags(c->r[r1], c);
		c->r[reg::IP] += 16;
	}

	static void _inc(RegVM::Context* c)
	{
		// read arg
		u64 r1 = AsType<u64>(c->mem[c->r[reg::IP] + 1]);
		// get value from stack
		++c->r[r1];
		SetArithmeticFlags(c->r[r1], c);
		// avoid reading args as opcode
		c->r[reg::IP] += 8;
	}

	static void _dec(RegVM::Context* c)
	{
		// read arg
		u64 r1 = AsType<u64>(c->mem[c->r[reg::IP] + 1]);
		// get value from stack
		--c->r[r1];
		SetArithmeticFlags(c->r[r1], c);
		// avoid reading args as opcode
		c->r[reg::IP] += 8;
	}

	static void _and(RegVM::Context* c)
	{
		u64 r1 = AsType<u64>(c->mem[c->r[reg::IP] + 1]);
		u64 r2 = AsType<u64>(c->mem[c->r[reg::IP] + 1 + 8]);
		c->r[r1] = c->r[r1] & c->r[r2];
		SetArithmeticFlags(c->r[r1], c);
		c->r[reg::IP] += 16;
	}

	static void _or(RegVM::Context* c)
	{
		u64 r1 = AsType<u64>(c->mem[c->r[reg::IP] + 1]);
		u64 r2 = AsType<u64>(c->mem[c->r[reg::IP] + 1 + 8]);
		c->r[r1] = c->r[r1] | c->r[r2];
		SetArithmeticFlags(c->r[r1], c);
		c->r[reg::IP] += 16;
	}

	static void _xor(RegVM::Context* c)
	{
		u64 r1 = AsType<u64>(c->mem[c->r[reg::IP] + 1]);
		u64 r2 = AsType<u64>(c->mem[c->r[reg::IP] + 1 + 8]);
		c->r[r1] = c->r[r1] ^ c->r[r2];
		SetArithmeticFlags(c->r[r1], c);
		c->r[reg::IP] += 16;
	}

	static void _not(RegVM::Context* c)
	{
		u64 r1 = AsType<u64>(c->mem[c->r[reg::IP] + 1]);
		c->r[r1] = ~c->r[r1];
		SetArithmeticFlags(c->r[r1], c);
		c->r[reg::IP] += 8;
	}

	static void _shr(RegVM::Context* c)
	{
		u64 r1 = AsType<u64>(c->mem[c->r[reg::IP] + 1]);
		u64 val = AsType<u64>(c->mem[c->r[reg::IP] + 1 + 8]);
		c->r[r1] = c->r[r1] >> val;
		SetArithmeticFlags(c->r[r1], c);
		c->r[reg::IP] += 16;
	}

	static void _shl(RegVM::Context* c)
	{
		u64 r1 = AsType<u64>(c->mem[c->r[reg::IP] + 1]);
		u64 val = AsType<u64>(c->mem[c->r[reg::IP] + 1 + 8]);
		c->r[r1] = c->r[r1] << val;
		SetArithmeticFlags(c->r[r1], c);
		c->r[reg::IP] += 16;
	}
#pragma endregion

#pragma region jumping/calling
	static void _calli(RegVM::Context* c)
	{
		// address of subroutine
		u64 address = AsType<u64>(c->mem[c->r[reg::IP] + 1]);
		c->r[reg::IP] += 8;
		// push ip to stack
		c->r[reg::SP] -= 8;
		memcpy(&c->mem[AsType<u64>(c->r[reg::SP])], &c->r[reg::IP], 8);
		// change ip
		c->r[reg::IP] = address - 1;
#ifndef NDEBUG
		printf("calling address %llu, return address is %llu\n", address, AsType<u64>(c->mem[c->r[reg::SP]]));
#endif
	}

	static void _callr(RegVM::Context* c)
	{
		u64 reg = AsType<u64>(c->mem[c->r[reg::IP] + 1]);
		c->r[reg::IP] += 8;
		// address of subroutine
		u64 address = AsType<u64>(&c->r[reg]);
		// push ip to stack
		c->r[reg::SP] -= 8;
		memcpy(&c->mem[AsType<u64>(c->r[reg::SP])], &c->r[reg::IP], 8);
		// change ip
		c->r[reg::IP] = address - 1;
#ifndef NDEBUG
		printf("calling address %llu\n", address);
#endif
	}

	static void _ret(RegVM::Context* c)
	{
		// get old ip from stack
		memcpy(&c->r[reg::IP], &c->mem[AsType<u64>(c->r[reg::SP])], 8);
		// shrink stack
		c->r[reg::SP] += 8;
#ifndef NDEBUG
		printf("returning to address %llu\n", AsType<u64>(c->r[reg::IP]));
#endif
	}

	static void _jmp(RegVM::Context* c)
	{
		u64 address = AsType<u64>(c->mem[c->r[reg::IP] + 1]);
		c->r[reg::IP] = address - 1u;
#ifndef NDEBUG
		printf("jmp to address %llu\n", address);
#endif
	}

	static void _jz(RegVM::Context* c)
	{
		// if zero flag set
		if (GetBit(c->r[reg::F], 0))
		{
			u64 address = AsType<u64>(c->mem[c->r[reg::IP] + 1]);
			c->r[reg::IP] = address - 1u;
#ifndef NDEBUG
			printf("jz to address %llu\n", address);
#endif
		}
		else
		{
			c->r[reg::IP] += 8;
		}
	}

	static void _jnz(RegVM::Context* c)
	{
		// if zero flag not set
		if (!(GetBit(c->r[reg::F], 0)))
		{
			u64 address = AsType<u64>(c->mem[c->r[reg::IP] + 1]);
			c->r[reg::IP] = address - 1u;
#ifndef NDEBUG
			printf("jnz to address %llu\n", address);
#endif
		}
		else
		{
			c->r[reg::IP] += 8;
		}
	}

	static void _jge(RegVM::Context* c)
	{
		// if zero flag set OR negative flag not set
		if (GetBit(c->r[reg::F], 0) || !(GetBit(c->r[reg::F], 1)))
		{
			u64 address = AsType<u64>(c->mem[c->r[reg::IP] + 1]);
			c->r[reg::IP] = address - 1u;
#ifndef NDEBUG
			printf("jge to address %llu\n", address);
#endif
		}
		else
		{
			c->r[reg::IP] += 8;
		}
	}

	static void _jle(RegVM::Context* c)
	{
		// if zero flag set OR negative flag set
		if (GetBit(c->r[reg::F], 0) || GetBit(c->r[reg::F], 1))
		{
			u64 address = AsType<u64>(c->mem[c->r[reg::IP] + 1]);
			c->r[reg::IP] = address - 1u;
#ifndef NDEBUG
			printf("jle to address %llu\n", address);
#endif
		}
		else
		{
			c->r[reg::IP] += 8;
		}
	}

	static void _jgt(RegVM::Context* c)
	{
		// if zero flag not set and negative flag not set
		if (!(GetBit(c->r[reg::F], 0)) && !(GetBit(c->r[reg::F], 1)))
		{
			u64 address = AsType<u64>(c->mem[c->r[reg::IP] + 1]);
			c->r[reg::IP] = address - 1u;
#ifndef NDEBUG
			printf("jgt to address %llu\n", address);
#endif
		}
		else
		{
			c->r[reg::IP] += 8;
		}
	}

	static void _jlt(RegVM::Context* c)
	{
		// if zero flag not set and negative flag set
		if (!(GetBit(c->r[reg::F], 0)) && GetBit(c->r[reg::F], 1))
		{
			u64 address = AsType<u64>(c->mem[c->r[reg::IP] + 1]);
			c->r[reg::IP] = address - 1u;
#ifndef NDEBUG
			printf("jlt to address %llu\n", address);
#endif
		}
		else
		{
			c->r[reg::IP] += 8;
		}
	}

#pragma endregion
};