#pragma once
#include "Definitions.h"

#define EXPORT __declspec(dllexport)

struct Instruction
{
public:
	u16 opcode = 0;
	u16 data = 0;
public:
	static EXPORT Instruction Get(u32 instruction);
	static EXPORT u16 GetOpcode(u32 instruction);
	static EXPORT u16 GetData(u32 instruction);
	static EXPORT u32 Create(u16 opcode = 0, u16 data = 0);
};