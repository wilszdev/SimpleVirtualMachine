#pragma once

#include <iostream>
#include <cstdio>
#include <vector>
#include <functional>

#include "VM.h"
#include "Definitions.h"
#include "Instruction.h"

class StackVM final : public VM
{
private:
	struct Context
	{
		Instruction next;
		u32* memory = nullptr;
		i16 stackPtr = 0;
		i16 programCtr= -1;
		i32 running = TRUE;
	};
	using InstructionHandler = std::function<void(Context*)>;
	using op = VMs::Stack::Opcode;
private:
	InstructionHandler m_handlers[op::OPCODE_END];
	Context m_context;
private:
	void configure();
	void fetch();
	void decode();
	void execute();
public:
	StackVM();
	~StackVM();
	void Run() override;
	void LoadProgram(const void* program, size_t size) override;
};