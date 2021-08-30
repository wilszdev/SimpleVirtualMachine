#pragma once

#include <iostream>
#include <vector>

typedef uint8_t byte;

enum class State : byte
{
	START,
	READCHAR,
	READBLOCK,
	SKIP,
	DUMP,
	COMMENT,
	END
};

class Lexer
{
private:
	bool isSpace(char);
	bool isSpecial(char);
	bool isGroup(char);
	char end_char, beg_char;
public:
	std::vector<std::string> lex(const std::string&);
};