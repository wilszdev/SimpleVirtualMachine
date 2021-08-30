#include "Instruction.h"

Instruction Instruction::Get(u32 instruction)
{
	return { Instruction::GetOpcode(instruction), Instruction::GetData(instruction) };
}
u16 Instruction::GetOpcode(u32 instruction)
{
	return instruction >> 16;
}

u16 Instruction::GetData(u32 instruction)
{
	return instruction & 0xffff;
}

u32 Instruction::Create(u16 opcode, u16 data)
{
	u32 upper = opcode << 16;
	u32 lower = data;
	u32 retval = upper | lower;
	return retval;
}