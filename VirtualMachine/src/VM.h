#pragma once

class VM
{
public:
	virtual void Run() = 0;
	virtual void LoadProgram(const void* mem, size_t size) = 0;
};