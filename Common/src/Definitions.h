#pragma once

#include <cstdint>

#define TRUE 1
#define FALSE 0

#define MAX(a, b) ((a)>(b)?(a):(b))
#define MIN(a, b) ((a)<(b)?(a):(b))

typedef int64_t i64;
typedef int32_t i32;
typedef int16_t i16;
typedef int8_t i8;

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t  u8;
typedef uint8_t  byte;

namespace VMs
{
	class Stack
	{
	public:
		enum Opcode : u16
		{
			NOP = 0, HALT, ALERT, PUSH, ADD, SUB, MUL, DIV,
			OPCODE_END
		};
	};
	class Reg
	{
	public:
		enum Opcode : u8
		{
			/* register instructions */
			CLF,		// clear flags register
			MOVI,		// move immediate: set to immediate value
			MOVF,		// move from: load value from address
			MOVT,		// move to: set value at address
			MOV,		// move from reg to reg
			PUSH,		// push register to stack
			PUSHI,		// push immediate value
			POP,		// pop stack to register
			POPTO,		// pop value to address
			PUSHF,		// push flags register
			POPF,		// pop flags register

			/* arithmetic instructions */
			ADD, SUB, MUL, DIV, MOD, 
			CMP,		// compare 2 registers
			INC, DEC, 
			AND, OR, XOR, 
			NOT, 
			SHR, SHL, 

			/* jumping, calling instructions */
			CALLI,		// call immediate (address)
			CALLR,		// call address in register
			RET,		// return from subroutine
			JMP,		// unconditional jump............ jmp address
			JE, JZ,		// jump if zero flag set
			JNE, JNZ,	// jump if zero flag not set
			JGT,		// jump if zero flag not set and negative flag not set
			JLT,		// jump if zero flag not set and negative flag set
			JGE,		// jump if zero flag not set and negative flag not set
			JLE,		// jump if zero flag not set and negative flag set

			/* miscellaneous instructions */
			INT,		// interrupt
			NOP,		// no operation
			HALT,		// stop execution

			OPCODE_END
		};
		enum Regcode : u64
		{
			A, B, C, IP, SP, F,
			REG_END
		};
	};
};


static inline u64 SetBits(u64 target, u64 mask, bool value)
{
	return target ^ ((-(value ? 1 : 0) ^ target) & mask);
}

static inline u64 GetBits(u64 target, u64 mask)
{
	return target & mask;
}

static inline u64 GetBit(u64 target, u64 index)
{
	return (target & ((u64)1 << index)) >> index;
}

static inline u64 SetBit(u64 target, u64 index, bool value)
{
	return SetBits(target, (u64)1 << index, value);
}

template<typename T, typename CurType>
// changes ptr type and dereferences (does not cast value)
static inline T AsType(const CurType& value) { return *(T*)(&value); }