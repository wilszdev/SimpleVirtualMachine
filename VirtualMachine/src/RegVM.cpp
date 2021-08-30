#include "RegVM.h"

RegVM::RegVM()
{
	Reset();
	size_t memSize = 1024 * 1024;
	m_context.mem = reinterpret_cast<byte*>(malloc(memSize));
	m_context.r[reg::SP] = memSize - 1;
	Configure();
}

RegVM::~RegVM()
{
	free(m_context.mem);
}

void RegVM::LoadProgram(const void* mem, size_t size)
{
#ifndef NDEBUG
	printf("loading program of size %llu bytes\n", size);
#endif
	memcpy(m_context.mem, mem, size);
}

void RegVM::Run()
{
	m_context.running = true;
	m_context.r[reg::IP] = -1;
	while (m_context.running)
	{
		// increment the instruction pointer
		m_context.r[reg::IP]++;
		// read instruction byte, call relevant handler
#ifndef NDEBUG
		printf("----------\n");
		printf("tos: %lld\n", AsType<i64>(m_context.mem[AsType<u64>(m_context.r[reg::SP])]));
		printf("opcode: %u\n", AsType<u8>(m_context.mem[AsType<u64>(m_context.r[reg::IP])]));
		printf("ip: %llu\n", AsType<u64>(m_context.r[reg::IP]));
		printf("----------\n");
#endif
		m_opTable[
			m_context.mem[
				AsType<u64>(
					m_context.r[reg::IP]
					)
			]
		](&m_context);
	}
}

void RegVM::PrintState()
{
	printf("REGISTERS:\n------------\n"
		"a:\t0x%016llx ( %lld )\n"
		"b:\t0x%016llx ( %lld )\n"
		"c:\t0x%016llx ( %lld )\n"
		"ip:\t0x%016llx ( %lld )\n"
		"sp:\t0x%016llx ( %lld )\n"
		"f:\t0x%016llx ( %lld )\n",
		m_context.r[0], m_context.r[0],
		m_context.r[1], m_context.r[1],
		m_context.r[2], m_context.r[2],
		m_context.r[3], m_context.r[3],
		m_context.r[4], m_context.r[4],
		m_context.r[5], m_context.r[5]
	);
}

//size_t RegVM::GetInstructionSize(byte opcode)
//{
//	return m_opSizeTable[opcode];
//}

/* reset context. DOES NOT FREE MEMORY */
void RegVM::Reset()
{
	memset(&m_context, 0, sizeof(Context));
}

