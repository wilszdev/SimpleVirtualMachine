#include "StackVM.h"

void StackVM::fetch()
{
	m_context.programCtr++;
}

void StackVM::decode()
{
	u32 instruction = m_context.memory[m_context.programCtr];
	m_context.next = Instruction::Get(instruction);
}

void StackVM::execute()
{
	m_handlers[m_context.next.opcode](&m_context);
}

StackVM::StackVM()
{
	u32 sz = 1000000;
	m_context.memory = new (std::nothrow) u32[sz];
	if (m_context.memory)
		memset(m_context.memory, 0, sz);
	m_context.stackPtr = sz;
	if (!m_context.memory)
	{
		// uh oh
		m_context.running = FALSE;
	}
	configure();
}

StackVM::~StackVM()
{
	delete[] m_context.memory;
}

void StackVM::Run()
{
	while (m_context.running)
	{
		std::cout << "tos: " << (i32)m_context.memory[m_context.stackPtr] << std::endl;
		fetch();
		decode();
		execute();
	}
}

void StackVM::LoadProgram(const void* program, size_t size)
{
	memcpy(m_context.memory, program, size);
}

void StackVM::configure()
{
	// instruction handlers
	m_handlers[op::NOP] = [](Context* c)
	{
		puts("NOP");
	};
	m_handlers[op::HALT] = [](Context* c)
	{
		puts("HALT");
		c->running = FALSE;
	};
	m_handlers[op::ALERT] = [](Context* c)
	{
		puts("ALERT!!");
	};
	m_handlers[op::PUSH] = [](Context* c)
	{
		puts("PUSH");
		c->memory[--c->stackPtr] = c->next.data;
	};
	m_handlers[op::ADD] = [](Context* c)
	{
		puts("ADD");
		*(i32*)&c->memory[c->stackPtr + 1] = (i16)c->memory[c->stackPtr + 1] + (i16)c->memory[c->stackPtr];
		++c->stackPtr;
	};
	m_handlers[op::SUB] = [](Context* c)
	{
		puts("SUB");
		*(i32*)&c->memory[c->stackPtr + 1] = (i16)c->memory[c->stackPtr + 1] - (i16)c->memory[c->stackPtr];
		++c->stackPtr;
	};
	m_handlers[op::MUL] = [](Context* c)
	{
		puts("MUL");
		*(i32*)&c->memory[c->stackPtr + 1] = (i16)c->memory[c->stackPtr + 1] * (i16)c->memory[c->stackPtr];
		++c->stackPtr;
	};
	m_handlers[op::DIV] = [](Context* c)
	{
		puts("DIV");
		*(i32*)&c->memory[c->stackPtr + 1] = (i16)c->memory[c->stackPtr + 1] / (i16)c->memory[c->stackPtr];
		++c->stackPtr;
	};
}