/* configure optable */
void RegVM::Configure()
{
	/* clear all to noop. should prevent crash on invalid opcode */
	for (int i = 0; i < 256; i++)
	{
		m_opTable[i] = OpImpl::_nop;
		//m_opSizeTable[i] = 1;
	}
	/* miscellaneous*/
	m_opTable[op::HALT] =	OpImpl::_halt;	//m_opSizeTable[op::HALT] = 1;
	m_opTable[op::NOP ] =	OpImpl::_nop;	//m_opSizeTable[op::NOP ] = 1;
	m_opTable[op::INT ] =	OpImpl::_int;	//m_opSizeTable[op::INT ] = 1;
	/* registers */							//
	m_opTable[op::CLF  ] =	OpImpl::_clf;	//m_opSizeTable[op::CLF  ] = 1;
	m_opTable[op::MOVF ] =	OpImpl::_movf;	//m_opSizeTable[op::MOVF ] = 1 + 8 + 8;
	m_opTable[op::MOVI ] =	OpImpl::_movi;	//m_opSizeTable[op::MOVI ] = 1 + 8 + 8;
	m_opTable[op::MOVT ] =	OpImpl::_movt;	//m_opSizeTable[op::MOVT ] = 1 + 8 + 8;
	m_opTable[op::MOV  ] =	OpImpl::_mov;	//m_opSizeTable[op::MOV  ] = 1 + 8 + 8;
	m_opTable[op::PUSH ] =	OpImpl::_push;	//m_opSizeTable[op::PUSH ] = 1 + 8;
	m_opTable[op::PUSHF] =	OpImpl::_pushf;	//m_opSizeTable[op::PUSHF] = 1;
	m_opTable[op::PUSHI] =	OpImpl::_pushi;	//m_opSizeTable[op::PUSHI] = 1 + 8;
	m_opTable[op::POP  ] =	OpImpl::_pop;	//m_opSizeTable[op::POP  ] = 1 + 8;
	m_opTable[op::POPF ] =	OpImpl::_popf;	//m_opSizeTable[op::POPF ] = 1;
	/* arithmetic */						//
	m_opTable[op::ADD] =	OpImpl::_add;	//m_opSizeTable[op::ADD] = 1 + 8 + 8;
	m_opTable[op::SUB] =	OpImpl::_sub;	//m_opSizeTable[op::SUB] = 1 + 8 + 8;
	m_opTable[op::CMP] =	OpImpl::_cmp;	//m_opSizeTable[op::CMP] = 1 + 8 + 8;
	m_opTable[op::MUL] =	OpImpl::_mul;	//m_opSizeTable[op::MUL] = 1 + 8 + 8;
	m_opTable[op::DIV] =	OpImpl::_div;	//m_opSizeTable[op::DIV] = 1 + 8 + 8;
	m_opTable[op::MOD] =	OpImpl::_mod;	//m_opSizeTable[op::MOD] = 1 + 8 + 8;
	m_opTable[op::INC] =	OpImpl::_inc;	//m_opSizeTable[op::INC] = 1 + 8;
	m_opTable[op::DEC] =	OpImpl::_dec;	//m_opSizeTable[op::DEC] = 1 + 8;
	m_opTable[op::AND] =	OpImpl::_and;	//m_opSizeTable[op::AND] = 1 + 8 + 8;
	m_opTable[op::OR ] =	OpImpl::_or;	//m_opSizeTable[op::OR ] = 1 + 8 + 8;
	m_opTable[op::XOR] =	OpImpl::_xor;	//m_opSizeTable[op::XOR] = 1 + 8 + 8;
	m_opTable[op::NOT] =	OpImpl::_not;	//m_opSizeTable[op::NOT] = 1 + 8;
	m_opTable[op::SHL] =	OpImpl::_shl;	//m_opSizeTable[op::SHL] = 1 + 8 + 8;
	m_opTable[op::SHR] =	OpImpl::_shr;	//m_opSizeTable[op::SHR] = 1 + 8 + 8;
	/* jumping/calling */					//
	m_opTable[op::CALLI] =	OpImpl::_calli; //m_opSizeTable[op::CALLI] = 1 + 8;
	m_opTable[op::CALLR] =	OpImpl::_callr;	//m_opSizeTable[op::CALLR] = 1 + 8;
	m_opTable[op::RET  ] =	OpImpl::_ret;	//m_opSizeTable[op::RET  ] = 1;
	m_opTable[op::JMP  ] =	OpImpl::_jmp;	//m_opSizeTable[op::JMP  ] = 1 + 8;
	m_opTable[op::JZ   ] =	OpImpl::_jz;	//m_opSizeTable[op::JZ   ] = 1 + 8;
	m_opTable[op::JNZ  ] =	OpImpl::_jnz;	//m_opSizeTable[op::JNZ  ] = 1 + 8;
	m_opTable[op::JE   ] =	OpImpl::_jz;	//m_opSizeTable[op::JE   ] = 1 + 8;
	m_opTable[op::JNE  ] =	OpImpl::_jnz;	//m_opSizeTable[op::JNE  ] = 1 + 8;
	m_opTable[op::JGT  ] =	OpImpl::_jgt;	//m_opSizeTable[op::JGT  ] = 1 + 8;
	m_opTable[op::JLT  ] =	OpImpl::_jlt;	//m_opSizeTable[op::JLT  ] = 1 + 8;
	m_opTable[op::JLE  ] =	OpImpl::_jle;	//m_opSizeTable[op::JLE  ] = 1 + 8;
	m_opTable[op::JGE  ] =	OpImpl::_jge;	//m_opSizeTable[op::JGE  ] = 1 + 8;
}
